// module;
#ifndef FAST_MUSQL_CONNECTION_CONFIG
#define FAST_MUSQL_CONNECTION_CONFIG
// #include "asio_coro_util.hpp"
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>       // For async_read
#include <boost/asio/read_until.hpp> // For async_read_until
#include <boost/mysql.hpp>
#include <coroutine>
#include <expected>
#include <future>
#include <iostream>
#include <nlohmann/json.hpp>
#include <regex>
#include <string>
#include <utility>

import stl;

#include <../../test/async_simple.hpp>
// export module musql_connection_config;
// #include "db/asio_coro_util.hpp"
//  数据库连接配置
struct ConnectionConfig {
  std::string host;
  std::uint16_t port;
  std::string user;
  std::string password;
  std::string database;
  std::size_t pool_size;
};

// namespace fast::db {

// namespace mysql_connection_pool {

/*
class ConnectionPool {
public:
    ConnectionPool(size_t pool_size, const mysql::connect_params& params, size_t
io_threads = std::thread::hardware_concurrency()) : params_(params),
pool_size_(pool_size), io_threads_(io_threads) {
        io_contexts_.resize(io_threads_);
        io_threads_pool_.resize(io_threads_);
        connections_.resize(io_threads_);
        for (size_t i = 0; i < io_threads_; ++i) {
            connections_[i].resize(pool_size_);
            io_threads_pool_[i] = std::thread([this, i] { io_contexts_[i].run();
}); for (size_t j = 0; j < pool_size_; ++j) { connections_[i][j] =
std::make_unique<mysql::tcp_ssl_connection>(io_contexts_[i]);
            }
        }
    }

    ~ConnectionPool() {
         for (size_t i = 0; i < io_threads_; ++i) {
            io_contexts_[i].stop();
            io_threads_pool_[i].join();
        }
    }

    async_simple::coro::Lazy<std::expected<mysql::tcp_ssl_connection&,
mysql::error_code>> get_connection() { std::unique_lock<std::mutex>
lock(mutex_); cv_.wait(lock, [this]{ return !available_connections_.empty(); });

        auto [io_index, conn_index] = available_connections_.front();
        available_connections_.pop();
        co_return *connections_[io_index][conn_index];
    }

    void return_connection(size_t io_index, size_t conn_index) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            available_connections_.push({io_index, conn_index});
        }
        cv_.notify_one();
    }

    async_simple::coro::Lazy<std::expected<void, mysql::error_code>>
init_connections() { for (size_t i = 0; i < io_threads_; ++i) { for (size_t j =
0; j < pool_size_; ++j) { auto handshake_result = co_await
fast::db::async_handshake(*connections_[i][j], params_); if
(!handshake_result.has_value()) { std::cerr << "Handshake failed: " <<
handshake_result.error().message() << std::endl; co_return
std::unexpected(handshake_result.error());
                }
                available_connections_.push({i, j});
            }
        }
        co_return {};
    }

private:
    mysql::connect_params params_;
    size_t pool_size_;
    size_t io_threads_;
    std::vector<boost::asio::io_context> io_contexts_;
    std::vector<std::thread> io_threads_pool_;
    std::vector<std::vector<std::unique_ptr<mysql::tcp_ssl_connection>>>
connections_; std::mutex mutex_; std::condition_variable cv_;
    std::queue<std::pair<size_t, size_t>> available_connections_;
};
*/

#endif
