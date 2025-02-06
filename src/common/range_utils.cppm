module;

#include <algorithm>
#include <map>

export module range_utils;

export namespace fast::util {
// flat_map to std::map
template <typename R> class RangeUtils {
public:
  typedef typename R::value_type value_type;

  explicit RangeUtils(R &range) { this->m_range = range; }

  ~RangeUtils() = default;

  template <typename key_type, typename Fn>
  auto group_by(const Fn &f) -> std::multimap<key_type, value_type> {

    std::multimap<key_type, value_type> mymap;
    std::for_each(begin(m_range), end(m_range), [&mymap, &f](value_type item) {
      mymap.insert(std::make_pair(f(item), item));
    });
    return mymap;
  }

  /// @note 传两个函数对象
  template <typename key_type, typename value_type_n, typename KeyFn,
            typename ValueFn>
  auto group_by(const KeyFn &fnk, const ValueFn &fnv)
      -> std::multimap<key_type, value_type_n> {

    std::multimap<key_type, value_type_n> mymap;
    std::for_each(begin(m_range), end(m_range),
                  [&mymap, &fnk, &fnv](const value_type &item) {
                    key_type key     = fnk(item);
                    value_type_n val = fnv(item);
                    mymap.insert(std::make_pair(key, val));
                  });
    return mymap;
  }

  /// @note 传一个函数对象
  /// 以该函数对象的返回值作为键值
  /// 最终返回一个存有键值对的 multimap
  template <typename Fn>
  auto group_by(const Fn &f)
      -> std::multimap<typename std::invoke_result_t<Fn(value_type)>::type,
                       value_type> {
    // decltype(f(*((value_type*)0))),f((value_type&)nullptr)

    /// @note 定义推测的键值类型
    typedef typename std::invoke_result_t<Fn(value_type)>::type ketype;
    // typedef decltype(std::declval<Fn>()(std::declval<value_type>())) ketype;
    // typedef decltype(f(value_type())) ketype;

    /// @note 遍历 m_range （比如 vector 里的 Person ），并构造键值对插入 mymap
    std::multimap<ketype, value_type> mymap;
    std::for_each(begin(m_range), end(m_range), [&mymap, &f](value_type item) {
      mymap.insert(std::make_pair(f(item), item));
    });
    return mymap;
  }

  /// @note 传两个函数对象
  template <typename KeyFn, typename ValueFn>
  auto group_by(const KeyFn &fnk, const ValueFn &fnv) -> std::multimap<
      typename std::invoke_result_t<KeyFn(value_type)>::type,
      typename std::invoke_result_t<ValueFn(value_type)>::type> {

    typedef typename std::invoke_result_t<KeyFn(value_type)>::type ketype;
    typedef typename std::invoke_result_t<ValueFn(value_type)>::type valype;

    /// @note 遍历 m_range ，并构造键值对插入 mymap
    std::multimap<ketype, valype> mymap;
    std::for_each(begin(m_range), end(m_range),
                  [&mymap, &fnk, &fnv](const value_type &item) {
                    ketype key = fnk(item);
                    valype val = fnv(item);
                    mymap.insert(std::make_pair(key, val));
                  });
            return mymap;
        }

    private:
        R m_range;
    };


}