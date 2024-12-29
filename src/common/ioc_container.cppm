module;

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <ostream>
#include <typeindex>
#include <unordered_map>

export module ioc_container;

export namespace fast::util {

class Any {
public:
  Any() : m_typeIndex(std::type_index(typeid(void))) {}

  Any(Any &other) : m_basePtr(other.clone()), m_typeIndex(other.m_typeIndex) {}

  Any(Any &&other)
      : m_basePtr(std::move(other.m_basePtr)), m_typeIndex(other.m_typeIndex) {}

  // 创建智能指针时，对于一般的类型，通过std::decay来移除引用和cv符，从而获得原始类型.
  template <
      typename U,
      class = typename std::enable_if<
          !std::is_same<typename std::decay<U>::type, Any>::value, U>::type>
  Any(U &&value)
      : m_basePtr(
            new Derived<typename std::decay<U>::type>(std::forward<U>(value))),
        m_typeIndex(std::type_index(typeid(typename std::decay<U>::type))) {}

  bool isNull() const { return !bool(m_basePtr); }

  template <typename U> bool is() const {
    return m_typeIndex == std::type_index(typeid(U));
  }

  // 将Any转换为实际的类型.
  template <typename U> U &anyCast() {
    if (!is<U>()) {
      std::cout << "Can not cast " << typeid(U).name() << " to "
                << m_typeIndex.name() << std::endl;
      throw std::bad_cast();
    }

    auto dervied = dynamic_cast<Derived<U> *>(m_basePtr.get());
    return dervied->m_value;
  }

  Any &operator=(const Any &other) {
    if (m_basePtr == other.m_basePtr) {
      return *this;
    }

    m_basePtr   = other.clone();
    m_typeIndex = other.m_typeIndex;
    return *this;
  }

private:
  class Base;

  using BasePtr = std::unique_ptr<Base>;

  class Base {
  public:
    virtual ~Base() {}

    virtual BasePtr clone() const = 0;
  };

  template <typename T> class Derived : public Base {
  public:
    template <typename U>
    Derived(U &&value) : m_value(std::forward<U>(value)) {}

    virtual BasePtr clone() const { return BasePtr(new Derived<T>(m_value)); }

    T m_value;
  };

  BasePtr clone() const {
    if (m_basePtr != nullptr) {
      return m_basePtr->clone();
    }
    return nullptr;
  }

  BasePtr m_basePtr;
  std::type_index m_typeIndex;
};

class IocContainer {
public:
  IocContainer()                                = default;

  IocContainer(const IocContainer &)            = delete;

  IocContainer &operator=(const IocContainer &) = delete;

  template <typename T, typename Depend, typename... Args>
  typename std::enable_if<!std::is_base_of<T, Depend>::value>::type
  registerType(const std::string &key) {
    // 通过闭包擦除参数类型
    std::function<T *(Args...)> func = [](Args... args) {
      return new T(new Depend(args...));
    };
    registerType(key, func);
  }

  template <typename T, typename Depend, typename... Args>
  typename std::enable_if<std::is_base_of<T, Depend>::value>::type
  registerType(const std::string &key) {
    // 通过闭包擦除参数类型
    std::function<T *(Args...)> func = [](Args... args) {
      return new Depend(args...);
    };
    registerType(key, func);
  }

  template <typename T, typename... Args>
  void registerSimple(const std::string &key) {
    std::function<T *(Args...)> func = [](Args... args) {
      return new T(args...);
    };
    registerType(key, func);
  }

  template <typename T, typename... Args>
  std::shared_ptr<T> resolveShared(const std::string &key, Args... args) {
    T *t = resolve<T>(key, args...);
    return std::shared_ptr<T>(t);
  }

  template <typename T, typename... Args>
  T *resolve(const std::string &key, Args... args) {
    auto iter = m_creatorMap.find(key);
    if (iter != m_creatorMap.end()) {
      Any resolver = iter->second;
      std::function<T *(Args...)> func =
          resolver.anyCast<std::function<T *(Args...)>>();
      return func(args...);
    }

    return nullptr;
  }

private:
  void registerType(const std::string &key, Any constructor) {
    auto iter = m_creatorMap.find(key);
    if (iter != m_creatorMap.end()) {
      std::string errorString = key + " key has already exist.";
      throw std::invalid_argument(errorString);
    }

    // 通过Any擦除不同类型的构造器
    m_creatorMap.emplace(key, constructor);
  }

private:
        std::unordered_map<std::string, Any> m_creatorMap;
    };

}
