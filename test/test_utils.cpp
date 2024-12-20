import stl;
import sys_utils;
import queue_utils;
import collect_utils;

#include <gtest/gtest.h>
#include "utils/log.h"
#include <ranges>
//#include <utility>
#include <iostream>
#include <format>

using namespace fast::util;

std::expected<int, std::string> safe_divide(int a, int b)
{
    if (b == 0) {
        return std::unexpected("Division by zero!");
    }
    return a / b;
}


TEST(LockFreeMap_test, test_utils)
{
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
    const int        field_num  = table_meta.field_num() - table_meta.sys_field_num();
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
void processVariant(const VarType& var) {
    std::visit([](auto&& arg) {
        std::cout << "Value: " << arg << "\n";
    }, var);
}


void processVariantConstexpr(const VarType& var) {
    std::visit([](auto&& arg) {
        using T = std::decay_t<decltype(arg)>; // 获取 arg 的类型
        if constexpr (std::is_same_v<T, int>) {
            std::cout << "Processing int: " << arg * 2 << "\n"; // 对 int 进行处理
        } else if constexpr (std::is_same_v<T, double>) {
            std::cout << "Processing double: " << arg + 1.0 << "\n"; // 对 double 进行处理
        } else if constexpr (std::is_same_v<T, std::string>) {
            std::cout << "Processing string: " << arg << " World!\n"; // 对 string 进行处理
        }
    }, var);
}


TEST(range_group, student)
{

    struct Student
    {
        std::string name;
        int         grade;

        Student(std::string n, const int g)
            : name(std::move(n)),
              grade(g)
        {
        }
    };

    // 创建一个学生的 list
    std::list<Student> students = {
        {"Alice", 1},
        {"Bob", 2},
        {"Charlie", 1},
        {"David", 2},
        {"Eve", 3},
        {"Frank", 1}
    };

    std::vector<int> numbers = {1, 2, 3, 4, 5, 6};
    auto taken = numbers | std::views::take_while([](int n) { return n < 4; });

    for (int n : taken) {
        std::cout << n << " "; // 输出: 1 2 3
    }

    std::vector<int> numberss = {1, 2, 3, 4, 5};
    auto squares = numberss | std::views::transform([](int n) { return n * n; }) | std::ranges::to<std::vector<int>>();

    for (int n : squares) {
        std::cout << n << " "; // 输出: 1 4 9 16 25
    }

    int value = 42;
    std::string formatted = std::format("The answer is: {}", value);
    std::cout << formatted << std::endl; // 输出: The answer is: 42

    using namespace std::chrono;
    auto today = system_clock::now();
    auto tomorrow = today + days(1);

    // 使用 std::print
    std::string basic_string = std::format("Tomorrow's timestamp: {},", tomorrow.time_since_epoch().count());// 输出: Name: Alice, Age: 30
    std::cout << std::format("Tomorrow's timestamp: {},", tomorrow.time_since_epoch().count()) << std::endl;

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
    std::visit([](auto&& arg) {
        std::cout << "Variant value: " << arg << "\n";
    }, v3);


    processVariantConstexpr(v1); // 输出: Processing int: 20
    processVariantConstexpr(v2); // 输出: Processing double: 3.5
    processVariantConstexpr(v3); // 输出: Processing string: Hello World!
}