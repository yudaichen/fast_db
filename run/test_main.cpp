/**
 *               ii.                                         ;9ABH,
 *              SA391,                                    .r9GG35&G
 *              &#ii13Gh;                               i3X31i;:,rB1
 *              iMs,:,i5895,                         .5G91:,:;:s1:8A
 *               33::::,,;5G5,                     ,58Si,,:::,sHX;iH1
 *                Sr.,:;rs13BBX35hh11511h5Shhh5S3GAXS:.,,::,,1AG3i,GG
 *                .G51S511sr;;iiiishS8G89Shsrrsh59S;.,,,,,..5A85Si,h8
 *               :SB9s:,............................,,,.,,,SASh53h,1G.
 *            .r18S;..,,,,,,,,,,,,,,,,,,,,,,,,,,,,,....,,.1H315199,rX,
 *          ;S89s,..,,,,,,,,,,,,,,,,,,,,,,,....,,.......,,,;r1ShS8,;Xi
 *        i55s:.........,,,,,,,,,,,,,,,,.,,,......,.....,,....r9&5.:X1
 *       59;.....,.     .,,,,,,,,,,,...        .............,..:1;.:&s
 *      s8,..;53S5S3s.   .,,,,,,,.,..      i15S5h1:.........,,,..,,:99
 *      93.:39s:rSGB@A;  ..,,,,.....    .SG3hhh9G&BGi..,,,,,,,,,,,,.,83
 *      G5.G8  9#@@@@@X. .,,,,,,.....  iA9,.S&B###@@Mr...,,,,,,,,..,.;Xh
 *      Gs.X8 S@@@@@@@B:..,,,,,,,,,,. rA1 ,A@@@@@@@@@H:........,,,,,,.iX:
 *     ;9. ,8A#@@@@@@#5,.,,,,,,,,,... 9A. 8@@@@@@@@@@M;    ....,,,,,,,,S8
 *     X3    iS8XAHH8s.,,,,,,,,,,...,..58hH@@@@@@@@@Hs       ...,,,,,,,:Gs
 *    r8,        ,,,...,,,,,,,,,,.....  ,h8XABMMHX3r.          .,,,,,,,.rX:
 *   :9, .    .:,..,:;;;::,.,,,,,..          .,,.               ..,,,,,,.59
 *  .Si      ,:.i8HBMMMMMB&5,....                    .            .,,,,,.sMr
 *  SS       :: h@@@@@@@@@@#; .                     ...  .         ..,,,,iM5
 *  91  .    ;:.,1&@@@@@@MXs.                            .          .,,:,:&S
 *  hS ....  .:;,,,i3MMS1;..,..... .  .     ...                     ..,:,.99
 *  ,8; ..... .,:,..,8Ms:;,,,...                                     .,::.83
 *   s&: ....  .sS553B@@HX3s;,.    .,;13h.                            .:::&1
 *    SXr  .  ...;s3G99XA&X88Shss11155hi.                             ,;:h&,
 *     iH8:  . ..   ,;iiii;,::,,,,,.                                 .;irHA
 *      ,8X5;   .     .......                                       ,;iihS8Gi
 *         1831,                                                 .,;irrrrrs&@
 *           ;5A8r.                                            .:;iiiiirrss1H
 *             :X@H3s.......                                .,:;iii;iiiiirsrh
 *              r#h:;,...,,.. .,,:;;;;;:::,...              .:;;;;;;iiiirrss1
 *             ,M8 ..,....,.....,,::::::,,...         .     .,;;;iiiiiirss11h
 *             8B;.,,,,,,,.,.....          .           ..   .:;;;;iirrsss111h
 *            i@5,:::,,,,,,,,.... .                   . .:::;;;;;irrrss111111
 *            9Bi,:,,,,......                        ..r91;;;;;iirrsss1ss1111


                     狗头保佑                              永无BUG

 */

// #include "gtest/gtest.h"
#include "utils/log.h"
// #include "dbg.h"
#include "redis/redis_asio.hpp"
#include <coroutine>

#include "doctest.h"

#include "db/boost_head.h"
#include <map>

//import stl;
import mysql_connect;
import small_utils;
import beast_net_server;
import BS.thread_pool;
import string_util;

void test()
{
    try {
        throw std::runtime_error("Something went wrong");
    } catch (...) {
        // 使用Boost Stacktrace捕获当前栈信息
        std::stringstream ss;
        ss << boost::stacktrace::stacktrace();

        // 将栈信息输出到控制台
        std::cout << "Caught exception, stack trace:" << ss.str() << std::endl;
    }
}

std::string create_json_ok_response(boost::json::value to_json, int code = 200)
{
    boost::json::object obj;
    obj["status"] = "ok";
    obj["code"] = code;
    obj["message"] = to_json;
    return boost::json::serialize(obj);
}

// 封装一个函数来创建错误的 JSON 响应
std::string create_json_error_response(const std::string &message, int code = 500)
{
    boost::json::object obj;
    obj["status"] = "error";
    obj["code"] = code;
    obj["message"] = message;
    return boost::json::serialize(obj);
}

void run_server()
{
    boost::asio::io_context ioc;
    //executor_type ioc;
    asio::io_context db_ctx(2);
    fast::db::OptionParams
        params{"127.0.0.1", 3306, "root", "12345678", "blog"}; // 替换为你的数据库配置
    boost::mysql::connection_pool pool(db_ctx, fast::db::Database::createMysqlOption(params));

    fast::db::Server server(
        boost::asio::ip::make_address("127.0.0.1"),
        8080,
        "/mnt/h/study/cpp/fast_db_master/cmake-build-debug-wsl24/",
        2,
        fast::db::Server::Protocol::HTTP,
        pool);

    server.add_route(
        "GET",
        "/get_blog_columns",
        [](boost::beast::http::request<boost::beast::http::string_body> &req,
           boost::beast::http::response<boost::beast::http::string_body> &res,
           fast::db::SessionData &session) -> boost::asio::awaitable<void> { // 修改 session 参数类型
            fast::db::Database *database = session.get_database();
            std::vector<fast::db::BlogColumn> columns;

            co_await database->get_all_blog_columns_awaitable(columns);

            std::vector<fast::db::BlogColumn> out_columns;
            build_column_tree_recursive(columns, out_columns);
            boost::json::value to_json = columns_to_json(out_columns);
            res.body() = create_json_ok_response(to_json);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.prepare_payload();
            co_return;
        });

    server.add_route_rest_full(
        "GET",
        "/get_all_blog/{id}/{page}/{page_size}",
        [](boost::beast::http::request<boost::beast::http::string_body> &req,
           boost::beast::http::response<boost::beast::http::string_body> &res,
           fast::db::SessionData &session,
           boost::variant<std::map<std::string, std::string>> &params)
            -> boost::asio::awaitable<void> {
            auto &path_params = boost::get<std::map<std::string, std::string>>(params);
            std::string id = path_params["id"];
            std::string page = path_params["page"];
            std::string page_size = path_params["page_size"];
            fast::db::Database *database = session.get_database();
            std::vector<fast::db::BlogArticle> blogs;

            co_await database->get_articles_by_column_id_awaitable(
                std::stoull(id), std::stoi(page), std::stoi(page_size), blogs);

            boost::json::value to_json = vec_article_to_json(blogs);
            res.body() = create_json_ok_response(to_json);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.prepare_payload();
            co_return;
        });

    server.add_route_rest_full(
        "POST",
        "/blog_full_text/{page}/{page_size}",
        [](boost::beast::http::request<boost::beast::http::string_body> &req,
           boost::beast::http::response<boost::beast::http::string_body> &res,
           fast::db::SessionData &session,
           boost::variant<std::map<std::string, std::string>> &params)
            -> boost::asio::awaitable<void> {
            auto &path_params = boost::get<std::map<std::string, std::string>>(params);

            std::string basic_string = req.body();
            boost::json::value json_value = boost::json::parse(basic_string);
            if (!json_value.is_object()) {
                std::cerr << "blog_full_text Error: JSON is not an object." << std::endl;
                co_return;
            }
            boost::json::object json_obj = json_value.as_object();
            std::string blog_full_text = json_obj.at("blog_full_text").as_string().c_str();
            if (blog_full_text.size() == 0) {
                std::cerr << "blog_full_text Error: JSON is not an empty string." << std::endl;
                co_return;
            }

            std::string page = path_params["page"];
            std::string page_size = path_params["page_size"];
            fast::db::Database *database = session.get_database();

            std::vector<fast::db::BlogArticle> blogs;
            co_await database->get_articles_full_text_awaitable(
                blog_full_text, std::stoi(page), std::stoi(page_size), blogs);

            boost::json::value to_json = vec_article_to_json(blogs);
            res.body() = create_json_ok_response(to_json);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.prepare_payload();
            co_return;
        });

    server.add_route_rest_full(
        "GET",
        "/get_one_blog/{id}",
        [](boost::beast::http::request<boost::beast::http::string_body> &req,
           boost::beast::http::response<boost::beast::http::string_body> &res,
           fast::db::SessionData &session,
           variant<std::map<std::string, std::string>> &params) -> boost::asio::awaitable<void> {
            auto &path_params = boost::get<std::map<std::string, std::string>>(params);
            std::string id = path_params["id"];

            fast::db::Database *database = session.get_database();

            fast::db::BlogArticle article;
            co_await database->get_article_by_article_id_awaitable(std::stoull(id), article);

            // 观看自增1
            if (article.article_id != 0) {
                co_await database->watch_number_added_awaitable(article.article_id);
            }

            boost::json::value to_json = fast::db::article_to_json(article);
            res.body() = create_json_ok_response(to_json);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.prepare_payload();
            co_return;
        });

    server.add_route_rest_full(
        "GET",
        "/user/{id}/{page}/{page_size}",
        [](boost::beast::http::request<boost::beast::http::string_body> &req,
           boost::beast::http::response<boost::beast::http::string_body> &res,
           fast::db::SessionData &session,
           boost::variant<std::map<std::string, std::string>> &params)
            -> boost::asio::awaitable<void> {
            auto &path_params = boost::get<std::map<std::string, std::string>>(params);
            std::string id = path_params["id"];
            std::string page = path_params["page"];
            std::string page_size = path_params["page_size"];
            res.body() = "/user/{name}/{id}/get with ID: " + id + " page:" + page
                         + " page_size:" + page_size;
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.prepare_payload();
            co_return;
        });

    BS::thread_pool<2> bs_pool;
    bs_pool.detach_task([&server]() { server.run(); });
    bs_pool.detach_task([&db_ctx, &pool]() {
        db_ctx.run();
        pool.cancel();
    });

    bs_pool.wait();
}

int main(int argc, char **argv)
{
    static fast::util::OnceToken
        tokenCreateStackTrace([&] { Log::GetInstance()->init(argv[0]); }, nullptr);

    run_server();
    /*redis_asio_connect();
  std::map<int, std::map<std::string, double>> nestedMap{
          {1, {{"a", 1.1}, {"b", 2.2}}},
          {2, {{"c", 3.3}, {"d", 4.4}}},
          {3, {{"e", 5.5}, {"f", 6.6}}}
  };


  dbg(nestedMap);*/
    // Set HTTP listener address and port
    // Load config file
    // drogon::app().loadConfigFile("../config.json");
    // Run HTTP framework,the method will block in the internal event loop
    /*testing::InitGoogleTest(&argc, argv);

  std::cout << "__cplusplus: " << __cplusplus << std::endl;
  std::cout << "__cpp_concepts: " << __cpp_concepts << std::endl;

  int result =  RUN_ALL_TESTS();
  if (result == 0) {
      // 处理内存分配失败的情况
  } else {
      // 使用分配的内存
  }*/

    doctest::Context context;
    context.applyCommandLine(argc, argv);
    // LOG_INFO_R << boost::stacktrace::stacktrace();
    int res = context.run(); // run doctest
    // important - query flags (and --exit) rely on the user doing this
    if (context.shouldExit()) {
        // propagate the result of the tests
    return res;
  }

    test();



    return 0;
}

