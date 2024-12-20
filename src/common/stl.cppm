module;

#include <algorithm>

#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exception>
#include <experimental/source_location>
#include <filesystem>
#include <forward_list>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <vector>
#include <map>
#include <queue>
#include <memory>
#include <optional>
#include <random>
#include <set>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <thread>
#include <future>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <expected>
#include <algorithm>

export module stl;

export namespace std {
    /*using std::experimental::source_location;
    using std::source_location;*/
    // using std::stringstream;
    using std::size_t;

    using std::forward;
    using std::move;

    using std::swap;

    using std::max;
    using std::min;

    using std::errc;
    using std::from_chars;
    using std::to_string;

    using std::stoi;
    using std::strtod;
    using std::strtof;
    using std::strtol;

    using std::tolower;
    using std::toupper;
    /*using std::memcpy;
    using std::memset;
    using std::memcmp;
    using std::strlen;*/
    using std::memchr;
    using std::memcmp;
    using std::memcpy;
    using std::memmove;
    using std::memset;
    using std::strcat;
    using std::strchr;
    using std::strcmp;
    using std::strcoll;
    using std::strcpy;
    using std::strcspn;
    using std::strerror;
    using std::strlen;
    using std::strncat;
    using std::strncmp;
    using std::strncpy;
    using std::strpbrk;
    using std::strrchr;
    using std::strspn;
    using std::strstr;
    using std::strtok;
    using std::strxfrm;

    using std::format;
    using std::print;
    using std::println;
    using std::printf;
    using std::time;

    using std::fill;
    using std::is_same;
    using std::lower_bound;

    using std::condition_variable;
    using std::condition_variable_any;
    using std::lock_guard;
    using std::memory_order;
    using std::memory_order_acq_rel;
    using std::memory_order_acquire;
    using std::memory_order_consume;
    using std::memory_order_relaxed;
    using std::memory_order_release;
    using std::memory_order_seq_cst;
    using std::mutex;
    using std::scoped_lock;
    using std::shared_lock;
    using std::shared_mutex;
    using std::unique_lock;

    using std::forward_list;
    using std::list;
    using std::priority_queue;
    using std::queue;
    using std::string;
    using std::vector;

    using std::binary_search;
    using std::fabs;
    using std::fill_n;
    using std::find;
    using std::floor;
    using std::fmod;
    using std::isalnum;
    using std::isalpha;
    using std::isinf;
    using std::isnan;
    using std::log2;
    using std::make_heap;
    using std::nearbyint;
    using std::pop_heap;
    using std::pow;
    using std::remove_if;
    using std::reverse;
    using std::reverse_copy;
    using std::sort;
    using std::sqrt;
    using std::transform;
    using std::unique;

    namespace ranges {
    using std::ranges::equal;
    using std::ranges::for_each;
    }    // namespace ranges

    using std::decay_t;
    using std::function;
    using std::numeric_limits;

    namespace chrono {
    using std::chrono::duration;
    using std::chrono::microseconds;
    using std::chrono::milliseconds;
    using std::chrono::nanoseconds;
    using std::chrono::seconds;
    using std::chrono::system_clock;

    using std::chrono::operator>;
    using std::chrono::operator>=;
    using std::chrono::operator<;
    using std::chrono::operator<=;
    using std::chrono::operator==;

    using std::chrono::operator+;
    using std::chrono::operator-;

    using std::chrono::minutes;
    using std::chrono::weeks;
    using std::chrono::years;

    using std::chrono::steady_clock;
    using std::chrono::time_point;
    }    // namespace chrono

    using std::cerr;
    using std::cout;
    using std::endl;

    using std::ifstream;
    using std::ios;
    using std::ofstream;
    using std::ostream;

    using std::align;

    using std::ptrdiff_t;

    using std::dynamic_pointer_cast;
    using std::static_pointer_cast;

    namespace filesystem {
    using std::filesystem::file_size;
    }

    using std::iota;
    using std::mt19937;
    using std::uniform_real_distribution;

    using std::exception;
    using std::unexpected;
    using std::unordered_set;
    using std::expected;

    using std::back_inserter;
    using std::hash;

    using std::streamsize;
    using std::string_view;

    using std::get;
    using std::holds_alternative;
    using std::variant;
    using std::variant_size_v;
    using std::visit;

    using std::common_type_t;
    using std::is_floating_point_v;
    using std::is_integral_v;
    using std::underlying_type_t;


    using std::function;
    using std::optional;
    using std::monostate;

    using std::async;
    using std::future;
    using std::jthread;
    using std::promise;
    using std::thread;
    namespace this_thread {
    using std::this_thread::sleep_for;
    }
    using std::atomic_flag;

}    // namespace std
