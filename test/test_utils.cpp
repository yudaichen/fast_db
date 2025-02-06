//import stl;
import sys_utils;
import queue_utils;
import collect_utils;
import robin_hood;
import BS.thread_pool;

// #include <stacktrace>
#include "utils/log.h"

#include <ranges>
// #include <utility>
#include <format>
#include <iostream>

#include "doctest.h"
#include <expected>
#include <future>
#include <variant>
#include <iostream>
#include <sstream>
#include <list>


// #include <execution>
// #include "redis/redis_asio.hpp"
using namespace fast::util;

std::expected<int, std::string> safe_divide(int a, int b) {
  if (b == 0) {
    return std::unexpected("Division by zero!");
  }
  return a / b;
}

DOCTEST_TEST_CASE("LockFreeMap_test, test_utils") {
  SafeQueue<std::string> safe_queue;

  std::string insertStr = "sadasd";
  safe_queue.enqueue(insertStr);

  LockFreeList<std::string> lock_free_list;
  lock_free_list.insert("One");

  LockFreeMap<int, std::string> lfmap(10);
  lfmap.insert(1, "One");
  lfmap.insert(2, "Two");
  lfmap.insert(3, "Three");

  /*
  // check the fields number
  const Value     *values     = inserts.values.data();
  const int        value_num  = static_cast<int>(inserts.values.size());
  const TableMeta &table_meta = table->table_meta();
  const int        field_num  = table_meta.field_num() -
  table_meta.sys_field_num();
  */

  std::cout << std::format("Map size: {}", lfmap.size()) << std::endl;

  lfmap.insert(1, "One");
  lfmap.insert(2, "Two");
  lfmap.insert(3, "Three");

  std::cout << "Map size: " << lfmap.size() << std::endl;

  lfmap.remove(2);

  std::cout << "Map size after removal: " << lfmap.size() << std::endl;

  auto result = safe_divide(10, 2);
  if (result.has_value()) {
    std::cout << "Result: " << result.value() << std::endl;
  } else {
    std::cout << "Error: " << result.error() << std::endl;
  }

  result = safe_divide(10, 0);
  if (result.has_value()) {
    std::cout << "Result: " << result.value() << std::endl;
  } else {
    std::cout << "Error: " << result.error() << std::endl;
  }

  /*lfmap.clear();

  std::cout << "Map size after clear: " << lfmap.size() << std::endl;*/
}

void printSpan(std::span<int> s) {
  for (int n : s) {
    std::cout << n << " ";
  }
}

std::optional<int> findValue(bool found) {
  if (found) {
    return 42; // 返回一个值
  }
  return std::nullopt; // 返回空值
}

using VarType = std::variant<int, double, std::string>;

void processVariant(const VarType &var) {
  std::visit([](auto &&arg) { std::cout << "Value: " << arg << "\n"; }, var);
}

void processVariantConstexpr(const VarType &var) {
  std::visit(
      [](auto &&arg) {
        using T = std::decay_t<decltype(arg)>; // 获取 arg 的类型
        if constexpr (std::is_same_v<T, int>) {
          std::cout << "Processing int: " << arg * 2 << "\n"; // 对 int 进行处理
        } else if constexpr (std::is_same_v<T, double>) {
          std::cout << "Processing double: " << arg + 1.0
                    << "\n"; // 对 double 进行处理
        } else if constexpr (std::is_same_v<T, std::string>) {
          std::cout << "Processing string: " << arg
                    << " World!\n"; // 对 string 进行处理
        }
      },
      var);
}

DOCTEST_TEST_CASE("range_group, student") {

  struct Student {
    std::string name;
    int grade;

    Student(std::string n, const int g) : name(std::move(n)), grade(g) {}
  };

  // 创建一个学生的 list
  std::list<Student> students = {{"Alice", 1}, {"Bob", 2}, {"Charlie", 1},
                                 {"David", 2}, {"Eve", 3}, {"Frank", 1}};

  std::vector<int> numbers    = {1, 2, 3, 4, 5, 6};
  auto taken = numbers | std::views::take_while([](int n) { return n < 4; });

  for (int n : taken) {
    std::cout << n << " "; // 输出: 1 2 3
  }

  /*
  std::vector<int> numberss = {1, 2, 3, 4, 5};
  auto squares = numberss | std::views::transform([](int n) { return n * n; }) |
                 std::ranges::to<std::vector<int>>();

  for (int n : squares) {
    std::cout << n << " "; // 输出: 1 4 9 16 25
  }*/

  int value             = 42;
  std::string formatted = std::format("The answer is: {}", value);
  std::cout << formatted << std::endl; // 输出: The answer is: 42

  using namespace std::chrono;
  auto today               = system_clock::now();
  auto tomorrow            = today + days(1);

  // 使用 std::print
  std::string basic_string = std::format("Tomorrow's timestamp: {},",
                                         tomorrow.time_since_epoch().count());
  // 输出: Name: Alice, Age: 30
  std::cout << std::format("Tomorrow's timestamp: {},",
                           tomorrow.time_since_epoch().count())
            << std::endl;

  int arr[] = {1, 2, 3, 4, 5};
  printSpan(arr); // 输出: 1 2 3 4 5

  std::optional<int> values = findValue(true);
  if (values) {
    std::cout << "Found value: " << *values << "\n"; // 输出: Found value: 42
  } else {
    std::cout << "Value not found\n";
  }

  VarType v1 = 10;
  VarType v2 = 3.14;
  VarType v3 = std::string("Hello");

  processVariant(v1);
  processVariant(v2);
  processVariant(v3);

  // 使用 std::visit 处理不同类型
  std::visit([](auto &&arg) { std::cout << "Variant value: " << arg << "\n"; },
             v3);

  //    LOG_INFO_R << boost::stacktrace::stacktrace();
  // LOG_STACK2_R;

  try {

    throw std::runtime_error("Something went wrong");
  } catch (...) {
    // 使用Boost Stacktrace捕获当前栈信息
    std::stringstream ss;
    ss << boost::stacktrace::stacktrace();

    // 将栈信息输出到控制台
    std::cout << "Caught exception, stack trace:" << ss.str() << std::endl;
  }

  // LOG_STACK_R;

  processVariantConstexpr(v1); // 输出: Processing int: 20
  processVariantConstexpr(v2); // 输出: Processing double: 3.5
  processVariantConstexpr(v3); // 输出: Processing string: Hello World!
}

DOCTEST_TEST_CASE("test_modules, hobin_hood") {
  robin_hood::unordered_flat_map<int, int> map;
  map[1] = 123;
  std::cout << "hello, world! " << map.size() << std::endl;
}



DOCTEST_TEST_CASE("BS.thread_pool_get") {
  BS::thread_pool pool(1);
  std::future<void> submit_task = pool.submit_task(
      []() {
        std::cout << "BS::thread_pool pool" << "\n";
        LOG_STACK2_R;
        /*for (const auto &frame : st) {
            std::cout << frame.description() << " at " << frame.source_file() <<
        ":" << frame.source_line() << std::endl;
        }*/

        /*try {
            throw std::runtime_error("An error occurred in func3");
        } catch (const std::exception &e) {
            std::cout << "Exception caught: " << e.what() << std::endl;
            // 捕获堆栈信息并输出
            std::cout << std::stacktrace(); // 输出当前堆栈跟踪信息
        }*/
      },
      1);
  submit_task.wait();
}

/*
#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "type_format.h"

struct foo{
    int x;
    double y;
    std::unordered_set<int> s;
    std::unordered_map<std::string, int> m;
};

template<typename _Type>
struct bar{
    std::tuple<foo, std::string, std::string> t;
    std::vector<std::string_view> v;
    _Type z;
    _Type w;
    char s[2] = "S";
    char n[2] = "N";
    char e[0];
    char m[3] = "NN";
    char l[0];
};

struct foobar{
public:
    template<std::convertible_to<bar<std::string>> _Cookie>
    foobar(_Cookie &&_cookie): cookie(std::forward<_Cookie>(_cookie)){}

private:
    int password = 114514;
    std::string secret = "embarrassing fact";
    bar<std::string> cookie;
};

// steal private members.
template<>
struct fake::tool::steal<[]{}, fake::tool::adaptor<foobar>, &foobar::password,
&foobar::secret, &foobar::cookie>{
    // register meta-implementations for token-based-cpo
'fake::for_each<fake::cpo::format<...>>' at compile-time. using token =
fake::cpo::format<fake::io::token::pretty>;
    static_assert(fake::custom::for_each::config::emplace_visitor<[]{}, token,
steal>());
};

DOCTEST_TEST_CASE("test fake printf")
{
    using namespace std::string_literals;

    bar<std::string> var{
                    {{114, 514.0, {1919, 893}, {{"ya", 8}, {"ju", 10}}}, "MGR",
"YUH"},
            {"SZ", "JOKER"},
            "DIYUSI"s,
            "NEL"s
        };

    foobar lost_owner{std::move(var)};

    std::cout << fake::io::pretty<>(lost_owner) << std::endl;
}
*/
#if CLANG



void stacktrace_stringstream(std::stringstream &ss, const std::stacktrace &st) {
  ss << "\nStack trace start:\n";
  int i = 0;
  for (const auto &frame : st) {
    ss << std::format(
        "{:4}: {} info \n {}:{}\n\n", i++,
        frame.description().c_str(), // Demangle for human-readable names
        frame.source_file(), frame.source_line());
  }
  ss << "Stack trace end...";
}

void stacktrace_vformat(std::string &output, const std::stacktrace &st) {
  output = "\nStack trace start:\n";
  int i  = 0;
  for (const auto &frame : st) {
    // 使用 std::string_view 避免复制
    std::string description = frame.description();
    std::string file        = frame.source_file();
    std::string line        = std::to_string(++i);
    std::string source_line = std::to_string(frame.source_line());
    output += std::vformat(
        "{:4}: {} info \n {}:{}\n\n",
        std::make_format_args(line, description, file, source_line));
  }
  output += "Stack trace end...";
}

DOCTEST_TEST_CASE("Stacktrace Performance") {
  const int iterations =
      20000; // Adjust the iteration count for more accurate results
  std::stacktrace st = std::stacktrace::current();

  DOCTEST_SUBCASE("std::stringstream") {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
      std::stringstream ss;
      stacktrace_stringstream(ss, st);
      volatile std::string str = ss.str(); // Prevent compiler optimization
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "std::stringstream: " << duration.count() << "ms";
  }

  DOCTEST_SUBCASE("std::vformat") {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
      std::string output;
      stacktrace_vformat(output, st);
      volatile std::string str = output; // Prevent compiler optimization
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout<< "std::vformat: " << duration.count() << "ms";
    }
}
#endif