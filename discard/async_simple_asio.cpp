//import stl;
//import sys_utils;
//import queue_utils;
//import collect_utils;
//import robin_hood;
//import BS.thread_pool;

#include <boost/mysql/results.hpp>

#include <boost/mysql/statement.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/read_until.hpp>
//#include <boost/mysql.hpp>

#include "utils/log.h"


#include <doctest.h>

#include <iostream>
#include <coroutine>

//#include "async_simple/coro/Lazy.h"
//#include "async_simple/coro/SyncAwait.h"
#include <chrono>
#include <concepts>

//#include "asio_coro_util.hpp"
#include "db/MySqlBlog.hpp"
#include <async_simple.hpp>
using namespace std;
using namespace async_simple;


async_simple::coro::Lazy<void> start(boost::asio::io_context &io_context,
                                     std::string host, std::string port) {
    boost::asio::ip::tcp::socket socket(io_context);
    auto ec = co_await fast::db::async_connect_lazy(io_context, socket, host, port);
    if (ec) {
        std::cout << "Connect error: " << ec.message() << '\n';
        throw system_error(ec);
    }
    std::cout << "Connect to " << host << ":" << port << " successfully.\n";
    const int max_length = 1024;
    char write_buf[max_length] = {"hello async_simple"};
    char read_buf[max_length];
    const int count = 100000;
    for (int i = 0; i < count; ++i) {
        co_await fast::db::async_write_lazy(socket, boost::asio::buffer(write_buf, max_length));
        auto [error, reply_length] = co_await fast::db::async_read_some_lazy(
            socket, boost::asio::buffer(read_buf, max_length));
        /*if (reply_length == boost::asio::error::eof) {
          std::cout << "eof at message index: " << i << '\n';
          break;
        }
        if (error) {
          std::cout << "error: " << error.message() << ", message index: " << i
                    << '\n';
          throw system_error(error);
        }*/

        // handle read data as your wish.
    }

    std::cout << "Finished send and receive " << count
              << " messages, client will close.\n";
    std::error_code ignore_ec;
   // socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
    io_context.stop();
}


DOCTEST_TEST_CASE("asio_get_baidu") {
    try {
        boost::asio::io_context io_context;
        /*std::thread thd([&io_context] {
            boost::asio::io_context work(io_context);
            io_context.run();
        });

        async_simple::coro::syncAwait(start(io_context, "127.0.0.1", "9980"));
        io_context.stop();
        thd.join();*/
        std::cout << "Finished ok, client quit.\n";
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
  return;
}


