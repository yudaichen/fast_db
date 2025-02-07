/*
 * Copyright (c) 2022, Alibaba Group Holding Limited;
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef ASYNC_SIMPLE_DEMO_ASIO_CORO_UTIL_H
#define ASYNC_SIMPLE_DEMO_ASIO_CORO_UTIL_H

/*#include <boost/asio.hpp>*/
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>       // For async_read
#include <boost/asio/read_until.hpp> // For async_read_until
// #include <mysql-boost-1.87.0/include/boost/mysql.hpp>

#include <../thirdparty/mysql-boost-1.87.0/include/boost/mysql/connect_params.hpp>
#include <../thirdparty/mysql-boost-1.87.0/include/boost/mysql/connection.hpp>
#include <../thirdparty/mysql-boost-1.87.0/include/boost/mysql/connection_pool.hpp>
#include <../thirdparty/mysql-boost-1.87.0/include/boost/mysql/error_with_diagnostics.hpp>
#include <../thirdparty/mysql-boost-1.87.0/include/boost/mysql/pfr.hpp>
#include <../thirdparty/mysql-boost-1.87.0/include/boost/mysql/pool_params.hpp>
#include <../thirdparty/mysql-boost-1.87.0/include/boost/mysql/static_results.hpp>
#include <../thirdparty/mysql-boost-1.87.0/include/boost/mysql/tcp.hpp>
#include <../thirdparty/mysql-boost-1.87.0/include/boost/mysql/tcp_ssl.hpp>
#include <../thirdparty/mysql-boost-1.87.0/include/boost/mysql/with_params.hpp>
// #include <mysql-boost-1.87.0/include/boost/mysql/tcp_ssl.hpp>
#include <chrono>
// #include <coroutine>
#include <bits/random.h>
#include <expected>
#include <future>
#include <iostream>
#include <nlohmann/json.hpp>
#include <regex>
#include <string>
#include <utility>

#include <async_simple.hpp>

#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>

/*namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;*/

import stl;
namespace fast::db {
using namespace boost;
// std::coroutine_handle<> std_handle; // 使用标准库的 coroutine_handle
// async_simple::coro::CoroHandle<> async_handle;
// inline std::coroutine_handle<> std_handle;

class AsioExecutor : public async_simple::Executor {
public:
  AsioExecutor(asio::io_context &io_context) : io_context_(io_context) {}

  virtual bool schedule(Func func) override {
    asio::post(io_context_, std::move(func));
    return true;
  }

private:
  asio::io_context &io_context_;
};

template <typename T>
  requires(!std::is_reference<T>::value)
struct AsioCallbackAwaiter {
public:
  using CallbackFunction =
      std::function<void(std::coroutine_handle<>, std::function<void(T)>)>;

  AsioCallbackAwaiter(CallbackFunction callback_function)
      : callback_function_(std::move(callback_function)) {}

  bool await_ready() noexcept { return false; }

  void await_suspend(std::coroutine_handle<> handle) {
    callback_function_(handle, [this](T t) { result_ = std::move(t); });
  }

  auto coAwait(async_simple::Executor *executor) noexcept {
    return std::move(*this);
  }

  T await_resume() noexcept { return std::move(result_); }

private:
  CallbackFunction callback_function_;
  T result_;
};

/******************************* asio
 * *******************************************/
inline async_simple::coro::Lazy<std::error_code>
async_accept_lazy(asio::ip::tcp::acceptor &acceptor,
                  asio::ip::tcp::socket &socket) noexcept {
  co_return co_await AsioCallbackAwaiter<std::error_code>{
      [&](std::coroutine_handle<> handle, auto set_resume_value) {
        acceptor.async_accept(
            socket, [handle, set_resume_value =
                                 std::move(set_resume_value)](auto ec) mutable {
              set_resume_value(std::move(ec));
              handle.resume();
            });
      }};
}

template <typename Socket, typename AsioBuffer>
inline async_simple::coro::Lazy<std::pair<std::error_code, size_t>>
async_read_some_lazy(Socket &socket, AsioBuffer &&buffer) noexcept {
  co_return co_await AsioCallbackAwaiter<std::pair<std::error_code, size_t>>{
      [&](std::coroutine_handle<> handle, auto set_resume_value) mutable {
        socket.async_read_some(
            std::move(buffer),
            [handle, set_resume_value = std::move(set_resume_value)](
                auto ec, auto size) mutable {
              set_resume_value(std::make_pair(std::move(ec), size));
              handle.resume();
            });
      }};
}

template <typename Socket, typename AsioBuffer>
inline async_simple::coro::Lazy<std::pair<std::error_code, size_t>>
async_read_lazy(Socket &socket, AsioBuffer &buffer) noexcept {
  co_return co_await AsioCallbackAwaiter<std::pair<std::error_code, size_t>>{
      [&](std::coroutine_handle<> handle, auto set_resume_value) mutable {
        boost::asio::async_read(
            socket, buffer,
            [handle, set_resume_value = std::move(set_resume_value)](
                auto ec, auto size) mutable {
              set_resume_value(std::make_pair(std::move(ec), size));
              handle.resume();
            });
      }};
}

template <typename Socket, typename AsioBuffer>
inline async_simple::coro::Lazy<std::pair<std::error_code, size_t>>
async_read_until_lazy(Socket &socket, AsioBuffer &buffer,
                      asio::string_view delim) noexcept {
  co_return co_await AsioCallbackAwaiter<std::pair<std::error_code, size_t>>{
      [&](std::coroutine_handle<> handle, auto set_resume_value) mutable {
        boost::asio::async_read_until(
            socket, buffer, delim,
            [handle, set_resume_value = std::move(set_resume_value)](
                auto ec, auto size) mutable {
              set_resume_value(std::make_pair(std::move(ec), size));
              handle.resume();
            });
      }};
}

template <typename Socket, typename AsioBuffer>
inline async_simple::coro::Lazy<std::pair<std::error_code, size_t>>
async_write_lazy(Socket &socket, AsioBuffer &&buffer) noexcept {
  co_return co_await AsioCallbackAwaiter<std::pair<std::error_code, size_t>>{
      [&](std::coroutine_handle<> handle, auto set_resume_value) mutable {
        boost::asio::async_write(
            socket, std::move(buffer),
            [handle, set_resume_value = std::move(set_resume_value)](
                auto ec, auto size) mutable {
              set_resume_value(std::make_pair(std::move(ec), size));
              handle.resume();
            });
      }};
}

/******************************* mysql ****************************************/

inline async_simple::coro::Lazy<std::error_code>
async_connect_lazy(asio::io_context &io_context, asio::ip::tcp::socket &socket,
                   const std::string &host, const std::string &port) noexcept {
  co_return co_await AsioCallbackAwaiter<std::error_code>{
      [&](std::coroutine_handle<> handle, auto set_resume_value) mutable {
        asio::ip::tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(host, port);
        boost::asio::async_connect(
            socket, endpoints,
            [handle, set_resume_value = std::move(set_resume_value)](
                auto ec, auto size) mutable {
              set_resume_value(std::move(ec));
              handle.resume();
            });
      }};
}

template <typename Error>
inline async_simple::coro::Lazy<std::error_code>
mysql_async_connect_lazy(mysql::tcp_connection conn, const std::string &host,
                         int port, std::string user_name, std::string password,
                         std::string database, Error &&e) noexcept {
  co_return co_await AsioCallbackAwaiter<std::error_code>{
      [&](std::coroutine_handle<> handle, auto set_resume_value) mutable {
        mysql::connect_params params;
        params.server_address.emplace_host_and_port(std::string(host));
        params.username = user_name;
        params.password = password;

        // Connect to the server
        conn.async_connect(
            params, e,
            [handle, set_resume_value = std::move(set_resume_value)](
                auto ec, auto size) mutable {
              set_resume_value(std::move(ec));
              handle.resume();
            });
      }};
}

/*
template <typename Result>
inline async_simple::coro::Lazy<
    std::pair<std::error_code, boost::mysql::resultset>>
async_execute_string_lazy(boost::mysql::pooled_connection &conn,
                          std::string &&stmt, Result &result) noexcept {
  co_return co_await AsioCallbackAwaiter<
      std::pair<std::error_code, boost::mysql::resultset>>{
      [&](std::coroutine_handle<> handle, auto set_resume_value) {
        conn.get().async_execute(
            stmt, result,
            [handle, set_resume_value = std::move(set_resume_value)](
                auto ec, Result &&rs) mutable {
              set_resume_value(std::make_pair(std::move(ec), std::move(rs)));
              handle.resume();
            });
      }};
}*/

/*
template <typename T>
inline async_simple::coro::Lazy<
    std::pair<boost::mysql::error_code, boost::mysql::resultset>>
async_execute_lazy(boost::mysql::pooled_connection &conn, T &&stmt,
                   boost::mysql::results &result, boost::mysql::diagnostics
&diag) noexcept { co_return co_await AsioCallbackAwaiter<
      std::pair<boost::mysql::error_code, boost::mysql::resultset>>{
      [&](std::coroutine_handle<> handle, auto set_resume_value) {
        conn.get().async_execute(
            stmt, result, diag,
            [handle, set_resume_value = std::move(set_resume_value)](
                auto ec, mysql::results &&rs) mutable {
              set_resume_value(std::make_pair(std::move(ec), std::move(rs)));
              handle.resume();
            });
      }};
}*/

template <typename T, typename Result>
inline async_simple::coro::Lazy<std::pair<boost::mysql::error_code, Result>>
async_execute_lazy(boost::mysql::pooled_connection &conn, T &&stmt,
                   Result &result, mysql::diagnostics &diagnostics) noexcept {
  co_return co_await AsioCallbackAwaiter<
      std::pair<boost::mysql::error_code, Result>>{
      [&](std::coroutine_handle<> handle, auto set_resume_value) {
        conn->async_execute(
            stmt, result, diagnostics,
            [handle, set_resume_value = std::move(set_resume_value)](
                auto ec, Result &&rs) mutable {
              set_resume_value(std::make_pair(std::move(ec), std::move(rs)));
              handle.resume();
            });
      }};
} /*
 template <typename T>
 inline async_simple::coro::Lazy<std::pair<boost::mysql::error_code,
 boost::mysql::resultset>> async_execute_lazy(boost::mysql::pooled_connection&
 conn, T&& stmt, boost::mysql::diagnostics& diag) noexcept { co_return co_await
 AsioCallbackAwaiter<std::pair<boost::mysql::error_code,
 boost::mysql::resultset>>{
     [&](std::coroutine_handle<> handle, auto set_resume_value) {
       boost::mysql::resultset result; // *关键：在 lambda 内部创建 resultset*

       conn.async_execute( // 使用 .get() 访问底层连接
           std::forward<T>(stmt), // 转发语句，避免不必要的拷贝
           result,                // 将 resultset 传递给 async_execute
           diag,                // 传递诊断对象
           [handle, set_resume_value = std::move(set_resume_value), result =
 std::move(result)](boost::system::error_code ec) mutable {
               // *关键：通过移动捕获 resultset*
               set_resume_value(std::make_pair(std::move(ec),
 std::move(result))); handle.resume();
           });
     }};
 }
 */

inline async_simple::coro::Lazy<
    std::pair<std::error_code, mysql::pooled_connection>>
async_get_connection_lazy(boost::mysql::connection_pool &pool) noexcept {
  co_return co_await AsioCallbackAwaiter<
      std::pair<std::error_code, mysql::pooled_connection>>{
      [&](std::coroutine_handle<> handle, auto set_resume_value) {
        pool.async_get_connection(
            [handle, set_resume_value = std::move(set_resume_value)](
                auto ec, mysql::pooled_connection &&rs) mutable {
              set_resume_value(std::make_pair(std::move(ec), std::move(rs)));
              handle.resume();
            });
      }};
}

inline async_simple::coro::Lazy<std::error_code>
async_close_lazy(boost::mysql::tcp_connection &conn) noexcept {
  co_return co_await AsioCallbackAwaiter<std::error_code>{
      [&](std::coroutine_handle<> handle, auto set_resume_value) {
        conn.async_close([handle, set_resume_value = std::move(
                                      set_resume_value)](auto ec) mutable {
          set_resume_value(std::move(ec));
          handle.resume();
        });
      }};
}

inline async_simple::coro::Lazy<std::error_code>
async_ping_lazy(boost::mysql::tcp_connection &conn) noexcept {
  co_return co_await AsioCallbackAwaiter<std::error_code>{
      [&](std::coroutine_handle<> handle, auto set_resume_value) {
        conn.async_ping([handle, set_resume_value = std::move(
                                     set_resume_value)](auto ec) mutable {
          set_resume_value(std::move(ec));
          handle.resume();
        });
      }};
}

template <typename Duration>
async_simple::coro::Lazy<void> sleep(asio::io_context &io_context,
                                     Duration duration) {
  co_await boost::asio::steady_timer{io_context.get_executor(), duration}
      .async_wait(boost::asio::use_awaitable);
}

/******************************* beast
 * *******************************************/

// 封装 http::async_read
template <typename Stream, typename BufferSequence, typename Request>
inline async_simple::coro::Lazy<std::pair<std::error_code, size_t>>
async_http_read_lazy(Stream &stream, BufferSequence buffers,
                     Request &req) noexcept {
  co_return co_await AsioCallbackAwaiter<std::pair<std::error_code, size_t>>{
      [&](std::coroutine_handle<> handle, auto set_resume_value) mutable {
        beast::http::async_read(
            stream, buffers, req,
            [handle, set_resume_value = std::move(set_resume_value)](
                auto ec, auto bytes_transferred) mutable {
              set_resume_value(
                  std::make_tuple(std::move(ec), bytes_transferred));
              handle.resume();
            });
      }};
}

// 封装 ssl::stream 的异步握手
template <typename Stream, typename Handshake_type>
inline async_simple::coro::Lazy<std::error_code>
async_ssl_handshake_lazy(Stream &stream, Handshake_type &type) noexcept {
  co_return co_await AsioCallbackAwaiter<std::error_code>{
      [&](std::coroutine_handle<> handle, auto set_resume_value) mutable {
        stream.async_handshake(
            type, [handle, set_resume_value =
                               std::move(set_resume_value)](auto ec) mutable {
              set_resume_value(std::move(ec));
              handle.resume();
            });
      }};
}

// 封装 http::async_write
template <typename Stream, typename Response>
inline async_simple::coro::Lazy<std::pair<std::error_code, size_t>>
async_http_write_lazy(Stream &stream, Response &res) noexcept {
  co_return co_await AsioCallbackAwaiter<std::pair<std::error_code, size_t>>{
      [&](std::coroutine_handle<> handle, auto set_resume_value) mutable {
        beast::http::async_write(
            stream, res,
            [handle, set_resume_value = std::move(set_resume_value)](
                auto ec, auto bytes_transferred) mutable {
              set_resume_value(
                  std::make_tuple(std::move(ec), bytes_transferred));
              handle.resume();
            });
      }};
}

// 我们自己的 PoolParams 结构体
class PoolParams {

public:
  struct OptionParams {
    std::string host    = "localhost";
    unsigned short port = 3306;
    std::string username;
    std::string password;
    std::string database;
    boost::mysql::ssl_mode ssl{boost::mysql::ssl_mode::disable};
    bool multi_queries{true};
    std::size_t initial_size{1};
    std::size_t max_size{5};
    std::chrono::seconds connect_timeout{20};
    std::chrono::seconds retry_interval{30};
    std::chrono::hours ping_interval{1};
    std::chrono::seconds ping_timeout{10};
    bool thread_safe{true};
  };

  static mysql::pool_params createMysqlOption(const OptionParams &params) {
    mysql::pool_params mysql_params;
    mysql_params.server_address.emplace_host_and_port(params.host, params.port);
    mysql_params.username        = params.username;
    mysql_params.password        = params.password;
    mysql_params.database        = params.database;
    mysql_params.ssl             = params.ssl;
    mysql_params.multi_queries   = params.multi_queries;
    mysql_params.initial_size    = params.initial_size;
    mysql_params.max_size        = params.max_size;
    mysql_params.connect_timeout = params.connect_timeout;
    mysql_params.retry_interval  = params.retry_interval;
    mysql_params.ping_interval   = params.ping_interval;
    mysql_params.ping_timeout    = params.ping_timeout;
    mysql_params.thread_safe     = params.thread_safe;
    return mysql_params;
  }

  static void createPool(asio::io_context &io_context,
                         const OptionParams &params) {
    auto pool_params = createMysqlOption(params);
    mysql::connection_pool pool_(io_context, std::move(pool_params));
    // PoolParams::GetInstance();
  }

  /*
  static mysql::connection_pool pool_;

  static void init(mysql::connection_pool pool) { pool_ = std::move(pool);
  }*/
};

} // namespace fast::db

// namespace mysql_utils

#endif
