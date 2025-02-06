//import stl;
#include <doctest.h>
#include "utils/log.h"
#include "db/boost_head.h"

#include <iostream>

using namespace std;
using namespace boost;
using namespace boost::asio;

using boost::asio::ip::tcp;


// Awaitable类，用于封装Asio的异步操作
template<typename TResult>
class AsioAwaiter {
public:
    using CallbackType = std::function<void(std::coroutine_handle<>, std::error_code, TResult)>;

    AsioAwaiter(asio::io_context& io_context, CallbackType callback)
        : io_context_(io_context), callback_(std::move(callback)) {}

    bool await_ready() const noexcept { return false; }

    void await_suspend(std::coroutine_handle<> handle) {
        callback_(handle, ec_, result_);
    }

    TResult await_resume() {
        if (ec_) {
            throw std::system_error(ec_);
        }
        return std::move(result_);
    }

private:
    asio::io_context& io_context_;
    CallbackType callback_;
    std::error_code ec_;
    TResult result_;
};

// 特化void类型的Awaitable类
template<>
class AsioAwaiter<void> {
public:
    using CallbackType = std::function<void(std::coroutine_handle<>, std::error_code)>;

    AsioAwaiter(asio::io_context& io_context, CallbackType callback)
        : io_context_(io_context), callback_(std::move(callback)) {}

    bool await_ready() const noexcept { return false; }

    void await_suspend(std::coroutine_handle<> handle) {
        callback_(handle, ec_);
    }

    void await_resume() {
        if (ec_) {
            throw std::system_error(ec_);
        }
    }

private:
    asio::io_context& io_context_;
    CallbackType callback_;
    std::error_code ec_;
};

/*// 异步连接函数，返回cppcoro::task
cppcoro::task<void> async_connect(
    asio::io_context& io_context, tcp::socket& socket,
    const std::string& host, const std::string& port) {

    co_await AsioAwaiter<void>{
        io_context,
        [&](std::coroutine_handle<> handle, std::error_code ec) {
            tcp::resolver resolver(io_context);
            auto endpoints = resolver.resolve(host, port);
            asio::async_connect(socket, endpoints,
                [handle](std::error_code ec, tcp::endpoint) mutable {
                    auto coro_handle = std::coroutine_handle<cppcoro::task<void>::promise_type>::from_address(handle.address());
                    if (ec) {
                        try {
                            throw std::system_error(ec);  // 抛出异常
                        } catch (...) {
                            coro_handle.promise().unhandled_exception();  // 捕获当前异常
                        }
                    } else {
                        coro_handle.promise().return_void();  // 任务成功完成
                    }
                    coro_handle.resume();
                });
        }
    };

    std::cout << "Connected to server!" << std::endl;
}*/

asio::awaitable<void> connect_to_server(const std::string &host,
                                        const std::string &port) {
  ip::tcp::resolver resolver(co_await asio::this_coro::executor);

  const auto endpoints =
      co_await resolver.async_resolve(host, port, asio::use_awaitable);

  ip::tcp::socket socket(co_await asio::this_coro::executor);
  co_await asio::async_connect(socket, endpoints, asio::use_awaitable);

  std::cout << "Connected to the server!" << std::endl;

  socket.close();
}

// 使用cppcoro::task和asio::use_awaitable进行异步连接
/*cppcoro::task<void> connect_to_server(cppcoro::io_service &io_service,
                                      const std::string &host,
                                      const std::string &port) {
  asio::io_context &io_context = io_service.get_io_context();

  ip::tcp::resolver resolver(io_context);
  auto endpoints =
      co_await resolver.async_resolve(host, port, asio::use_awaitable);

  ip::tcp::socket socket(io_context);
  co_await asio::async_connect(socket, endpoints, asio::use_awaitable);

  std::cout << "Connected to the server!" << std::endl;

  socket.close();
}*/

DOCTEST_TEST_CASE("test_cppcoro,simple") {

}