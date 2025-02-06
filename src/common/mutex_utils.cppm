module;

#include <thread>

#ifdef NO_USE_MODULE

#ifdef WIN32

#include <Windows.h>


typedef CRITICAL_SECTION pthread_mutex_t;

#else

#include <pthread.h>

#endif
#endif

// uint32_t
// 自旋锁
#include <atomic>
// 信号量
#include <mutex>
#include <condition_variable>
#include <shared_mutex>
#include <cstddef>

export module mutex_utils;


export namespace fast::util {


#ifdef NO_USE_MODULE

    /**
     * @brief windows下为临界区:	通过对多线程的串行化来访问公共资源或一段代码，速度快，适合控制数据访问
     * 		  linux下为互斥锁:	为协调共同对一个共享资源的单独访问而设计的
     */
    class Mutex /*: public NonCopyable*/ {
    public:
        Mutex()
                : _threadId(0) {
#ifdef WIN32
            InitializeCriticalSection(&_mutex);
#else
            pthread_mutex_init(&_mutex, NULL);
#endif
        }

        ~Mutex() {
#ifdef WIN32
            DeleteCriticalSection(&_mutex);
#else
            pthread_mutex_destroy(&_mutex);
#endif
        }

        void lock() {
#ifdef WIN32
            EnterCriticalSection(&_mutex);
        _threadId = static_cast<uint32_t>(GetCurrentThreadId());
#else
            pthread_mutex_lock(&_mutex);
            _threadId = static_cast<uint32_t>(pthread_self());
#endif
        }

        void unlock() {
            _threadId = 0;
#ifdef WIN32
            LeaveCriticalSection(&_mutex);
#else
            pthread_mutex_unlock(&_mutex);
#endif
        }

        pthread_mutex_t *getMutex() {
            return &_mutex;
        }

    private:
        uint32_t _threadId;             // 获取当前线程id
        pthread_mutex_t _mutex;         // 锁
    };

#endif

/**
 * @brief 自旋锁
 * atomic_flag是原子操作的，
 * 获得锁后会把此变量置为true，
 * 另外一个线程就会因为flag=true而一直在while处执行循环，
 * 直到获得锁的线程将锁释放(将flag置为false)
 */
    class SpinlockMutex/* : public NonCopyable */{
    public:
        SpinlockMutex() = default;

        void lock() {
            while (flag.test_and_set(std::memory_order_acquire));
        }

        void unlock() {
            flag.clear(std::memory_order_release);
        }

    private:
        std::atomic_flag flag;
    };

// 使用C++11的原子操作实现自旋锁（默认内存序，memory_order_seq_cst）
    class spin_mutex {
        // flag对象所封装的bool值为false时，说明自旋锁未被线程占有。
        std::atomic<bool> flag = ATOMIC_VAR_INIT(false);
    public:
        spin_mutex() = default;

        spin_mutex(const spin_mutex &) = delete;

        spin_mutex &operator=(const spin_mutex &) = delete;

        void lock() {
            bool expected = false;
            // CAS原子操作。判断flag对象封装的bool值是否为期望值(false)，若为bool值为false，与期望值相等，说明自旋锁空闲。
            // 此时，flag对象封装的bool值写入true，CAS操作成功，结束循环，即上锁成功。
            // 若bool值为为true，与期望值不相等，说明自旋锁被其它线程占据，即CAS操作不成功。然后，由于while循环一直重试，直到CAS操作成功为止。
            while (!flag.compare_exchange_strong(expected, true)) {
                expected = false;
            }
        }

        void unlock() {
            flag.store(false);
        }
    };

    class spin_lock {
    public:
        spin_lock() noexcept {
            v_.clear();
        }

        bool try_lock() noexcept {
            return !v_.test_and_set(std::memory_order_acquire);
        }

        void lock() noexcept {
            for (unsigned k = 0; !try_lock(); ++k) {
                if (k < 16) {
                    std::this_thread::yield();
                } else if (k < 32) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(0));
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
        }

        void unlock() noexcept {
            v_.clear(std::memory_order_release);
        }

    public:
        std::atomic_flag v_;
    };


    // 读优先
    class readWriteLock {
    private:
        std::shared_mutex readMtx;
        std::mutex writeMtx;
        int readCnt; // 已加读锁个数
    public:
        readWriteLock() : readCnt(0) {}

        void readLock() {
            readMtx.lock();
            if (++readCnt == 1) {
                writeMtx.lock();  // 存在线程读操作时，写加锁（只加一次）
            }
            readMtx.unlock();
        }

        void readUnlock() {
            readMtx.lock();
            if (--readCnt == 0) { // 没有线程读操作时，释放写锁
                writeMtx.unlock();
            }
            readMtx.unlock();
        }

        void writeLock() {
            writeMtx.lock();
        }

        void writeUnlock() {
            writeMtx.unlock();
        }
    };

    /**
     * @brief 自动析构锁
     */
    template<typename Mutex>
    class MutexGuard /*: public NonCopyable*/ {
    public:
        explicit MutexGuard(Mutex &mux)
                : _mutex(mux) {
            _mutex.lock();
        }

        ~MutexGuard() {
            _mutex.unlock();
        }

    private:
        Mutex &_mutex;                // 锁封装
    };

    // 信号量
    class semaphore {
    public:
        explicit semaphore(std::size_t initial = 0) {
#if defined(HAVE_SEM)
            sem_init(&_sem, 0, initial);
#else
            _count = 0;
#endif
        }

        ~semaphore() {
#if defined(HAVE_SEM)
            sem_destroy(&_sem);
#endif
        }

        // 取消阻止
        void post(std::size_t n = 1) {
#if defined(HAVE_SEM)
            while (n--) {
                sem_post(&_sem);
            }
#else
            std::unique_lock<std::recursive_mutex> lock(_mutex);
            _count += n;
            if (n == 1) {
                _condition.notify_one();
            } else {
                _condition.notify_all();
            }
#endif
        }

        // 阻止线程
        void wait() {
#if defined(HAVE_SEM)
            sem_wait(&_sem);
#else
            std::unique_lock<std::recursive_mutex> lock(_mutex);
            while (_count == 0) {
                _condition.wait(lock);
            }
            --_count;
#endif
        }

    private:
#if defined(HAVE_SEM)
        sem_t _sem;
#else
        std::size_t _count;
        std::recursive_mutex _mutex;
        std::condition_variable_any _condition;
#endif
    };

    template<typename T>
    class scoped_ptr {
    public:
        // Constructor.
        explicit scoped_ptr(T *p = 0)
                : p_(p) {
        }

        // Destructor.
        ~scoped_ptr() {
            delete p_;
        }

        // Access.
        T *get() {
            return p_;
        }

        // Access.
        T *operator->() {
            return p_;
        }

        // Dereference.
        T &operator*() {
            return *p_;
        }

        // Reset pointer.
        void reset(T *p = 0) {
            delete p_;
            p_ = p;
        }

        // Release ownership of the pointer.
        T *release() {
            T *tmp = p_;
            p_ = 0;
            return tmp;
        }

    private:
        // Disallow copying and assignment.
        scoped_ptr(const scoped_ptr &);

        scoped_ptr &operator=(const scoped_ptr &);

        T *p_;
    };

}