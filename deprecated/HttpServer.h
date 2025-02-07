//
// Created by ydc on 25-1-11.
//

#ifndef HTTPSERVER_H
#define HTTPSERVER_H
#include "MySqlBlog.hpp"
#include "async_mutex.h"
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/random_access_file.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/cobalt.hpp>
#include <boost/filesystem.hpp>
#include <boost/mysql.hpp>
#include <boost/url.hpp>

#include <boost/asio/buffers_iterator.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/regex.hpp>
#include <boost/variant.hpp>
#include <iostream>
#include <string>

#include <boost/asio/execution_context.hpp> // 执行上下文必不可少
#include <boost/asio/executor.hpp>          // 执行器必不可少

/*#define BOOST_ASIO_HAS_FILE 1
#define BOOST_ASIO_USE_IO_URING 1
#define ASIO_HAS_FILE 1
#define ASIO_HAS_IO_URING 1*/

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace mysql = boost::mysql;
namespace cobalt = boost::cobalt;
namespace fs = boost::filesystem;
using tcp = asio::ip::tcp;

using RequestHandler = std::function<cobalt::task<void>(
    http::request<http::string_body> &,
    http::response<http::string_body> &,
    std::vector<std::string_view>)>;

struct Route
{
    boost::regex regex;
    RequestHandler handler;
};

struct SessionData
{
    std::string user_id;
    std::chrono::time_point<std::chrono::system_clock> last_access_time;
    Database *database;
    // 其他 session 相关数据
};

class HttpServer
{
public:
    HttpServer(
        asio::io_context &ioc,
        unsigned short port,
        bool enable_https = false,
        const std::string &cert_path = "",
        const std::string &key_path = "")
        : ioc_(ioc)
        , acceptor_(ioc, {tcp::v4(), port})
        , enable_https_(enable_https)
        , timer_(ioc)
        , strand_(ioc.get_executor())
    {
        if (enable_https) {
            ctx_.set_options(
                asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2
                | asio::ssl::context::single_dh_use);
            ctx_.use_certificate_chain_file(cert_path);
            ctx_.use_private_key_file(key_path, asio::ssl::context::pem);
        }
    }

    cobalt::task<void> run() { co_await accept_loop(); }

public:
    cobalt::task<void> accept_loop()
    {
        for (;;) {
            beast::error_code ec;
            tcp::socket socket(ioc_);
            co_await acceptor_.async_accept(socket, cobalt::use_op); // 使用 cobalt::use_op
            if (ec) {
                std::cerr << "Accept failed: " << ec.message() << std::endl;
                continue;
            }
            if (enable_https_) {
                asio::ssl::stream<tcp::socket> ssl_stream(std::move(socket), ctx_);
                co_await ssl_stream.async_handshake(
                    asio::ssl::stream_base::server,
                    cobalt::use_op); // 使用 cobalt::use_op
                boost::cobalt::spawn(ioc_, process_session(std::move(ssl_stream)), asio::detached);
            } else {
                //auto &&basic_stream = beast::tcp_stream(socket, ioc_);
                boost::cobalt::spawn(ioc_, process_session(std::move(socket)), asio::detached);
            }
        }
    }

    template<typename T>
    T convert_arg(std::string_view arg)
    {
        if constexpr (std::is_same_v<T, uint64_t>) {
            uint64_t val;
            if (auto [p, ec] = std::from_chars(arg.data(), arg.data() + arg.size(), val);
                ec == std::errc{}) {
                return val;
            } else {
                throw std::runtime_error("convert error");
            }
        } else if constexpr (std::is_same_v<T, std::string>) {
            return std::string(arg);
        } else if constexpr (std::is_same_v<T, std::string_view>) {
            return arg;
        } //... 其他类型转换
        else if constexpr (std::is_floating_point_v<T>) {
            T val;
            std::stringstream ss(std::string(arg));
            ss >> val;
            return val;
        } else {
            static_assert(std::is_same_v<T, void>, "unsupport type");
            return {};
        }
    }

    template<typename... Args>
    void add_route(
        std::string method,
        std::string path_with_placeholders,
        std::function<cobalt::task<void>(
            http::request<http::string_body> &,
            http::response<http::string_body> &,
            std::shared_ptr<SessionData>,
            Args...)> handler)
    {
        // 编译时断言，检查类型是否匹配
        // static_assert(std::is_same_v<HandlerType, ExpectedType>, "Handler type mismatch!");

        std::string regex_str;
        std::vector<std::string> param_names;

        // 将占位符转换为正则表达式
        boost::regex placeholder_regex("\\{([^\\}]+)\\}"); // 匹配 {id} 这样的占位符
        boost::sregex_iterator
            it(path_with_placeholders.begin(), path_with_placeholders.end(), placeholder_regex);
        boost::sregex_iterator end;

        size_t last_pos = 0;
        for (; it != end; ++it) {
            boost::smatch match = *it;
            regex_str += boost::regex_replace(
                path_with_placeholders.substr(last_pos, match.position() - last_pos),
                boost::regex("\\."),
                "\\.");                      //将.替换为\.，防止regex解析错误
            regex_str += "([^/]+)";          // 匹配一个或多个非斜杠字符
            param_names.push_back(match[1]); // 提取占位符名称
            last_pos = match.position() + match.length();
        }
        regex_str += boost::regex_replace(
            path_with_placeholders.substr(last_pos),
            boost::regex("\\."),
            "\\."); //将.替换为\.，防止regex解析错误
        if (regex_str.empty())
            regex_str = path_with_placeholders;

        routes_[method].emplace(
            std::piecewise_construct,
            std::forward_as_tuple(path_with_placeholders),
            std::forward_as_tuple(
                boost::regex(regex_str),
                [handler, this](
                    http::request<http::string_body> &req,
                    http::response<http::string_body> &res,
                    std::vector<std::string_view> args) mutable -> cobalt::task<void> {
                    try {
                        auto session = co_await this->get_or_create_session(req, res);
                        if constexpr (sizeof...(Args) > 0) {
                            if (args.size() != sizeof...(Args)) {
                                res.result(http::status::bad_request);
                                res.body() = "Incorrect number of arguments";
                                co_return;
                            }
                            try {
                                size_t i = 0;
                                co_await
                                handler(req, res, session, this->convert_arg<Args>(args[i++])...);
                            } catch (const std::runtime_error &e) {
                                res.result(http::status::bad_request);
                                res.body() = e.what();
                                co_return;
                            }
                        } else {
                            co_await handler(req, res, session);
                        }
                    } catch (const std::exception &e) {
                        std::cerr << "Handler exception: " << e.what() << std::endl;
                        res.result(http::status::internal_server_error);
                        res.body() = "Internal Server Error";
                        co_return;
                    }
                }));
    }

    template<typename Stream>
    cobalt::task<void> process_session(Stream &&stream)
    {
        try {
            beast::flat_buffer req_buffer;
            for (;;) {
                if constexpr (std::is_same_v<Stream, beast::tcp_stream>) {
                    // Set the timeout.
                    stream.expires_after(std::chrono::seconds(30));
                } else if constexpr (std::is_same_v<
                                         Stream,
                                         boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>) {
                }
                http::request<http::string_body> req;
                co_await http::async_read(stream, req_buffer, req);

                http::response<http::string_body> res;
                boost::urls::url_view target(req.target());
                std::string_view path = target.path();
                auto const method = req.method_string();

                // 处理静态资源
                if (req.method() == http::verb::get) {
                    std::string file_path = "./static/" + std::string(req.target());

                    if (file_path.find('.') != std::string::npos) {
                        std::string_view mime = mime_type(file_path);
                        res.set(http::field::content_type, mime);

                        if (fs::exists(file_path) && fs::is_regular_file(file_path)) {
                            // 使用 random_access_file 进行异步文件操作
                            asio::random_access_file
                                file(stream.get_executor(), file_path, asio::file_base::read_only);

                            std::size_t file_size = fs::file_size(file_path);
                            boost::beast::flat_buffer buffer_file;
                            buffer_file.reserve(file_size);

                            // 异步读取文件内容到 buffer
                            std::size_t offset = 0; // 从文件开始处读取
                            std::size_t bytes_read = co_await file.async_read_some_at(
                                offset, buffer_file.prepare(file_size), cobalt::use_op);
                            buffer_file.commit(bytes_read);

                            // 异步读取文件内容到 buffer
                            // 将 buffer 数据转为字符串并设置为响应体
                            //res.body() = std::string(buffer.data(),buffer.e, bytes_read);
                            std::string_view file_string = std::string_view(
                                static_cast<const char *>(buffer_file.data().data()),
                                buffer_file.size());
                            res.body() = file_string;
                            res.prepare_payload();

                            co_await http::async_write(stream, res, cobalt::use_op);
                            continue;
                        } else {
                            res.result(http::status::not_found);
                            res.body() = "Not Found";
                            co_await http::async_write(stream, res, cobalt::use_op);
                            continue;
                        }
                    }
                }

                // 设置动态接口
                if (auto const it = routes_.find(method); it != routes_.end()) {
                    for (const auto &[path_regex_str, route] : it->second) {
                        boost::regex path_regex(path_regex_str);
                        boost::cmatch match;
                        if (boost::regex_match(path.begin(), path.end(), match, path_regex)) {
                            try {
                                std::vector<std::string_view> args;
                                if (match.size() > 1) {
                                    for (size_t i = 1; i < match.size(); ++i) {
                                        args.push_back(match[i].str());
                                    }
                                }
                                // 设置中文响应的 Content-Type
                                // if (/* 判断是否是中文响应的条件 */) {
                                //}
                                res.set(http::field::content_type, "text/plain; charset=utf-8");

                                co_await route.handler(req, res, args);
                                co_await http::async_write(stream, std::move(res), cobalt::use_op);
                                continue;
                            } catch (const std::exception &e) {
                                res.result(http::status::internal_server_error);
                                res.body() = "Internal Server Error";
                                //co_await http::async_write(stream, res, cobalt::use_op);
                                //co_return;
                                continue;
                            }
                        }
                    }
                }

                if (res.body().empty()) {
                    res.result(http::status::not_found);
                    res.body() = "Not Found";
                }
                co_await http::async_write(stream, std::move(res), cobalt::use_op);
            }
        } catch (const std::exception &e) {
            std::cerr << "process_session error: " << e.what() << std::endl;
        }
    }

    std::string generate_session_id()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(0, 255);

        std::stringstream ss;
        for (int i = 0; i < 16; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << distrib(gen);
        }
        return ss.str();
    }

    std::string_view mime_type(beast::string_view path)
    {
        using beast::iequals;
        auto const ext = path.substr(path.rfind("."));
        // Use flat_map beast::string_view to avoid unnecessary copies
        return mime_types.find(ext) != mime_types.end() ? mime_types.at(ext) : "application/text";
    }

    void cleanup_expired_sessions()
    {
        boost::asio::post(strand_, [this]() -> boost::cobalt::task<void> {
            co_await sessions_mutex_.coLock(); // 获取异步锁
            auto it = sessions_.begin();
            while (it != sessions_.end()) {
                if (std::chrono::system_clock::now() - it->second.last_access_time
                    > session_timeout_) {
                    it = sessions_.erase(it);
                } else {
                    ++it;
                }
            }
            sessions_mutex_.unlock(); // 释放异步锁
        });
    }

    void handle_timeout(const boost::system::error_code &error)
    {
        if (!error) {
            cleanup_expired_sessions();
            timer_.expires_at(timer_.expiry() + std::chrono::minutes(1));
            timer_.async_wait(
                boost::asio::bind_executor(
                    strand_, std::bind(&HttpServer::handle_timeout, this, std::placeholders::_1)));
        }
    }

    void start_cleanup_timer()
    {
        boost::asio::post(strand_, [this]() { this->handle_timeout(boost::system::error_code{}); });
    }

    cobalt::task<std::shared_ptr<SessionData>> get_or_create_session(
        http::request<http::string_body> &req, http::response<http::string_body> &res)
    {
        std::string session_id;

        auto const it = req.find(http::field::cookie);
        if (it != req.end()) {
            std::string cookie_value = it->value();
            size_t pos = cookie_value.find("session_id=");
            if (pos != std::string::npos) {
                session_id = cookie_value.substr(pos + 11);
            }
        }

        co_return co_await find_or_create_session_impl(session_id, res);
        ;
    }

    cobalt::task<std::shared_ptr<SessionData>> find_or_create_session_impl(
        const std::string &session_id, http::response<http::string_body> &res)
    {
        co_await sessions_mutex_.coLock();
        auto session_it = sessions_.find(session_id);
        if (session_it != sessions_.end()) {
            session_it->second.last_access_time = std::chrono::system_clock::now();
            std::shared_ptr<SessionData> ptr(&session_it->second, [](SessionData *) {
            }); // No-op deleter
            sessions_mutex_.unlock();
            co_return ptr;
        }

        std::string new_session_id = generate_session_id();
        SessionData new_session_data;
        new_session_data.last_access_time = std::chrono::system_clock::now();
        sessions_[new_session_id] = new_session_data;
        res.set(http::field::set_cookie, "session_id=" + new_session_id + "; Path=/; HttpOnly");
        std::shared_ptr<SessionData> ptr(&sessions_.at(new_session_id), [](SessionData *) {
        }); // No-op deleter
        sessions_mutex_.unlock();
        co_return ptr;
    }

private:
    asio::io_context &ioc_;
    tcp::acceptor acceptor_;
    bool enable_https_;
    asio::ssl::context ctx_{asio::ssl::context::tlsv12};

    boost::asio::steady_timer timer_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    fast::db::SpinLock sessions_mutex_;

    using SessionsMap = std::unordered_map<std::string, SessionData>;
    SessionsMap sessions_;
    const std::chrono::minutes session_timeout_{30};

    // 请求映射表，按方法分类
    std::unordered_map<std::string, std::unordered_map<std::string, Route>> routes_;

    // Define a lookup table for common extensions and their MIME types
    const boost::container::flat_map<std::string, std::string> mime_types = {
        {".htm", "text/html"},
        {".html", "text/html"},
        {".php", "text/html"}, // Placeholder, actual type may vary
        {".css", "text/css"},
        {".txt", "text/plain"},
        {".js", "application/javascript"},
        {".json", "application/json"},
        {".xml", "application/xml"},
        {".swf", "application/x-shockwave-flash"},
        {".flv", "video/x-flv"},
        {".png", "image/png"},
        {".jpe", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".jpg", "image/jpeg"},
        {".gif", "image/gif"},
        {".bmp", "image/bmp"},
        {".ico", "image/vnd.microsoft.icon"},
        {".tiff", "image/tiff"},
        {".tif", "image/tiff"},
        {".svg", "image/svg+xml"},
        {".svgz", "image/svg+xml"},
    };
};

#endif //HTTPSERVER_H
