module;


#include <functional>
#include <memory>

#include <boost/type_index.hpp>

export module small_utils;

export  namespace fast::util{
    //禁止拷贝基类
    class NonCopyable {
    public:
        NonCopyable() = default;

        NonCopyable(const NonCopyable &) = delete;

        NonCopyable &operator=(const NonCopyable &) = delete;

        NonCopyable &operator=(NonCopyable &&that) = delete;
    };

    /**
     *
     *     std::shared_ptr<void> auto_free(nullptr, [&i](void *)
     *                                   {
     *                                       LOG_INFO << i;
     *                                   }
     *   );
     *    可用共享指针替代
      */

    class OnceToken : public NonCopyable {
    public:
        using task = std::function<void(void)>;

        template<typename FUNC>
        explicit OnceToken(const FUNC &onConstructed, task onDestructed = nullptr) {
            onConstructed();
            _onDestructed = std::move(onDestructed);
        }

        explicit OnceToken(std::nullptr_t, task onDestructed = nullptr) {
            _onDestructed = std::move(onDestructed);
        }

        ~OnceToken() {
            if (_onDestructed) {
                _onDestructed();
            }
        }

    public:
        OnceToken() = delete;

        OnceToken(const OnceToken &) = delete;

        OnceToken(OnceToken &&) = delete;

        OnceToken &operator=(const OnceToken &) = delete;

        OnceToken &operator=(OnceToken &&) = delete;

    private:
        task _onDestructed;
    };


    //对象安全的构建和析构
    //构建后执行onCreate函数
    //析构前执行onDestory函数
    //在函数onCreate和onDestory中可以执行构造或析构中不能调用的方法，比如说shared_from_this或者虚函数
    class Creator {
    public:
        template<typename C, typename ...ArgsType>
        static std::shared_ptr<C> create(ArgsType &&...args) {
            std::shared_ptr<C> ret(new C(std::forward<ArgsType>(args)...), [](C *ptr) {
                ptr->onDestory();
                delete ptr;
            });
            ret->onCreate();
            return ret;
        }

    private:
        Creator() = default;

        ~Creator() = default;
    };

    /**
     * @brief 单例模式封装类
     * @details T 类型
     */
    template<class T>
    class Singleton {
    public:
        /**
         * @brief 返回单例裸指针
         */
        static T *GetInstance() {
            static T v;
            return &v;
        }
    };

    /**
     * @brief 单例模式智能指针封装类
     * @details T 类型
     */
    template<class T>
    class SingletonPtr {
    public:
        /**
         * @brief 返回单例智能指针
         */
        static std::shared_ptr<T> GetInstance() {
            static std::shared_ptr<T> v(new T);
            return v;
        }
    };
/*
    *//**
    * @brief a factory template class
    * @tparam unique
    * @tparam product_base
    * @tparam products
    * @see https://hedzr.com/c++/algorithm/cxx17-factory-pattern/
    *//*
    template<typename product_base, typename... products>
    class factory final {
    public:
#if defined(_MSC_VER)
        using string = std::string; // or std::string_view
#else
        using string = std::string_view;
#endif
        template<typename T>
        struct clz_name_t {
            string id = debug::type_name<T>();
            T data;
        };
        using named_products = std::tuple<clz_name_t<products>...>;
        // using _T = typename std::conditional<unique, std::unique_ptr<product_base>, std::shared_ptr<product_base>>::type;

        template<typename... Args>
        static auto create(const string &id, Args &&...args) {
            std::unique_ptr<product_base> result{};

            std::apply([](auto &&...it) {
                           ((static_check<decltype(it.data)>()), ...);
                       },
                       named_products{});

            std::apply([&](auto &&...it) {
                           ((it.id == id ? result = std::make_unique<decltype(it.data)>(args...) : result), ...);
                       },
                       named_products{});
            return result;
        }
        template<typename... Args>
        static std::shared_ptr<product_base> make_shared(const string &id, Args &&...args) {
            std::shared_ptr<product_base> ptr = create(id, args...);
            return ptr;
        }
        template<typename... Args>
        static std::unique_ptr<product_base> make_unique(const string &id, Args &&...args) {
            return create(id, args...);
        }
        template<typename... Args>
        static product_base *create_nacked_ptr(const string &id, Args &&...args) {
            return create(id, args...).release();
        }

    private:
        template<typename product>
        static void static_check() {
            static_assert(std::is_base_of<product_base, product>::value, "all products must inherit from product_base");
        }
    }; // class factory*/

    /**
    * @brief a factory template class
    * @tparam unique
    * @tparam product_base
    * @tparam products
    * @see https://hedzr.com/c++/algorithm/cxx17-factory-pattern/
    */
    template<typename product_base, typename... products>
    class factory final {
    public:
#if defined(_MSC_VER)
        using string = std::string; // or std::string_view
#else
        using string = std::string_view;
#endif

        template<typename T>
        struct clz_name_t {
            string id = boost::typeindex::type_id_with_cvr<T>().pretty_name();
            T data;
        };
        using named_products = std::tuple<clz_name_t<products>...>;
        // using _T = typename std::conditional<unique, std::unique_ptr<product_base>, std::shared_ptr<product_base>>::type;

        template<typename... Args>
        static auto create(const string &id, Args &&...args) {
            std::unique_ptr<product_base> result{};

            std::apply([](auto &&...it) {
                           ((static_check<decltype(it.data)>()), ...);
                       },
                       named_products{});

            std::apply([&](auto &&...it) {
                           ((it.id == id ? result = std::make_unique<decltype(it.data)>(args...) : result), ...);
                       },
                       named_products{});
            return result;
        }

        template<typename... Args>
        static std::shared_ptr<product_base> make_shared(const string &id, Args &&...args) {
            std::shared_ptr<product_base> ptr = create(id, args...);
            return ptr;
        }

        template<typename... Args>
        static std::unique_ptr<product_base> make_unique(const string &id, Args &&...args) {
            return create(id, args...);
        }

        template<typename... Args>
        static product_base *create_nacked_ptr(const string &id, Args &&...args) {
            return create(id, args...).release();
        }

    private:
        template<typename product>
        static void static_check() {
            static_assert(std::is_base_of<product_base, product>::value, "all products must inherit from product_base");
        }
    }; // class factory


    template<class T>
    inline bool compare_vector_values(std::vector<T> const &v1, std::vector<T> const &v2) {
        bool not_ok = false;
        if (v1.size() == v2.size()) {
            for (std::size_t i = 0; i < v1.size(); i++) {
                if (std::strcmp(v1[i], v2[i]) != 0) {
                    not_ok = true;
                    break;
                }
            }
        } else {
            not_ok = true;
        }
        return (not_ok == false);
    }
}

