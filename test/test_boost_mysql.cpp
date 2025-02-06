#include "db/boost_head.h"

#include <chrono>
#include <cstdint>
#include <exception>
#include <iostream>
#include <string>

#include "db/async_mutex.h"
#include "doctest.h"
#include "utils/log.h"
#include <boost/cobalt.hpp>
#include <map>

import beast_net_server;
import mysql_connect;
import BS.thread_pool;

namespace mysql = boost::mysql;
namespace asio = boost::asio;

/*boost::asio::awaitable<void> test_asio(sam::asio_async_mutex & m, int id) {
    co_await m.async_lock();
    std::cout << "Coroutine " << id << " (asio) acquired the lock\n";
    co_await boost::asio::post(boost::asio::use_awaitable);
    std::cout << "Coroutine " << id << " (asio) releases the lock\n";
    m.unlock();
}*/

boost::cobalt::task<void> test_cobalt(
    fast::db::SpinLock &&m,
    int id,
    std::vector<std::chrono::duration<double>> &durations,
    int iterations)
{
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        co_await m.coLock();
        int sum = 0;
        for (int j = 0; j < 100; ++j) {
            sum += j;
        }
        m.unlock();
    }
    auto end = std::chrono::high_resolution_clock::now();
    durations[id] = end - start;
}

boost::cobalt::task<void> test_cobalt_scoped(
    fast::db::SpinLock &lock,
    int id,
    std::vector<std::chrono::duration<double>> &durations,
    int iterations)
{
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto g = lock.coScopedLock();
        co_await lock.coScopedLock();
        int sum = 0;
        for (int j = 0; j < 100; ++j) {
            sum += j;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    durations[id] = end - start;
}

DOCTEST_TEST_CASE("cobalt_mutex concurrent test")
{
    constexpr int num_coroutines = 10;
    constexpr int iterations = 10000;
    boost::asio::io_context ioc(10);
    std::vector<std::chrono::duration<double>> durations(num_coroutines);

    for (int i = 0; i < num_coroutines; ++i) {
        fast::db::SpinLock m;
        boost::cobalt::spawn(
            ioc, test_cobalt(std::move(m), i, durations, iterations), boost::asio::detached);
    }
    ioc.run();
    double total_lock_time = 0;
    for (const auto &duration : durations) {
        total_lock_time += duration.count();
    }
    std::cout << "cobalt_mutex Total lock time: " << total_lock_time << " seconds\n";
}

DOCTEST_TEST_CASE("SpinLock coScopedLock test")
{
    constexpr int num_coroutines = 10;
    constexpr int iterations = 10000;
    boost::asio::io_context ioc(std::thread::hardware_concurrency());
    fast::db::SpinLock lock;
    std::vector<std::chrono::duration<double>> durations(num_coroutines);

    for (int i = 0; i < num_coroutines; ++i) {
        boost::cobalt::spawn(
            ioc, test_cobalt_scoped(lock, i, durations, iterations), boost::asio::detached);
    }
    ioc.run();
    double total_lock_time = 0;
    for (const auto &duration : durations) {
        total_lock_time += duration.count();
    }
    std::cout << "coScopedLock Total lock time: " << total_lock_time << " seconds\n";
}

boost::cobalt::task<void> handle_chinese(
    boost::beast::http::request<boost::beast::http::string_body> &req,
    boost::beast::http::response<boost::beast::http::string_body> &res,
    fast::db::SessionData &session_datadata,
    std::vector<std::any> args)
{
    res.body() = "你好，世界！";                                                   // 中文回复
    res.set(boost::beast::http::field::content_type, "text/plain; charset=utf-8"); // 明确设置 Content-Type
    co_return;
}

boost::cobalt::task<void> handle_query_by_id(
    boost::beast::http::request<boost::beast::http::string_body> &req,
    boost::beast::http::response<boost::beast::http::string_body> &res,
    fast::db::SessionData &session_datadata,
    uint64_t id)
{
    /*auto result = co_await db.query_by_id(id);
    res.body() = result;*

    res.set(http::field::content_type, "text/plain");
    co_return;
}

cobalt::task<void> handle_update_by_id(http::request<http::string_body>& req, http::response<http::string_body>& res, uint64_t id, std::string data) {
    /*auto rows_affected = co_await db.update_by_id(id, data);
    res.body() = "Updated " + std::to_string(rows_affected) + " row(s)";*/
    res.set(boost::beast::http::field::content_type, "text/plain");
    co_return;
}
boost::cobalt::task<void> handle_query_by_name(
    boost::beast::http::request<boost::beast::http::string_body> &req,
    boost::beast::http::response<boost::beast::http::string_body> &res,
    std::shared_ptr<fast::db::SessionData> session_datadata,
    std::string name)
{
    // auto result = co_await db.query_by_name(name);
    // res.body() = result;
    res.set(boost::beast::http::field::content_type, "text/plain");
    co_return;
}

boost::cobalt::task<void> handle_product(
    boost::beast::http::request<boost::beast::http::string_body> &req,
    boost::beast::http::response<boost::beast::http::string_body> &res,
    std::shared_ptr<fast::db::SessionData> session_datadata,
    uint64_t id,
    std::string name)
{
    res.set(boost::beast::http::field::content_type, "text/plain");
    co_return;
}

boost::cobalt::task<void> work_http(asio::io_context &ctx, fast::db::Database &db)
{
    /*HttpServer http_server(ctx, 8080);

    co_await http_server.run();*/
    /*c++20*/
    //http_server.add_route("GET", "/queryname/{name}", handle_query_by_name);

    /**c++17 */
    // 正确的调用方式：使用 std::function 包装 handler
    /*http_server.add_route<std::string>("GET", "/queryname/{name}",
        std::function<cobalt::task<void>(http::request<http::string_body>&, http::response<http::string_body>&,SessionData &session_datadata, std::string)>(handle_query_by_name)
    );
    http_server.add_route<uint64_t, std::string>("GET", "/products/{id}/{name}",
        std::function<cobalt::task<void>(http::request<http::string_body>&, http::response<http::string_body>&, SessionData &session_datadata,uint64_t, std::string)>(handle_product)
    );*/
    co_return;
}
using executor_type = boost::asio::strand<boost::asio::io_context::executor_type>;

DOCTEST_TEST_CASE("Router and Server Test")
{
    boost::asio::io_context ioc;
    //executor_type ioc;
    asio::io_context db_ctx(2);
    fast::db::OptionParams
        params{"127.0.0.1", 3306, "root", "12345678", "blog"}; // 替换为你的数据库配置
    mysql::connection_pool pool(db_ctx, fast::db::Database::createMysqlOption(params));

    fast::db::Server server(
        boost::asio::ip::make_address("127.0.0.1"),
        8080,
        "/mnt/h/study/cpp/fast_db_master/cmake-build-debug-wsl24/",
        2,
        fast::db::Server::Protocol::HTTP,
        pool);

    server.add_route(
        "GET",
        "/hello",
        [](boost::beast::http::request<boost::beast::http::string_body> &req,
           boost::beast::http::response<boost::beast::http::string_body> &res,
           fast::db::SessionData &session) -> boost::asio::awaitable<void> { // 修改 session 参数类型
            fast::db::Database *database = session.get_database();
            std::vector<fast::db::BlogColumn> columns;

            co_await database->get_all_blog_columns_awaitable(columns);

            std::vector<fast::db::BlogColumn> out_columns;
            build_column_tree_recursive(columns, out_columns);
            boost::json::value to_json = columns_to_json(out_columns);
            res.body() = boost::json::serialize(to_json);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.prepare_payload();
            co_return;
        });

    server.add_route_rest_full(
        "GET",
        "/hello/{id}",
        [](boost::beast::http::request<boost::beast::http::string_body> &req,
           boost::beast::http::response<boost::beast::http::string_body> &res,
           fast::db::SessionData &session,
           boost::variant<std::map<std::string, std::string>> &params)
            -> boost::asio::awaitable<void> {
            auto &path_params = boost::get<std::map<std::string, std::string>>(params);
            std::string id = path_params["id"];
            fast::db::Database *database = session.get_database();
            std::vector<fast::db::BlogColumn> columns;

            co_await database->get_all_blog_columns_awaitable(columns);

            std::vector<fast::db::BlogColumn> out_columns;
            build_column_tree_recursive(columns, out_columns);
            boost::json::value to_json = columns_to_json(out_columns);
            res.body() = boost::json::serialize(to_json);
            res.set(boost::beast::http::field::content_type, "text/plain");
            res.prepare_payload();
            co_return;
        });

    server.add_route_rest_full(
        "GET",
        "/user/{id}/{name}",
        [](boost::beast::http::request<boost::beast::http::string_body> &req,
           boost::beast::http::response<boost::beast::http::string_body> &res,
           fast::db::SessionData &session,
           boost::variant<std::map<std::string, std::string>> &params)
            -> boost::asio::awaitable<void> {
            auto &path_params = boost::get<std::map<std::string, std::string>>(params);
            std::string id = path_params["id"];
            std::string name = path_params["name"];
            res.body() = "/user/{id}/{name} with ID: " + id + " name:" + name;
            res.set(boost::beast::http::field::content_type, "text/plain");
            res.prepare_payload();
            co_return;
        });

    server.add_route_rest_full(
        "GET",
        "/user/{name}/{id}/get",
        [](boost::beast::http::request<boost::beast::http::string_body> &req,
           boost::beast::http::response<boost::beast::http::string_body> &res,
           fast::db::SessionData &session,
           boost::variant<std::map<std::string, std::string>> &params)
            -> boost::asio::awaitable<void> {
            auto &path_params = boost::get<std::map<std::string, std::string>>(params);
            std::string id = path_params["id"];
            std::string name = path_params["name"];
            res.body() = "/user/{name}/{id}/get with ID: " + id + " name:" + name;
            res.set(boost::beast::http::field::content_type, "text/plain");
            res.prepare_payload();
            co_return;
        });

    server.add_route_rest_full(
        "GET",
        "/user/boost/{name}/{id}/get",
        [](boost::beast::http::request<boost::beast::http::string_body> &req,
           boost::beast::http::response<boost::beast::http::string_body> &res,
           fast::db::SessionData &session,
           boost::variant<std::map<std::string, std::string>> &params)
            -> boost::asio::awaitable<void> {
            auto &path_params = boost::get<std::map<std::string, std::string>>(params);
            std::string id = path_params["id"];
            std::string name = path_params["name"];
            res.body() = "/user/boost/{name}/{id}/get with ID: " + id + " name:" + name;
            res.set(boost::beast::http::field::content_type, "text/plain");
            res.prepare_payload();
            co_return;
        });
    BS::thread_pool<4> bs_pool;
    bs_pool.detach_task([&server]() { server.run(); });
    bs_pool.detach_task([&db_ctx, &pool]() {
        db_ctx.run();
        pool.cancel();
    });

    bs_pool.wait();

    /*std::vector<std::thread> threads;
    // threads.emplace_back([&ioc]() { ioc.run(); });
    threads.emplace_back([&db_ctx, &pool]() {
        db_ctx.run();
        pool.cancel();
    });*/

    // 测试 /hello 路由
    /*
    {
    boost::asio::co_spawn(
        ioc, [&server]() ->  boost::asio::awaitable<void> { co_await server.run();  },  boost::asio::detached);


         boost::asio::ip::tcp::resolver resolver(ioc);
        auto const results = resolver.resolve("127.0.0.1", "8080");
        boost::beast::tcp_stream stream(ioc);
        stream.connect(results);

        boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get, "/hello", 11};
        req.set(boost::beast::http::field::host, "127.0.0.1");
        req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        boost::beast::http::write(stream, req);

        boost::beast::flat_buffer buffer;
        boost::beast::http::response<boost::beast::http::string_body> res;
        boost::beast::http::read(stream, buffer, res);

        CHECK(res.result() == boost::beast::http::status::ok);
        CHECK(res.body() == "Hello, World!");

        boost::beast::error_code ec;
        stream.socket().shutdown( boost::asio::ip::tcp::socket::shutdown_both, ec);
    }

    // 测试 /user/{id} 路由
    {
         boost::asio::ip::tcp::resolver resolver(ioc);
        auto const results = resolver.resolve("127.0.0.1", "8080");
        boost::beast::tcp_stream stream(ioc);
        stream.connect(results);

        boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get, "/user/123", 11};
        req.set(boost::beast::http::field::host, "127.0.0.1");
        req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        boost::beast::http::write(stream, req);

        boost::beast::flat_buffer buffer;
        boost::beast::http::response<boost::beast::http::string_body> res;
        boost::beast::http::read(stream, buffer, res);

        CHECK(res.result() == boost::beast::http::status::ok);
        CHECK(res.body() == "User ID: 123");

        boost::beast::error_code ec;
        stream.socket().shutdown( boost::asio::ip::tcp::socket::shutdown_both, ec);
    }
    */
}

boost::cobalt::task<void> work_sql(fast::db::Database &db)
{
    fast::db::BlogArticle blog_article;
    co_await db.get_article_by_article_id(1, blog_article);
    LOG_INFO_R << blog_article;
    std::cout << blog_article;
    std::vector<fast::db::BlogColumn> info;
    co_await db.get_all_blog_columns(info);

    std::vector<fast::db::BlogColumn> out;
    build_column_tree_recursive(info, out);

    boost::json::value to_json = columns_to_json(out);
    LOG_INFO_R << to_json;

    /*std::vector<BlogColumn> out;
    build_column_tree_recursive(info,out);*/
    /*
    auto to_json = columns_to_json(out);
    LOG_INFO_R << to_json;*/
}

boost::cobalt::task<int> work2();
DOCTEST_TEST_CASE("blog_cat_test")
{
    asio::io_context ctx;
    fast::db::OptionParams
        params{"127.0.0.1", 3306, "root", "12345678", "blog"}; // 替换为你的数据库配置
    boost::mysql::connection_pool pool(ctx, fast::db::Database::createMysqlOption(params));
    fast::db::Database db(pool);

    auto cpl = [&](std::exception_ptr ep) {
        if (ep)
            try {
                std::rethrow_exception(ep);
            } catch (std::exception &e) {
                pool.cancel();
                printf("Completed with exception %s\n", e.what());
            }
    };
    boost::cobalt::spawn(ctx, work_sql(db), cpl);
    ctx.run();
    pool.cancel();
}

struct employee
{
    std::string first_name;
    std::string last_name;
};

asio::awaitable<std::string> get_employee_details(
    mysql::connection_pool &pool, std::int64_t employee_id)
{
    //[tutorial_connection_pool_get_connection_timeout
    // Get a connection from the pool.
    // This will wait until a healthy connection is ready to be used.
    // pooled_connection grants us exclusive access to the connection until
    // the object is destroyed.
    // Fail the operation if no connection becomes available in the next 20 seconds.
    mysql::pooled_connection conn = co_await pool.async_get_connection(
        asio::cancel_after(std::chrono::seconds(1)));

    //]
    /*mysql::static_results<mysql::pfr_by_name<employee>> result;
  co_await conn->async_execute(
         mysql::with_params("SELECT first_name, last_name FROM employee WHERE id = {}", employee_id),
         result
     );*/

    //[tutorial_connection_pool_use
    // Use the connection normally to query the database.
    // operator-> returns a reference to an any_connection,
    // so we can apply all what we learnt in previous tutorials
    /*mysql::static_results<mysql::pfr_by_name<employee>> result;
    co_await conn->async_execute(
        mysql::with_params("SELECT first_name, last_name FROM employee WHERE id = {}", employee_id),
        result
    );
    //]

    // Compose the message to be sent back to the client
    if (result.rows().empty())
    {
        co_return "NOT_FOUND";
    }
    else
    {
        const auto& emp = result.rows()[0];
        co_return emp.first_name + ' ' + emp.last_name;
    }*/

    // When the pooled_connection is destroyed, the connection is returned
    // to the pool, so it can be re-used.
    co_return "123";
}

void main_impl(int argc, char **argv)
{
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0]
                  << " <username> <password> <server-hostname> <listener-port>\n";
        exit(1);
    }

    const char *username = argv[1];
    const char *password = argv[2];
    const char *server_hostname = argv[3];
    auto listener_port = static_cast<unsigned short>(std::stoi(argv[4]));

    //[tutorial_connection_pool_main
    //[tutorial_connection_pool_create
    // Create an I/O context, required by all I/O objects
    asio::io_context ctx;

    // pool_params contains configuration for the pool.
    // You must specify enough information to establish a connection,
    // including the server address and credentials.
    // You can configure a lot of other things, like pool limits
    mysql::pool_params params;
    params.server_address.emplace_host_and_port(server_hostname);
    params.username = username;
    params.password = password;
    params.database = "boost_mysql_examples";

    // Construct the pool.
    // ctx will be used to create the connections and other I/O objects
    mysql::connection_pool pool(ctx, std::move(params));
    //]

    //[tutorial_connection_pool_run
    // You need to call async_run on the pool before doing anything useful with it.
    // async_run creates connections and keeps them healthy. It must be called
    // only once per pool.
    // The detached completion token means that we don't want to be notified when
    // the operation ends. It's similar to a no-op callback.
    pool.async_run(asio::detached);
    //]

    //[tutorial_connection_pool_signals
    // signal_set is an I/O object that allows waiting for signals
    asio::signal_set signals(ctx, SIGINT, SIGTERM);

    // Wait for signals
    signals.async_wait([&](boost::system::error_code, int) {
        // Stop the execution context. This will cause io_context::run to return
        ctx.stop();
    });
    //]

    // Calling run will actually execute the coroutine until completion
    ctx.run();
    //]
}