module;

//#include <boost/asio/co_spawn.hpp>
#include "../src/db/boost_head.h"

#include "utils/log.h"

#include <chrono>
#include <iostream>
#include <map>
#include <ostream>
#include <regex>
#include <string>
#include <vector>

export module mysql_connect;

export namespace fast::db {
namespace json = boost::json;
namespace mysql = boost::mysql;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;

using executor_type = net::strand<net::io_context::executor_type>;

// 数据库连接配置
struct DbConfig
{
    std::string host;
    std::string user;
    std::string password;
    std::string database;
};

// 移除 HTML 标签的函数
std::string remove_html_tags(const std::string &html)
{
    std::regex re("<[^>]*>");
    return std::regex_replace(html, re, "");
}

// BlogArticle 结构体
struct BlogArticle
{
    friend void swap(BlogArticle &lhs, BlogArticle &rhs) noexcept
    {
        using std::swap;
        swap(lhs.article_id, rhs.article_id);
        swap(lhs.title, rhs.title);
        swap(lhs.content_preview, rhs.content_preview);
        swap(lhs.markdown, rhs.markdown);
        swap(lhs.create_time, rhs.create_time);
        swap(lhs.update_time, rhs.update_time);
    }
    friend std::ostream &operator<<(std::ostream &os, const BlogArticle &obj)
    {
        return os << "article_id: " << obj.article_id << " title: " << obj.title
                  << " content_preview: " << obj.content_preview << " markdown: " << obj.markdown
                  << " create_time: " << obj.create_time << " update_time: " << obj.update_time;
    }
    uint64_t article_id;
    std::string title;
    std::string content_preview;
    std::string markdown;
    mysql::datetime create_time;
    mysql::datetime update_time;
};

class BlogColumn
{
public:
    BlogColumn()
        : column_id(0)
        , pid(0)
    {}

    BlogColumn(const BlogColumn &other)
        : column_id(other.column_id)
        , column_name(other.column_name)
        , column_description(other.column_description)
        , pid(other.pid)
        , children(other.children)
    {}
    BlogColumn(BlogColumn &&other) noexcept
        : column_id(other.column_id)
        , column_name(std::move(other.column_name))
        , column_description(std::move(other.column_description))
        , pid(other.pid)
        , children(std::move(other.children))
    {}
    BlogColumn &operator=(const BlogColumn &other)
    {
        if (this == &other)
            return *this;
        column_id = other.column_id;
        column_name = other.column_name;
        column_description = other.column_description;
        pid = other.pid;
        children = other.children;
        return *this;
    }
    BlogColumn &operator=(BlogColumn &&other) noexcept
    {
        if (this == &other)
            return *this;
        column_id = other.column_id;
        column_name = std::move(other.column_name);
        column_description = std::move(other.column_description);
        pid = other.pid;
        children = std::move(other.children);
        return *this;
    }
    friend std::ostream &operator<<(std::ostream &os, const BlogColumn &obj)
    {
        return os << "column_id: " << obj.column_id << " column_name: " << obj.column_name
                  << " column_description: " << obj.column_description << " pid: " << obj.pid
                  << " children: ";
    }
    uint64_t column_id;
    std::string column_name;
    std::string column_description;
    uint64_t pid;
    std::vector<BlogColumn> children; // 用于存储子栏目
};

struct OptionParams
{
    std::string host = "localhost";
    unsigned short port = 3306;
    std::string username;
    std::string password;
    std::string database;
    boost::mysql::ssl_mode ssl{boost::mysql::ssl_mode::disable};
    bool multi_queries{true};
    std::size_t initial_size{1};
    std::size_t max_size{2};
    std::chrono::seconds connect_timeout{1};
    std::chrono::seconds retry_interval{1};
    std::chrono::hours ping_interval{1};
    std::chrono::seconds ping_timeout{1};
    bool thread_safe{true};
};

inline std::string as_string(std::string_view view)
{
    return std::string(view); // 最简洁的拷贝方式
}
// 判断一个字节是否是 UTF-8 编码的起始字节
bool is_utf8_starter(char c)
{
    return (c & 0x80) != 0 && (c & 0x40) != 0; // 10xxxxxx 不是起始字节，C0和C1控制区也不是有效起始
}

// 分割 UTF-8 编码的 std::string 中的前 n 个 Unicode 字符
std::string split_first_n_unicode_chars(const std::string &utf8_str, size_t n)
{
    std::string result;
    size_t unicode_chars_count = 0;
    size_t i = 0;

    while (i < utf8_str.size() && unicode_chars_count < n) {
        if (is_utf8_starter(utf8_str[i])) {
            // 找到一个 UTF-8 字符的起始字节，需要确定这个字符占用的字节数
            size_t bytes_needed = 1;
            if ((utf8_str[i] & 0xE0) == 0xC0)
                bytes_needed = 2; // 110xxxxx
            else if ((utf8_str[i] & 0xF0) == 0xE0)
                bytes_needed = 3; // 1110xxxx
            else if ((utf8_str[i] & 0xF8) == 0xF0)
                bytes_needed = 4; // 11110xxx (仅支持到 4 字节 UTF-8 字符)

            // 将这个 UTF-8 字符添加到结果字符串中
            result.append(utf8_str.begin() + i, utf8_str.begin() + i + bytes_needed);
            unicode_chars_count++;
            i += bytes_needed - 1; // 跳过已经处理的字节（除了当前的起始字节）
        }
        i++;
    }

    return result;
}

class Database
{
public:
    Database(mysql::connection_pool &pool)
    {
        pool_ = &pool;
        pool_->async_run(asio::detached);
    }

    boost::cobalt::task<std::vector<BlogArticle>> get_articles_by_column_id(
        uint64_t column_id, int page = 0, int page_size = 10)
    {
        mysql::pooled_connection conn = co_await pool_->async_get_connection(boost::cobalt::use_op);

        int offset = page * page_size;
        mysql::results result;
        mysql::diagnostics diag;
        auto with_params = mysql::with_params(
            "SELECT article_id, title, content, create_time, update_time FROM "
            "blog_article WHERE column_id = {} LIMIT {}, {}",
            column_id,
            offset,
            page_size);
        co_await conn->async_execute(with_params, result, diag, boost::cobalt::use_op);

        std::vector<BlogArticle> articles;
        for (auto &&row : result.rows()) {
            BlogArticle article;
            article.article_id = row.at(0).as_int64();
            article.title = row.at(1).as_string();
            article.content_preview = remove_html_tags(row.at(2).as_string());
            article.create_time = row.at(3).as_datetime();
            article.update_time = row.at(4).as_datetime();
            if (article.content_preview.length() > 100) {
                article.content_preview = split_first_n_unicode_chars(article.content_preview, 40)
                                          + "...";
            }
        }
        co_return articles;
    }

    boost::cobalt::task<void> get_all_blog_columns(std::vector<BlogColumn> &columns)
    {
        auto conn = co_await pool_->async_get_connection(boost::cobalt::use_op);

        mysql::results result;
        mysql::diagnostics diag;
        co_await conn->async_execute(
            "SELECT column_id, column_name, column_description, pid FROM blog_column",
            result,
            diag,
            boost::cobalt::use_op);

        if (!result.has_value()) {
            LOG_INFO_R << "not info";
        }
        for (auto &&row : result.rows()) {
            try {
                BlogColumn column;
                column.column_id = row.at(0).as_int64();
                column.column_name = as_string(row.at(1).as_string());
                column.column_description = as_string(row.at(2).as_string());
                column.pid = row.at(3).as_int64();
                columns.push_back(column);
            } catch (const std::invalid_argument &e) {
                std::cerr << "Invalid column_id: " << e.what() << std::endl;
                continue;
            } catch (const std::out_of_range &e) {
                std::cerr << "Out of range column_id: " << e.what() << std::endl;
                continue;
            } catch (std::exception &e) {
                std::cout << e.what() << std::endl;
            }
        }
    }

    boost::asio::awaitable<void> get_articles_full_text_awaitable(
        std::string &full_text,
        int page,
        int page_size,
        std::vector<fast::db::BlogArticle> &articles)
    {
        try {
            mysql::pooled_connection conn = co_await pool_->async_get_connection(
                asio::use_awaitable);

            int offset = (page > 0) ? (page - 1) * page_size : 0;
            mysql::results result;
            mysql::diagnostics diag;

            std::ostringstream query;
            query << "SELECT article_id, title, content FROM "
                  << "blog_article WHERE MATCH (title, markdown) AGAINST ('" << full_text
                  << "' IN NATURAL LANGUAGE MODE) "
                  << "LIMIT " << offset << ", " << page_size;
            std::string sql_str = query.str();

            co_await conn->async_execute(sql_str, result, diag, asio::use_awaitable);

            for (auto &&row : result.rows()) {
                BlogArticle article;
                article.article_id = row.at(0).as_int64();
                article.title = row.at(1).as_string();
                article.content_preview = remove_html_tags(row.at(2).as_string());
                if (article.content_preview.length() > 30) {
                    article.content_preview
                        = split_first_n_unicode_chars(article.content_preview, 40) + "...";
                }
                articles.emplace_back(article);
            }

        } catch (const std::exception &e) {
            std::cerr << "General Exception in get_articles_full_text_awaitable: " << e.what()
                      << std::endl;
        }
        co_return;
    }

    boost::asio::awaitable<void> get_articles_by_column_id_awaitable(
        uint64_t column_id, int page, int page_size, std::vector<fast::db::BlogArticle> &articles)
    {
        try {
            mysql::pooled_connection conn = co_await pool_->async_get_connection(
                asio::use_awaitable);

            int offset = (page > 0) ? (page - 1) * page_size : 0;
            mysql::results result;
            mysql::diagnostics diag;

            if (column_id == 0) {
                co_await conn->async_execute(
                    mysql::with_params(
                        "SELECT article_id, title, content FROM "
                        "blog_article LIMIT {}, {}",
                        offset,
                        page_size),
                    result,
                    diag,
                    asio::use_awaitable);
            } else {
                co_await conn->async_execute(
                    mysql::with_params(
                        "SELECT article_id, title, content FROM "
                        "blog_article WHERE column_id = {} LIMIT {}, {}",
                        column_id,
                        offset,
                        page_size),
                    result,
                    diag,
                    asio::use_awaitable);
            }

            for (auto &&row : result.rows()) {
                BlogArticle article;
                article.article_id = row.at(0).as_int64();
                article.title = row.at(1).as_string();
                article.content_preview = remove_html_tags(row.at(2).as_string());
                if (article.content_preview.length() > 30) {
                    article.content_preview
                        = split_first_n_unicode_chars(article.content_preview, 40) + "...";
                }
                articles.emplace_back(article);
            }

        } catch (const std::exception &e) {
            std::cerr << "General Exception in get_articles_by_column_id_awaitable: " << e.what()
                      << std::endl;
        }
        co_return;
    }

    boost::asio::awaitable<void> get_all_blog_columns_awaitable(std::vector<BlogColumn> &columns)
    {
        try {
            mysql::pooled_connection conn = co_await pool_->async_get_connection(
                asio::use_awaitable);

            mysql::results result;
            mysql::diagnostics diag;
            co_await conn->async_execute(
                "SELECT column_id, column_name, column_description, pid FROM blog_column",
                result,
                diag,
                asio::use_awaitable);

            if (!result.has_value()) {
                LOG_INFO_R << "not info";
            }
            for (auto &&row : result.rows()) {
                try {
                    BlogColumn column;
                    column.column_id = row.at(0).as_int64();
                    column.column_name = as_string(row.at(1).as_string());
                    column.column_description = as_string(row.at(2).as_string());
                    column.pid = row.at(3).as_int64();
                    columns.push_back(column);
                } catch (const std::invalid_argument &e) {
                    std::cerr << "Invalid column_id: " << e.what() << std::endl;
                    continue;
                } catch (const std::out_of_range &e) {
                    std::cerr << "Out of range column_id: " << e.what() << std::endl;
                    continue;
                } catch (std::exception &e) {
                    std::cout << e.what() << std::endl;
                }
            }
        } catch (const boost::system::system_error &ec) {
            std::cerr << "Database Error (system_error): " << ec.code().message() << " ("
                      << ec.code().value() << ")" << std::endl;
        } catch (const std::exception &e) {
            std::cerr << "General Exception in get_all_blog_columns_awaitable: " << e.what()
                      << std::endl;
        }
    }

    boost::asio::awaitable<void> get_article_by_article_id_awaitable(
        uint64_t article_id, BlogArticle &article)
    {
        try {
            auto conn = co_await pool_->async_get_connection(asio::use_awaitable);

            mysql::results result;
            mysql::diagnostics diag;
            co_await conn->async_execute(
                mysql::with_params(
                    "SELECT title, markdown, create_time, update_time "
                    "FROM blog_article "
                    "WHERE article_id = {}",
                    article_id),
                result,
                diag,
                asio::use_awaitable);

            if (!result.has_value()) {
                co_return;
            }
            mysql::row_view row = result.rows().at(0);
            article.title = row.at(0).as_string();
            article.markdown = row.at(1).as_string();
            article.create_time = row.at(2).as_datetime();
            article.update_time = row.at(3).as_datetime();
        } catch (const boost::system::system_error &ec) {
            std::cerr << "Database Error (system_error): " << ec.code().message() << " ("
                      << ec.code().value() << ")" << std::endl;
        } catch (const std::exception &e) {
            std::cerr << "General Exception in get_all_blog_columns_awaitable: " << e.what()
                      << std::endl;
        }
    }

    boost::asio::awaitable<void> watch_number_added_awaitable(uint64_t article_id)
    {
        auto conn = co_await pool_->async_get_connection(asio::use_awaitable);
        mysql::results result;
        mysql::diagnostics diag;

        co_await conn->async_execute(
            mysql::with_params(
                "UPDATE blog_article "
                "SET watch_number = watch_number + 1 "
                "WHERE article_id = {}",
                article_id),
            result,
            diag,
            asio::use_awaitable);
    }

    boost::cobalt::task<void> get_article_by_article_id(uint64_t article_id, BlogArticle &article)
    {
        auto conn = co_await pool_->async_get_connection(boost::cobalt::use_op);

        mysql::results result;
        mysql::diagnostics diag;
        co_await conn->async_execute(
            mysql::with_params(
                "SELECT title, markdown, create_time, update_time "
                "FROM blog_article "
                "WHERE article_id = {}",
                article_id),
            result,
            diag,
            boost::cobalt::use_op);

        if (!result.has_value()) {
            co_return;
        }
        mysql::row_view row = result.rows().at(0);
        article.title = row.at(0).as_string();
        article.markdown = row.at(1).as_string();
        article.create_time = row.at(2).as_datetime();
        article.update_time = row.at(3).as_datetime();
        //LOG_INFO_R << article;
    }

    boost::cobalt::task<void> watch_number_added(uint64_t article_id)
    {
        auto conn = co_await pool_->async_get_connection(boost::cobalt::use_op);
        mysql::results result;
        mysql::diagnostics diag;

        co_await conn->async_execute(
            mysql::with_params(
                "UPDATE blog_article "
                "SET watch_number = watch_number + 1 "
                "WHERE article_id ",
                article_id),
            result,
            diag,
            boost::cobalt::use_op);
    }

    static mysql::pool_params createMysqlOption(const OptionParams &params)
    {
        mysql::pool_params mysql_params;
        mysql_params.server_address.emplace_host_and_port(params.host, params.port);
        mysql_params.username = params.username;
        mysql_params.password = params.password;
        mysql_params.database = params.database;
        mysql_params.ssl = params.ssl;
        mysql_params.multi_queries = params.multi_queries;
        mysql_params.initial_size = params.initial_size;
        mysql_params.max_size = params.max_size;
        mysql_params.connect_timeout = params.connect_timeout;
        mysql_params.retry_interval = params.retry_interval;
        mysql_params.ping_interval = params.ping_interval;
        mysql_params.ping_timeout = params.ping_timeout;
        mysql_params.thread_safe = params.thread_safe;
        return mysql_params;
    }

private:
    mysql::connection_pool *pool_;
};

// 递归构建树形结构
inline void build_column_tree(std::vector<BlogColumn> &columns)
{
    std::map<uint64_t, std::vector<BlogColumn *>> children_map;

    for (auto &column : columns) {
        children_map[column.pid].push_back(&column);
    }

    for (auto &column : columns) {
        if (children_map.contains(column.column_id)) {
            for (auto *child : children_map[column.column_id]) {
                column.children.push_back(std::move(*child));
            }
        }
    }
    std::erase_if(columns, [](const BlogColumn &c) { return c.pid != 0; });
}
// 递归构建树形结构并排序
void build_column_tree_recursive(
    std::vector<BlogColumn> &all_columns,
    std::vector<BlogColumn> &current_level,
    uint64_t parent_id = 0)
{
    for (auto &column : all_columns) {
        if (column.pid == parent_id) {
            current_level.push_back(column);
        }
    }

    // 按照 column_id 排序当前层
    std::sort(current_level.begin(), current_level.end(), [](const BlogColumn &a, const BlogColumn &b) {
        return a.column_id < b.column_id;
    });

    for (auto &column : current_level) {
        std::vector<BlogColumn> children;
        build_column_tree_recursive(all_columns, children, column.column_id);
        column.children = std::move(children);
    }
}

// 转换函数
std::chrono::system_clock::time_point mysql_datetime_to_time_point(const boost::mysql::datetime &dt)
{
    using namespace std::chrono;

    // 从 1970-01-01 00:00:00 UTC 开始，逐级添加年月日时分秒
    auto tp = system_clock::from_time_t(0);
    tp += hours(dt.hour()) + minutes(dt.minute()) + seconds(dt.second());
    tp += days(dt.day() - 1) + months(dt.month() - 1) + years(dt.year() - 1970);

    return tp;
}

// 辅助函数：将时间点转换为 ISO 8601 格式的字符串
std::string to_iso_string(const std::chrono::system_clock::time_point &tp)
{
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::localtime(&tt);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
    return oss.str();
}

boost::json::object article_to_json(const BlogArticle &article)
{
    boost::json::object jobj;
    jobj["article_id"] = article.article_id;
    jobj["title"] = article.title;
    jobj["content_preview"] = article.content_preview;
    jobj["markdown"] = article.markdown;
    jobj["create_time"] = to_iso_string(mysql_datetime_to_time_point(article.create_time));
    jobj["update_time"] = to_iso_string(mysql_datetime_to_time_point(article.update_time));
    return jobj;
}
boost::json::value vec_article_to_json(const std::vector<BlogArticle> &article)
{
    json::array j_columns;
    for (auto &&blog_article : article) {
        j_columns.push_back(article_to_json(blog_article));
    }
    return j_columns;
}

// 将 BlogColumn 转换为 JSON
json::value column_to_json(const BlogColumn &column)
{
    json::object j;
    j["column_id"] = column.column_id;
    j["column_name"] = column.column_name;
    j["column_description"] = column.column_description;
    j["pid"] = column.pid;

    if (!column.children.empty()) {
        json::array children_array;
        for (const auto &child : column.children) {
            children_array.push_back(column_to_json(child)); // 递归调用
        }
        j["children"] = children_array;
    }
    return j;
}

json::value columns_to_json(const std::vector<BlogColumn> &columns)
{
    json::array j_columns;
    for (const auto &column : columns) {
        j_columns.push_back(column_to_json(column));
    }
    return j_columns;
}

}