

//import stl;
#include "utils/log.h"
/*#include <coroutine>
#include <exception>
#include <future>
#include <list>
#include <stdexcept>
#include <thread>
#include <queue>*/
#include "doctest.h"
#include <coroutine>
#include <future>
#include <list>
#include <stdexcept>
#include <queue>
/**
 * coro_ret<int> coroutine_7in7out() 就是协程函数。
 * coro_ret<int> c_r 就是协程的返回值。
 * 在后续，都是通过c_r和协程进行交互。
 * coro_ret<int>::promise_type 就是承诺对象std::coroutine_handle<promise_type> 就是句柄。
 * @tparam T
 */
//!coro_ret 协程函数的返回值，内部定义promise_type，承诺对象
template<typename T>
struct coro_ret {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;
    //! 协程句柄
    handle_type coro_handle_;

    explicit coro_ret(handle_type h)
            : coro_handle_(h) {
    }

    coro_ret(const coro_ret &) = delete;

    coro_ret(coro_ret &&s) noexcept : coro_handle_(s.coro_handle_) {
        s.coro_handle_ = nullptr;
    }

    ~coro_ret() {
        //!自行销毁
        if (coro_handle_)
            coro_handle_.destroy();
    }

    coro_ret &operator=(const coro_ret &) = delete;

    coro_ret &operator=(coro_ret &&s) noexcept {
        coro_handle_ = s.coro_handle_;
        s.coro_handle_ = nullptr;
        return *this;
    }

    //!恢复协程，返回是否结束
    bool move_next() {
        coro_handle_.resume();
        return coro_handle_.done();
    }

    //!通过promise获取数据，返回值
    T get() {
        return coro_handle_.promise().return_data_;
    }

    //!promise_type就是承诺对象，承诺对象用于协程内外交流
    struct promise_type {
        promise_type() = default;

        ~promise_type() = default;

        //!生成协程返回值
        auto get_return_object() {
            return coro_ret<T>{handle_type::from_promise(*this)};
        }

        //! 注意这个函数,返回的就是awaiter
        //! 如果返回std::suspend_never{}，就不挂起，
        //! 返回std::suspend_always{} 挂起
        //! 当然你也可以返回其他awaiter
        auto initial_suspend() {
            //return std::suspend_never{};
            return std::suspend_always{};
        }

        //!co_return 后这个函数会被调用
        void return_value(T v) {
            return_data_ = v;
        }

        //!
        auto yield_value(T v) {
            std::cout << "yield_value invoked." << std::endl;
            return_data_ = v;
            return std::suspend_always{};
        }

        //! 在协程最后退出后调用的接口。
        //! 若 final_suspend 返回 std::suspend_always 则需要用户自行调用
        //! handle.destroy() 进行销毁，但注意final_suspend被调用时协程已经结束
        //! 返回std::suspend_always并不会挂起协程（实测 VSC++ 2022）
        auto final_suspend() noexcept {
            std::cout << "final_suspend invoked." << std::endl;
            return std::suspend_always{};
        }

        //
        void unhandled_exception() {
            std::exit(1);
        }

        //返回值
        T return_data_;
    };
};

//这就是一个协程函数
coro_ret<int> coroutine_7in7out() {
    //进入协程看initial_suspend，返回std::suspend_always{};会有一次挂起

    std::cout << "Coroutine co_await std::suspend_never" << std::endl;
    //co_await std::suspend_never{} 不会挂起
    co_await std::suspend_never{};
    std::cout << "Coroutine co_await std::suspend_always" << std::endl;
    co_await std::suspend_always{};

    std::cout << "Coroutine stage 1 ,co_yield" << std::endl;
    co_yield 101;
    std::cout << "Coroutine stage 2 ,co_yield" << std::endl;
    co_yield 202;
    std::cout << "Coroutine stage 3 ,co_yield" << std::endl;
    co_yield 303;
    std::cout << "Coroutine stage end, co_return" << std::endl;
    co_return 808;
}

DOCTEST_TEST_CASE("std_cororutine, std_coroutine_test") {

    bool done = false;
    std::cout << "Start coroutine_7in7out ()\n";
//调用协程,得到返回值c_r，后面使用这个返回值来管理协程。
    auto c_r = coroutine_7in7out();
//第一次停止因为initial_suspend 返回的是suspend_always
//此时没有进入Stage 1
    std::cout << "Coroutine " << (done ? "is done " : "isn't done ")
              << "ret =" << c_r.get() << std::endl;
    done = c_r.move_next();
//此时是，co_await std::suspend_always{}
    std::cout << "Coroutine " << (done ? "is done " : "isn't done ")
              << "ret =" << c_r.get() << std::endl;
    done = c_r.move_next();
//此时打印Stage 1
    std::cout << "Coroutine " << (done ? "is done " : "isn't done ")
              << "ret =" << c_r.get() << std::endl;
    done = c_r.move_next();
    std::cout << "Coroutine " << (done ? "is done " : "isn't done ")
              << "ret =" << c_r.get() << std::endl;
    done = c_r.move_next();
    std::cout << "Coroutine " << (done ? "is done " : "isn't done ")
              << "ret =" << c_r.get() << std::endl;
    done = c_r.move_next();
    std::cout << "Coroutine " << (done ? "is done " : "isn't done ")
              << "ret =" << c_r.get() << std::endl;
}


//  只支持移动对象，而不支持复制对象
struct Generator {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;
    std::coroutine_handle<promise_type> handle;

    explicit Generator(std::coroutine_handle<promise_type> handle) noexcept
            : handle(handle) {}

    Generator(Generator &&generator) noexcept
            : handle(std::exchange(generator.handle, {})) {}

    Generator(Generator &) = delete;

    Generator &operator=(Generator &) = delete;

    struct promise_type {

        // 开始执行时直接挂起等待外部调用 resume 获取下一个值
        std::suspend_always initial_suspend() { return {}; };

        // 总是挂起，让 Generator 来销毁
        std::suspend_always final_suspend() noexcept { return {}; }

        // 为了简单，我们认为序列生成器当中不会抛出异常，这里不做任何处理
        void unhandled_exception() {}

        // 构造协程的返回值类型
        Generator get_return_object() {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};;
        }

        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<Generator::promise_type> handle) const {
            handle.promise().value = value;
        }

        // 但通常情况下我们使用 co_await 更多的关注点在挂起自己，等待别人上，而使用 co_yield 则是挂起自己传值出去

        // await_transform 函数之后，co_await expr 就相当于 co_await promise.await_transform(expr)
        // 传值的同时要挂起，值存入 value 当中
        std::suspend_always await_transform(int value) {
            this->value = value;
            return {};
        }

        //co_yield expr 等价于 co_await promise.yield_value(expr)
        // 将 await_transform 替换为 yield_value
        std::suspend_always yield_value(int value) {
            this->value = value;
            is_ready = true;
            return {};
        }

        void await_resume() {}

        // 没有返回值
        void return_void() {}

        int value;
        bool is_ready = false;
    };

    bool has_next() const {
        // 协程已经执行完成
        if (handle.done()) {
            return false;
        }

        // 协程还没有执行完成，并且下一个值还没有准备好
        if (!handle.promise().is_ready) {
            handle.resume();
        }

        if (handle.done()) {
            // 恢复执行之后协程执行完，这时候必然没有通过 co_await 传出值来
            return false;
        } else {
            return true;
        }
    }

    int next() const {
        if (has_next()) {
            // 此时一定有值，is_ready 为 true
            // 消费当前的值，重置 is_ready 为 false
            handle.promise().is_ready = false;
            return handle.promise().value;
        }
        throw std::exception();
    }

    ~Generator() {
        // 销毁协程
        if (handle) handle.destroy();
    }
};

Generator sequence() {
    int i = 0;
    while (true) {
        co_await i++;
    }
}

Generator fibonacci() {
    co_yield 0; // fib(0)
    co_yield 1; // fib(1)

    int a = 0;
    int b = 1;
    while (true) {
        co_yield a + b; // fib(N), N > 1
        b = a + b;
        a = b - a;
    }
}

DOCTEST_TEST_CASE("std_cororutine, get_gener_test") {
    auto gen = fibonacci();
    for (int i = 0; i < 15; ++i) {
        if (gen.has_next()) {
            std::cout << gen.next() << std::endl;
        } else {
            break;
        }
    }
}


template<typename T>
struct Generator_t {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;
    std::coroutine_handle<promise_type> handle;

    explicit Generator_t(std::coroutine_handle<promise_type> handle) noexcept
            : handle(handle) {}

    Generator_t(Generator &&generator) noexcept
            : handle(std::exchange(generator.handle, {})) {}

    Generator_t(Generator &) = delete;

    Generator_t &operator=(Generator_t &) = delete;

    struct promise_type {

        // 开始执行时直接挂起等待外部调用 resume 获取下一个值
        std::suspend_always initial_suspend() { return {}; };

        // 总是挂起，让 Generator 来销毁
        std::suspend_always final_suspend() noexcept { return {}; }

        // 为了简单，我们认为序列生成器当中不会抛出异常，这里不做任何处理
        void unhandled_exception() {}

        // 构造协程的返回值类型
        Generator_t get_return_object() {
            return Generator_t{std::coroutine_handle<promise_type>::from_promise(*this)};;
        }

        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<Generator::promise_type> handle) const {
            handle.promise().value = value;
        }

        // 但通常情况下我们使用 co_await 更多的关注点在挂起自己，等待别人上，而使用 co_yield 则是挂起自己传值出去

        // await_transform 函数之后，co_await expr 就相当于 co_await promise.await_transform(expr)
        // 传值的同时要挂起，值存入 value 当中
        std::suspend_always await_transform(int value) {
            this->value = value;
            return {};
        }

        //co_yield expr 等价于 co_await promise.yield_value(expr)
        // 将 await_transform 替换为 yield_value
        std::suspend_always yield_value(int value) {
            this->value = value;
            is_ready = true;
            return {};
        }

        void await_resume() {}

        // 没有返回值
        void return_void() {}

        T value;
        bool is_ready = false;
    };

    bool has_next() const {
        // 协程已经执行完成
        if (handle.done()) {
            return false;
        }

        // 协程还没有执行完成，并且下一个值还没有准备好
        if (!handle.promise().is_ready) {
            handle.resume();
        }

        if (handle.done()) {
            // 恢复执行之后协程执行完，这时候必然没有通过 co_await 传出值来
            return false;
        } else {
            return true;
        }
    }

    int next() const {
        if (has_next()) {
            // 此时一定有值，is_ready 为 true
            // 消费当前的值，重置 is_ready 为 false
            handle.promise().is_ready = false;
            return handle.promise().value;
        }
        throw std::exception();
    }

    ~Generator_t() {
        // 销毁协程
        if (handle) handle.destroy();
    }

    Generator_t static from_array(T array[], int n) {
        for (int i = 0; i < n; ++i) {
            co_yield array[i];
        }
    }

    Generator_t static from_list(std::list<T> list) {
        for (auto t: list) {
            co_yield t;
        }
    }

    Generator_t static from(std::initializer_list<T> args) {
        for (auto t: args) {
            co_yield t;
        }
    }

    template<typename ...TArgs>
    Generator_t static from(TArgs ...args) {
        (co_yield args, ...);
    }

    template<typename R, typename F>
    R fold(R initial, F f) {
        R acc = initial;
        while (has_next()) {
            acc = f(acc, next());
        }
        return acc;
    }

    template<typename U>
    Generator_t<U> map(std::function<U(T)> f) {
        // 判断 this 当中是否有下一个元素
        while (has_next()) {
            // 使用 next 读取下一个元素
            // 通过 f 将其变换成 U 类型的值，再使用 co_yield 传出
            co_yield f(next());
        }
    }

    template<typename F>
    Generator_t<std::invoke_result_t<F, T>> map(F f) {
        while (has_next()) {
            co_yield f(next());
        }
    }

    template<typename F>
    // 返回值类型就是 F 的返回值类型
    std::invoke_result_t<F, T> flat_map(F f) {
        while (has_next()) {
            // 值映射成新的 Generator
            auto generator = f(next());
            // 将新的 Generator 展开
            while (generator.has_next()) {
                co_yield generator.next();
            }
        }
    }

    template<typename F>
    void for_each(F f) {
        while (has_next()) {
            f(next());
        }
    }

    // 截取前 n 个值的 take(n)
    T sum() {
        T sum = 0;
        while (has_next()) {
            sum += next();
        }
        return sum;
    }

    // 截取前 n 个值的 take(n)
    template<typename F>
    Generator_t filter(F f) {
        while (has_next()) {
            T value = next();
            if (f(value)) {
                co_yield value;
            }
        }
    }

    // 截取前 n 个值的 take(n)
    Generator_t take(int n) {
        int i = 0;
        while (i++ < n && has_next()) {
            co_yield next();
        }
    }

    template<typename F>
    Generator_t take_while(F f) {
        while (has_next()) {
            T value = next();
            if (f(value)) {
                co_yield value;
            } else {
                break;
            }
        }
    }
};

DOCTEST_TEST_CASE("std_cororutine, get_gener_algorithm") {

    Generator_t<int>::from(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
            .filter([](auto i) {
                std::cout << "filter: " << i << std::endl;
                return i % 2 == 0;
            })
            .map([](auto i) {
                std::cout << "map: " << i << std::endl;
                return i * 3;
            })
            .flat_map([](auto i) -> Generator_t<int> {
                std::cout << "flat_map: " << i << std::endl;
                for (int j = 0; j < i; ++j) {
                    co_yield j;
                }
            })
            .take(3)
            .for_each([](auto i) {
                std::cout << "for_each: " << i << std::endl;
            });
}

template<typename ResultType, typename Executor>
struct Task;

template<typename T>
struct Result {
    // 初始化为默认值
    explicit Result() = default;

    // 当 Task 正常返回时用结果初始化 Result
    explicit Result(T &&value) : _value(value) {}

    // 当 Task 抛异常时用异常初始化 Result
    explicit Result(std::exception_ptr &&exception_ptr) : _exception_ptr(exception_ptr) {}

    // 读取结果，有异常则抛出异常
    T get_or_throw() {
        if (_exception_ptr) {
            std::rethrow_exception(_exception_ptr);
        }
        return _value;
    }

private:
    T _value{};
    std::exception_ptr _exception_ptr;
};

#if 0
template<typename R>
struct TaskAwaiter {
    explicit TaskAwaiter(Task<R> &&task) noexcept
            : task(std::move(task)) {}

    TaskAwaiter(TaskAwaiter &&completion) noexcept
            : task(std::exchange(completion.task, {})) {}

    TaskAwaiter(TaskAwaiter &) = delete;

    TaskAwaiter &operator=(TaskAwaiter &) = delete;

    constexpr bool await_ready() const noexcept {
        return false;
    }

    void await_suspend(std::coroutine_handle<> handle) noexcept {
        // 当 task 执行完之后调用 resume
        task.finally([handle]() {
            handle.resume();
        });
    }

    // 协程恢复执行时，被等待的 Task 已经执行完，调用 get_result 来获取结果
    R await_resume() noexcept {
        return task.get_result();
    }

private:
    Task<R> task;

};


template<typename ResultType>
struct TaskPromise {
    // 协程立即执行
    std::suspend_never initial_suspend() { return {}; }

    // 执行结束后挂起，等待外部销毁。该逻辑与前面的 Generator 类似
    std::suspend_always final_suspend() noexcept { return {}; }

    // 构造协程的返回值对象 Task
    Task<ResultType> get_return_object() {
        return Task{std::coroutine_handle<TaskPromise>::from_promise(*this)};
    }

    void unhandled_exception() {
        std::lock_guard lock(completion_lock);
        result = Result<ResultType>(std::current_exception());
        completion.notify_all();
        // 调用回调
        notify_callbacks();
    }


    void return_value(ResultType value) {
        std::lock_guard lock(completion_lock);
        result = Result<ResultType>(std::move(value));
        completion.notify_all();
        // 调用回调
        notify_callbacks();
    }


    // 注意这里的模板参数
    template<typename _ResultType>
    TaskAwaiter<_ResultType> await_transform(Task<_ResultType> &&task) {
        return TaskAwaiter<_ResultType>(std::move(task));
    }


    ResultType get_result() {
        // 如果 result 没有值，说明协程还没有运行完，等待值被写入再返回
        std::unique_lock lock(completion_lock);
        if (!result.has_value()) {
            // 等待写入值之后调用 notify_all
            completion.wait(lock);
        }
        // 如果有值，则直接返回（或者抛出异常）
        return result->get_or_throw();
    }

    void on_completed(std::function<void(Result<ResultType>)> &&func) {
        std::unique_lock lock(completion_lock);
        // 加锁判断 result
        if (result.has_value()) {
            // result 已经有值
            auto value = result.value();
            // 解锁之后再调用 func
            lock.unlock();
            func(value);
        } else {
            // 否则添加回调函数，等待调用
            completion_callbacks.push_back(func);
        }
    }

private:
    // 使用 std::optional 可以区分协程是否执行完成
    std::optional<Result<ResultType>> result;

    std::mutex completion_lock;
    std::condition_variable completion;


    // 回调列表，我们允许对同一个 Task 添加多个回调
    std::list<std::function<void(Result<ResultType>)>> completion_callbacks;

    void notify_callbacks() {
        auto value = result.value();
        for (auto &callback: completion_callbacks) {
            callback(value);
        }
        // 调用完成，清空回调
        completion_callbacks.clear();
    }
};

template<typename ResultType>
struct Task {

    // 声明 promise_type 为 TaskPromise 类型
    using promise_type = TaskPromise<ResultType>;

    ResultType get_result() {
        return handle.promise().get_result();
    }

    Task &then(std::function<void(ResultType)> &&func) {
        handle.promise().on_completed([func](auto result) {
            try {
                func(result.get_or_throw());
            } catch (std::exception &e) {
                // 忽略异常
            }
        });
        return *this;
    }

    // 获取抛出的异常的回调 catching
    Task &catching(std::function<void(std::exception &)> &&func) {
        handle.promise().on_completed([func](auto result) {
            try {
                // 忽略返回值
                result.get_or_throw();
            } catch (std::exception &e) {
                func(e);
            }
        });
        return *this;
    }

    Task &finally(std::function<void()> &&func) {
        handle.promise().on_completed([func](auto result) { func(); });
        return *this;
    }

    explicit Task(std::coroutine_handle<promise_type> handle) noexcept: handle(handle) {}

    Task(Task &&task) noexcept: handle(std::exchange(task.handle, {})) {}

    Task(Task &) = delete;

    Task &operator=(Task &) = delete;

    ~Task() {
        if (handle) handle.destroy();
    }

private:
    std::coroutine_handle<promise_type> handle;
};



Task<int> simple_task2() {
    LOG_INFO_R << ("task 2 start ...");
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    LOG_INFO_R << ("task 2 returns after 1s.");
    co_return 2;
}

Task<int> simple_task3() {
    LOG_INFO_R << ("in task 3 start ...");
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(2s);
    LOG_INFO_R << ("task 3 returns after 2s.");
    co_return 3;
}

Task<int> simple_task() {
    LOG_INFO_R << ("task start ...");
    auto result2 = co_await simple_task2();
    LOG_INFO_R << "returns from task2: " << result2;
    auto result3 = co_await simple_task3();
    LOG_INFO_R << "returns from task3: " << result3;
    co_return 1 + result2 + result3;
}
#endif

class AbstractExecutor {
public:
    // func 继续执行
    virtual void execute(std::function<void()> &&func) = 0;
};

class NoopExecutor : public AbstractExecutor {
public:
    void execute(std::function<void()> &&func) override {
        func();
    }
};

class NewThreadExecutor : public AbstractExecutor {
public:
    void execute(std::function<void()> &&func) override {
        std::thread(func).detach();
    }
};

class AsyncExecutor : public AbstractExecutor {
public:
    void execute(std::function<void()> &&func) override {
        auto future = std::async(func);
    }
};

class LooperExecutor : public AbstractExecutor {
private:
    std::condition_variable queue_condition;
    std::mutex queue_lock;
    std::queue<std::function<void()>> executable_queue;

    // true 的时候是工作状态，如果要关闭事件循环，就置为 false
    std::atomic<bool> is_active;
    std::thread work_thread;

    // 处理事件循环
    void run_loop() {
        // 检查当前事件循环是否是工作状态，或者队列没有清空
        while (is_active.load(std::memory_order_relaxed) || !executable_queue.empty()) {
            std::unique_lock lock(queue_lock);
            if (executable_queue.empty()) {
                // 队列为空，需要等待新任务加入队列或者关闭事件循环的通知
                queue_condition.wait(lock);
                // 如果队列为空，那么说明收到的是关闭的通知
                if (executable_queue.empty()) {
                    // 现有逻辑下此处用 break 也可
                    // 使用 continue 可以再次检查状态和队列，方便将来扩展
                    continue;
                }
            }
            // 取出第一个任务，解锁再执行。
            // 解锁非常：func 是外部逻辑，不需要锁保护；func 当中可能请求锁，导致死锁
            auto func = executable_queue.front();
            executable_queue.pop();
            lock.unlock();

            func();
        }
    }

public:

    LooperExecutor() {
        is_active.store(true, std::memory_order_relaxed);
        work_thread = std::thread(&LooperExecutor::run_loop, this);
    }

    ~LooperExecutor() {
        shutdown(false);
        // 等待线程执行完，防止出现意外情况
        join();
    }

    void execute(std::function<void()> &&func) override {
        std::unique_lock lock(queue_lock);
        if (is_active.load(std::memory_order_relaxed)) {
            executable_queue.push(func);
            lock.unlock();
            // 通知队列，主要用于队列之前为空时调用 wait 等待的情况
            // 通知不需要加锁，否则锁会交给 wait 方导致当前线程阻塞
            queue_condition.notify_one();
        }
    }

    void shutdown(bool wait_for_complete = true) {
        // 修改后立即生效，在 run_loop 当中就能尽早（加锁前）就检测到 is_active 的变化
        is_active.store(false, std::memory_order_relaxed);
        if (!wait_for_complete) {
            std::unique_lock lock(queue_lock);
            // 清空任务队列
            decltype(executable_queue) empty_queue;
            std::swap(executable_queue, empty_queue);
            lock.unlock();
        }

        // 通知 wait 函数，避免 Looper 线程不退出
        // 不需要加锁，否则锁会交给 wait 方导致当前线程阻塞
        queue_condition.notify_all();
    }

    void join() {
        if (work_thread.joinable()) {
            work_thread.join();
        }
    }
};

// 协程共享一个 LooperExecutor 实例
class SharedLooperExecutor : public AbstractExecutor {
public:
    void execute(std::function<void()> &&func) override {
        static LooperExecutor sharedLooperExecutor;
        sharedLooperExecutor.execute(std::move(func));
    }
};

template<typename Result, typename Executor>
struct TaskAwaiter {
    // 构造 TaskAwaiter 的时候传入调度器的具体实现
    explicit TaskAwaiter(AbstractExecutor *executor, Task<Result, Executor> &&task) noexcept
            : task(std::move(task)),_executor(executor) {}

    TaskAwaiter(TaskAwaiter &&completion) noexcept
            : task(std::exchange(completion.task, {})) {}

    TaskAwaiter(TaskAwaiter &) = delete;

    TaskAwaiter &operator=(TaskAwaiter &) = delete;

    constexpr bool await_ready() const noexcept {
        return false;
    }

    void await_suspend(std::coroutine_handle<> handle) noexcept {
        task.finally([handle, this]() {
            // 将 resume 函数的调用交给调度器执行
            _executor->execute([handle]() {
                handle.resume();
            });
        });
    }

    // 协程恢复执行时，被等待的 Task 已经执行完，调用 get_result 来获取结果
    Result await_resume() noexcept {
        return task.get_result();
    }

private:
    Task<Result, Executor> task;
    AbstractExecutor *_executor;

};

struct DispatchAwaiter {

    explicit DispatchAwaiter(AbstractExecutor *executor) noexcept
            : _executor(executor) {}

    bool await_ready() const { return false; }

    void await_suspend(std::coroutine_handle<> handle) const {
        // 调度到协程对应的调度器上
        _executor->execute([handle]() {
            handle.resume();
        });
    }

    void await_resume() {}

private:
    AbstractExecutor *_executor;
};

class DelayedExecutable {
public:
    DelayedExecutable(std::function<void()> &&func, long long delay) : func(std::move(func)) {
        using namespace std;
        using namespace std::chrono;
        auto now = system_clock::now();
        // 当前的时间戳，单位毫秒
        auto current = duration_cast<milliseconds>(now.time_since_epoch()).count();

        // 计算出任务的计划执行时间
        scheduled_time = current + delay;
    }

    // 调用时，返回从当前时间还需要多少毫秒到任务执行时间
    long long delay() const {
        using namespace std;
        using namespace std::chrono;

        auto now = system_clock::now();
        auto current = duration_cast<milliseconds>(now.time_since_epoch()).count();
        return scheduled_time - current;
    }

    long long get_scheduled_time() const {
        return scheduled_time;
    }

    void operator()() {
        func();
    }

private:
    long long scheduled_time;
    std::function<void()> func;
};

class DelayedExecutableCompare {
public:
    bool operator()(DelayedExecutable &left, DelayedExecutable &right) {
        return left.get_scheduled_time() > right.get_scheduled_time();
    }
};

class Scheduler {
private:
    std::condition_variable queue_condition;
    std::mutex queue_lock;
    // 注意这里改用优先级队列
    std::priority_queue<DelayedExecutable, std::vector<DelayedExecutable>, DelayedExecutableCompare> executable_queue;

    // true 的时候是工作状态，如果要关闭事件循环，就置为 false
    std::atomic<bool> is_active;
    std::thread work_thread;

    // 处理事件循环
    void run_loop() {
        while (is_active.load(std::memory_order_relaxed) || !executable_queue.empty()) {
            std::unique_lock lock(queue_lock);
            if (executable_queue.empty()) {
                queue_condition.wait(lock);
                if (executable_queue.empty()) {
                    continue;
                }
            }

            // 从这里开始于 LooperExecutor 不同，这里需要判断优先级队头的任务，也就是最先要执行的任务是否需要立即执行
            auto executable = executable_queue.top();
            long long delay = executable.delay();
            if (delay > 0) {
                // 队头的任务还没到执行时间，等待 delay 毫秒
                auto status = queue_condition.wait_for(lock, std::chrono::milliseconds(delay));
                // 如果等待期间没有延时比 delay 更小的任务加入，这里就会返回 timeout
                if (status != std::cv_status::timeout) {
                    // 不是 timeout，需要重新计算队头的延时
                    continue;
                }
            }
            executable_queue.pop();
            lock.unlock();
            executable();
        }
    }

public:

    void execute(std::function<void()> &&func, long long delay) {
        delay = delay < 0 ? 0 : delay;
        std::unique_lock lock(queue_lock);
        if (is_active.load(std::memory_order_relaxed)) {
            // 只有队列为空或者比当前队头任务的延时更小时，需要调用 notify_one
            // 其他情况只需要按顺序依次执行即可
            bool need_notify = executable_queue.empty() || executable_queue.top().delay() > delay;
            executable_queue.push(DelayedExecutable(std::move(func), delay));
            lock.unlock();
            if (need_notify) {
                queue_condition.notify_one();
            }
        }
    }


    Scheduler() {
        is_active.store(true, std::memory_order_relaxed);
        work_thread = std::thread(&Scheduler::run_loop, this);
    }

    ~Scheduler() {
        shutdown(false);
        // 等待线程执行完，防止出现意外情况
        join();
    }


    void shutdown(bool wait_for_complete = true) {
        // 修改后立即生效，在 run_loop 当中就能尽早（加锁前）就检测到 is_active 的变化
        is_active.store(false, std::memory_order_relaxed);
        if (!wait_for_complete) {
            std::unique_lock lock(queue_lock);
            // 清空任务队列
            decltype(executable_queue) empty_queue;
            std::swap(executable_queue, empty_queue);
            lock.unlock();
        }

        // 通知 wait 函数，避免 Looper 线程不退出
        // 不需要加锁，否则锁会交给 wait 方导致当前线程阻塞
        queue_condition.notify_all();
    }

    void join() {
        if (work_thread.joinable()) {
            work_thread.join();
        }
    }
};

struct SleepAwaiter {

    explicit SleepAwaiter(AbstractExecutor *executor, long long duration) noexcept
            : _executor(executor), _duration(duration) {}

    bool await_ready() const { return false; }

    void await_suspend(std::coroutine_handle<> handle) const {
        // 自定义的延时执行工具类，全局只需要一个实例
        static Scheduler scheduler;

        scheduler.execute([this, handle]() {
            // _duration 毫秒之后执行下面的代码
            _executor->execute([handle]() {
                handle.resume();
            });
        }, _duration);
    }

    void await_resume() {}

private:
    AbstractExecutor *_executor;
    long long _duration;
};

template<typename ValueType>
struct WriterAwaiter;

template<typename ValueType>
struct ReaderAwaiter;

template<typename ValueType>
struct Channel {

    auto write(ValueType value) {
        check_closed();
        return WriterAwaiter<ValueType>(this, value);
    }

    auto operator<<(ValueType value) {
        return write(value);
    }

    auto read() {
        check_closed();
        return ReaderAwaiter<ValueType>(this);
    }

    auto operator>>(ValueType &value_ref) {
        auto awaiter = read();
        // 保存待赋值的变量的地址，方便后续写入
        awaiter.p_value = &value_ref;
        return awaiter;
    }

    void try_push_writer(WriterAwaiter<ValueType> *writer_awaiter) {
        std::unique_lock lock(channel_lock);
        check_closed();
        // 检查有没有挂起的读取者，对应情况 1
        if (!reader_list.empty()) {
            auto reader = reader_list.front();
            reader_list.pop_front();
            lock.unlock();

            reader->resume(writer_awaiter->_value);
            writer_awaiter->resume();
            return;
        }

        // buffer 未满，对应情况 2
        if (static_cast<int>(buffer.size()) < buffer_capacity) {
            buffer.push(writer_awaiter->_value);
            lock.unlock();
            writer_awaiter->resume();
            return;
        }

        // buffer 已满，对应情况 3
        writer_list.push_back(writer_awaiter);
    }

    void try_push_reader(ReaderAwaiter<ValueType> *reader_awaiter) {
        std::unique_lock lock(channel_lock);
        check_closed();

        // buffer 非空，对应情况 1
        if (!buffer.empty()) {
            auto value = buffer.front();
            buffer.pop();

            if (!writer_list.empty()) {
                // 有挂起的写入者要及时将其写入 buffer 并恢复执行
                auto writer = writer_list.front();
                writer_list.pop_front();
                buffer.push(writer->_value);
                lock.unlock();

                writer->resume();
            } else {
                lock.unlock();
            }

            reader_awaiter->resume(value);
            return;
        }

        // 有写入者挂起，对应情况 2
        if (!writer_list.empty()) {
            auto writer = writer_list.front();
            writer_list.pop_front();
            lock.unlock();

            reader_awaiter->resume(writer->_value);
            writer->resume();
            return;
        }

        // buffer 为空，对应情况 3
        reader_list.push_back(reader_awaiter);
    }

    struct ChannelClosedException : std::exception {
        const char *what() const noexcept override {
            return "Channel is closed.";
        }
    };

    void check_closed() {
        // 如果已经关闭，则抛出异常
        if (!_is_active.load(std::memory_order_relaxed)) {
            throw ChannelClosedException();
        }
    }


    explicit Channel(int capacity = 0) : buffer_capacity(capacity) {
        _is_active.store(true, std::memory_order_relaxed);
    }

    // true 表示 Channel 尚未关闭
    bool is_active() {
        return _is_active.load(std::memory_order_relaxed);
    }

    // 关闭 Channel
    void close() {
        bool expect = true;
        // 判断如果已经关闭，则不再重复操作
        // 比较 _is_active 为 true 时才会完成设置操作，并且返回 true
        if (_is_active.compare_exchange_strong(expect, false, std::memory_order_relaxed)) {
            // 清理资源
            clean_up();
        }
    }

    // 不希望 Channel 被移动或者复制
    Channel(Channel &&channel) = delete;

    Channel(Channel &) = delete;

    Channel &operator=(Channel &) = delete;

    // 销毁时关闭
    ~Channel() {
        close();
    }

    void remove_reader(ReaderAwaiter<ValueType> *reader_awaiter) {
        // 并发环境，修改 reader_list 的操作都需要加锁
        std::lock_guard lock(channel_lock);
        reader_list.remove(reader_awaiter);
    }

private:
    // buffer 的容量
    int buffer_capacity;
    std::queue<ValueType> buffer;
    // buffer 已满时，新来的写入者需要挂起保存在这里等待恢复
    std::list<WriterAwaiter<ValueType> *> writer_list;
    // buffer 为空时，新来的读取者需要挂起保存在这里等待恢复
    std::list<ReaderAwaiter<ValueType> *> reader_list;
    // Channel 的状态标识
    std::atomic<bool> _is_active;

    std::mutex channel_lock;
    std::condition_variable channel_condition;

    void clean_up() {
        std::lock_guard lock(channel_lock);

        // 需要对已经挂起等待的协程予以恢复执行
        for (auto writer: writer_list) {
            writer->resume();
        }
        writer_list.clear();

        for (auto reader: reader_list) {
            reader->resume();
        }
        reader_list.clear();

        // 清空 buffer
        decltype(buffer) empty_buffer;
        std::swap(buffer, empty_buffer);
    }
};

template<typename ValueType>
struct WriterAwaiter {
    Channel<ValueType> *channel;
    // 调度器不是必须的，如果没有，则直接在当前线程执行（等价于 NoopExecutor）
    AbstractExecutor *executor = nullptr;
    // 写入 Channel 的值
    ValueType _value;
    std::coroutine_handle<> handle;

    WriterAwaiter(Channel<ValueType> *channel, ValueType value)
            : channel(channel), _value(value) {}

    bool await_ready() {
        return false;
    }

    auto await_suspend(std::coroutine_handle<> coroutine_handle) {
        // 记录协程 handle，恢复时用
        this->handle = coroutine_handle;
        // 将自身传给 Channel，Channel 内部会根据自身状态处理是否立即恢复或者挂起
        channel->try_push_writer(this);
    }

    void await_resume() {
        // Channel 关闭时也会将挂起的读写协程恢复
        // 要检查是否是关闭引起的恢复，如果是，check_closed 会抛出 Channel 关闭异常
        channel->check_closed();
    }

    // Channel 当中恢复该协程时调用 resume 函数
    void resume() {
        // 我们将调度器调度的逻辑封装在这里
        if (executor) {
            executor->execute([this]() { handle.resume(); });
        } else {
            handle.resume();
        }
    }
};

template<typename ValueType>
struct ReaderAwaiter {
    Channel<ValueType> *channel;
    AbstractExecutor *executor = nullptr;
    ValueType _value;
    // 用于 channel >> received; 这种情况
    // 需要将变量的地址传入，协程恢复时写入变量内存
    ValueType *p_value = nullptr;
    std::coroutine_handle<> handle;

    // 实现移动构造函数，主要目的是将原对象的 channel 置为空
    ReaderAwaiter(ReaderAwaiter &&other) noexcept
            : channel(std::exchange(other.channel, nullptr)),
              executor(std::exchange(other.executor, nullptr)),
              _value(other._value),
              p_value(std::exchange(other.p_value, nullptr)),
              handle(other.handle) {}

    explicit ReaderAwaiter(Channel<ValueType> *channel) : channel(channel) {}

    ~ReaderAwaiter() {
        // channel 不为空，说明协程提前被销毁了
        // 调用 channel 的 remove_reader 将自己直接移除
        if (channel) channel->remove_reader(this);
    }

    bool await_ready() { return false; }

    auto await_suspend(std::coroutine_handle<> coroutine_handle) {
        this->handle = coroutine_handle;
        // 将自身传给 Channel，Channel 内部会根据自身状态处理是否立即恢复或者挂起
        channel->try_push_reader(this);
    }

    int await_resume() {
        // Channel 关闭时也会将挂起的读写协程恢复
        // 要检查是否是关闭引起的恢复，如果是，check_closed 会抛出 Channel 关闭异常
        channel->check_closed();
        channel = nullptr;
        return _value;
    }

    // Channel 当中正常恢复读协程时调用 resume 函数
    void resume(ValueType value) {
        this->_value = value;
        if (p_value) {
            *p_value = value;
        }
        resume();
    }

    // Channel 关闭时调用 resume() 函数来恢复该协程
    // 在 await_resume 当中，如果 Channel 关闭，会抛出 Channel 关闭异常
    void resume() {
        if (executor) {
            executor->execute([this]() { handle.resume(); });
        } else {
            handle.resume();
        }
    }
};

template<typename ResultType, typename Executor>
struct TaskPromise {

    // 协程启动时也需要在恢复时实现调度
    DispatchAwaiter initial_suspend() { return DispatchAwaiter{&executor}; }

    // 执行结束后挂起，等待外部销毁。该逻辑与前面的 Generator 类似
    std::suspend_always final_suspend() noexcept { return {}; }

    // 构造协程的返回值对象 Task
    // Task 类型增加模板参数 Executor 可以方便创建协程时执行调度器的类型
    Task<ResultType, Executor> get_return_object() {
        return Task{std::coroutine_handle<TaskPromise>::from_promise(*this)};
    }

    void unhandled_exception() {
        std::lock_guard lock(completion_lock);
        result = Result<ResultType>(std::current_exception());
        completion.notify_all();
        // 调用回调
        notify_callbacks();
    }

    void return_value(ResultType value) {
        std::lock_guard lock(completion_lock);
        result = Result<ResultType>(std::move(value));
        completion.notify_all();
        notify_callbacks();
    }


    // 注意模板参数
    template<typename _ResultType, typename _Executor>
    TaskAwaiter<_ResultType, _Executor> await_transform(Task<_ResultType, _Executor> &&task) {
        return TaskAwaiter<_ResultType, _Executor>(&executor, std::move(task));
    }

    template<typename _Rep, typename _Period>
    SleepAwaiter await_transform(std::chrono::duration<_Rep, _Period> &&duration) {
        return SleepAwaiter(&executor, std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
    }

    template<typename _ValueType>
    auto await_transform(ReaderAwaiter<_ValueType> reader_awaiter) {
        reader_awaiter.executor = &executor;
        return reader_awaiter;
    }

    template<typename _ValueType>
    auto await_transform(WriterAwaiter<_ValueType> writer_awaiter) {
        writer_awaiter.executor = &executor;
        return writer_awaiter;
    }

    ResultType get_result() {
        // 如果 result 没有值，说明协程还没有运行完，等待值被写入再返回
        std::unique_lock lock(completion_lock);
        if (!result.has_value()) {
            // 等待写入值之后调用 notify_all
            completion.wait(lock);
        }
        // 如果有值，则直接返回（或者抛出异常）
        return result->get_or_throw();
    }

    void on_completed(std::function<void(Result<ResultType>)> &&func) {
        std::unique_lock lock(completion_lock);
        // 加锁判断 result
        if (result.has_value()) {
            // result 已经有值
            auto value = result.value();
            // 解锁之后再调用 func
            lock.unlock();
            func(value);
        } else {
            // 否则添加回调函数，等待调用
            completion_callbacks.push_back(func);
        }
    }

private:
    Executor executor;

    // 使用 std::optional 可以区分协程是否执行完成
    std::optional<Result<ResultType>> result;

    std::mutex completion_lock;
    std::condition_variable completion;

    // 回调列表，我们允许对同一个 Task 添加多个回调
    std::list<std::function<void(Result<ResultType>)>> completion_callbacks;

    void notify_callbacks() {
        auto value = result.value();
        for (auto &callback: completion_callbacks) {
            callback(value);
        }
        // 调用完成，清空回调
        completion_callbacks.clear();
    }
};


// NewThreadExecutor 是 AbstractExecutor 的子类，作为模板参数 Executor 的默认值
template<typename ResultType, typename Executor>
struct Task {

    // 声明 promise_type 为 TaskPromise 类型
    using promise_type = TaskPromise<ResultType, Executor>;;

    ResultType get_result() {
        return handle.promise().get_result();
    }

    Task &then(std::function<void(ResultType)> &&func) {
        handle.promise().on_completed([func](auto result) {
            try {
                func(result.get_or_throw());
            } catch (std::exception &e) {
                // 忽略异常
            }
        });
        return *this;
    }

    // 获取抛出的异常的回调 catching
    Task &catching(std::function<void(std::exception &)> &&func) {
        handle.promise().on_completed([func](auto result) {
            try {
                // 忽略返回值
                result.get_or_throw();
            } catch (std::exception &e) {
                func(e);
            }
        });
        return *this;
    }

    Task &finally(std::function<void()> &&func) {
        handle.promise().on_completed([func](auto result) { func(); });
        return *this;
    }

    explicit Task(std::coroutine_handle<promise_type> handle) noexcept: handle(handle) {}

    Task(Task &&task) noexcept: handle(std::exchange(task.handle, {})) {}

    Task(Task &) = delete;

    Task &operator=(Task &) = delete;

    ~Task() {
        if (handle) handle.destroy();
    }

private:
    std::coroutine_handle<promise_type> handle;
};

/*

// 使用了 Async 调度器
// 这意味着每个恢复的位置都会通过 std::async 上执行
Task<int, AsyncExecutor> simple_task2() {
    LOG_INFO_R <<("task 2 start ...");
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    LOG_INFO_R <<("task 2 returns after 1s.");
    co_return 2;
}

// 使用了 NewThread 调度器
// 这意味着每个恢复的位置都会新建一个线程来执行
Task<int, NewThreadExecutor> simple_task3() {
    LOG_INFO_R <<("in task 3 start ...");
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(2s);
    LOG_INFO_R <<("task 3 returns after 2s.");
    co_return 3;
}

// 使用了 Looper 调度器
// 这意味着每个恢复的位置都会在同一个线程上执行
Task<int, LooperExecutor> simple_task() {
    LOG_INFO_R <<"task start ...";
    auto result2 = co_await simple_task2();
    LOG_INFO_R <<"returns from task2: " << result2;
    auto result3 = co_await simple_task3();
    LOG_INFO_R <<"returns from task3: "<< result3;
    co_return 1 + result2 + result3;
}
*/

Task<int, AsyncExecutor> simple_task2() {
    LOG_INFO_R << "task 2 start ...";
    using namespace std::chrono_literals;
    co_await 1s;
    LOG_INFO_R << "task 2 returns after 1s.";
    co_return 2;
}

Task<int, NewThreadExecutor> simple_task3() {
    LOG_INFO_R << "in task 3 start ...";
    using namespace std::chrono_literals;
    co_await 2s;
    LOG_INFO_R << "task 3 returns after 2s.";
    co_return 3;
}

Task<int, LooperExecutor> simple_task() {
    LOG_INFO_R << "task start ...";
    using namespace std::chrono_literals;
    co_await 100ms;
    LOG_INFO_R << "after 100ms ...";
    auto result2 = co_await simple_task2();
    LOG_INFO_R << "returns from task2: " << result2;

    co_await 500ms;
    LOG_INFO_R << "after 500ms ...";
    auto result3 = co_await simple_task3();
    LOG_INFO_R << "returns from task3: " << result3;
    co_return 1 + result2 + result3;
}

using namespace std::chrono_literals;

Task<int, LooperExecutor> Producer(Channel<int> &channel) {
    int i = 0;
    while (i < 10) {
        LOG_INFO_R << "send: " << i;
        // 或者使用 write 函数：co_await channel.write(i++);
        co_await (channel << i++);
        co_await 300ms;
    }

    channel.close();
    LOG_INFO_R << ("close channel, exit.");
    co_return 1;
}

Task<int, LooperExecutor> Consumer(Channel<int> &channel) {
    while (channel.is_active()) {
        try {
            // 或者使用 read 函数：auto received = co_await channel.read();
            int received;
            co_await (channel >> received);
            LOG_INFO_R << "receive: " << received;
            co_await 2s;
        } catch (std::exception &e) {
            LOG_INFO_R << "exception: " << e.what();
        }
    }

    LOG_INFO_R << ("exit.");
    co_return 1;
}

Task<int, LooperExecutor> Consumer2(Channel<int> &channel) {
    while (channel.is_active()) {
        try {
            auto received = co_await channel.read();
            LOG_INFO_R << "receive2: " << received;
            co_await 3s;
        } catch (std::exception &e) {
            LOG_INFO_R << "exception2: " << e.what();
        }
    }

    LOG_INFO_R << ("exit.");
    co_return 1;
}

DOCTEST_TEST_CASE("std_cororutine, get_gener_promise") {
    auto simpleTask = simple_task();
    simpleTask.then([](int i) {
        LOG_INFO_R << "simple task end: " << i;
    }).catching([](std::exception &e) {
        LOG_INFO_R << "error occurred" << e.what();
    });
    try {
        auto i = simpleTask.get_result();
        LOG_INFO_R << "simple task end from get: " << i;
    } catch (std::exception &e) {
        LOG_INFO_R << "error: " << e.what();
    }
}
/*

Task<std::string, AsyncExecutor> http_get(std::string host, std::string path) {
    httplib::Client cli(host);

    auto res = cli.Get(path.c_str());

    if (res) {
        co_return res->body;
    } else {
        co_return httplib::to_string(res.error());
    }
}

Task<void, LooperExecutor> test_http() {
    try {
        debug("send request...");
        auto result = co_await http_get("https://api.github.com", "/users/bennyhuo");
        debug("done.");
        auto json = nlohmann::json::parse(result);
        debug(json.dump(2));
        debug(json["login"]);
        debug(json["url"]);
        debug(json["bio"]);
    } catch (std::exception &e) {
        debug(e.what());
    }
}

int main() {
    test_http().get_result();
    return 0;
}
*/


DOCTEST_TEST_CASE("std_cororutine, get_channel") {
    auto channel = Channel<int>(2);
    auto producer = Producer(channel);
    auto consumer = Consumer(channel);
    auto consumer2 = Consumer2(channel);

    // 等待协程执行完成再退出
    producer.get_result();
    consumer.get_result();
    consumer2.get_result();
}

