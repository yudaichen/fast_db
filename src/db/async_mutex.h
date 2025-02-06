
#ifndef ASYNC_MUTEX_H
#define ASYNC_MUTEX_H
#include <boost/asio.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/cobalt.hpp>
#include <queue>
#include <mutex>
#include <atomic>
#include <functional>
#include <variant>

#include <boost/asio.hpp>
#include <boost/cobalt.hpp>
#include <boost/cobalt/op.hpp>
#include <queue>
#include <mutex>
#include <atomic>
#include <functional>

#include <boost/cobalt/op.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <deque>
#include <optional>
#include <coroutine>

namespace fast {
namespace db {

class SpinLock {
public:
    explicit SpinLock(std::int32_t count = 1024) noexcept
        : _spinCount(count), _locked(false) {}

    bool tryLock() noexcept {
        return !_locked.exchange(true, std::memory_order_acquire);
    }

    boost::cobalt::task<int> coLock() noexcept {
        auto counter = _spinCount;
        while (!tryLock()) {
            while (_locked.load(std::memory_order_relaxed)) {
                if (counter-- <= 0) {
                    //co_await 0;
                    counter = _spinCount;
                }
            }
        }
        co_return 0;
       // co_return ;
    }

    void lock() noexcept {
        auto counter = _spinCount;
        while (!tryLock()) {
            while (_locked.load(std::memory_order_relaxed)) {
                if (counter-- <= 0) {
                    std::this_thread::yield();
                    counter = _spinCount;
                }
            }
        }
    }

    void unlock() noexcept { _locked.store(false, std::memory_order_release); }

   boost::cobalt::generator<std::unique_lock<SpinLock>> coScopedLock() noexcept {
        co_await coLock();
        co_return std::unique_lock<SpinLock>{*this, std::adopt_lock};
    }

private:
    std::int32_t _spinCount;
    std::atomic<bool> _locked;
};

class ScopedSpinLock {
public:
    explicit ScopedSpinLock(SpinLock &lock) : _lock(lock) { _lock.lock(); }
    ~ScopedSpinLock() { _lock.unlock(); }

private:
    ScopedSpinLock(const ScopedSpinLock &) = delete;
    ScopedSpinLock &operator=(const ScopedSpinLock &) = delete;
    SpinLock &_lock;
};

}  // namespace fast
}  // namespace db

#endif //ASYNC_MUTEX_H
