module;

#include <algorithm>

#include "utils/macro_definition.hpp"
#include <algorithm>
#include <charconv>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exception>
#include <expected>
#include <experimental/source_location>
#include <filesystem>
#include <format>
#include <forward_list>
#include <functional>
#include <future>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <random>
#include <set>
#include <shared_mutex>
#include <sstream>
#include <stacktrace>
#include <string>
#include <thread>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

export module stl;

export namespace std {
/*using std::experimental::source_location;
using std::source_location;*/
// using std::stringstream;
// [utility.swap], swap
using std::swap;

// [utility.exchange], exchange
using std::exchange;

// [forward], forward/move
using std::forward;
#ifdef IS_CPP23_OR_LATER
using std::forward_like;
#endif
using std::move;
using std::move_if_noexcept;

using std::identity;

using std::not_fn;

using std::equal_to;
using std::greater;
using std::greater_equal;
using std::less;
using std::less_equal;
using std::not_equal_to;

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
using std::format_args;
using std::make_format_args;
using std::print;
using std::printf;
using std::println;
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
using std::map;
using std::priority_queue;
using std::queue;
using std::set;
using std::string;
using std::unordered_map;
using std::unordered_set;
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
} // namespace ranges

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
} // namespace chrono

using std::cerr;
using std::cout;
using std::end;
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
using std::filesystem::create_directories;
using std::filesystem::create_directory;
using std::filesystem::directory_entry;
using std::filesystem::directory_iterator;
using std::filesystem::exists;
using std::filesystem::file_size;
using std::filesystem::is_directory;
using std::filesystem::path;
using std::filesystem::recursive_directory_iterator;
using std::filesystem::remove;
using std::filesystem::remove_all;
using std::filesystem::temp_directory_path;
} // namespace filesystem

using std::iota;
using std::mt19937;
using std::uniform_real_distribution;

using std::bad_alloc;
using std::bad_array_new_length;
using std::bad_cast;
using std::bad_exception;
using std::bad_function_call;
using std::bad_optional_access;
using std::bad_typeid;
using std::bad_weak_ptr;
using std::exception;

using std::back_inserter;
using std::hash;

using std::streamsize;
using std::string_view;

inline namespace literals {
inline namespace string_view_literals {

using std::literals::string_view_literals::operator""sv;

} // namespace string_view_literals
} // namespace literals

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
using std::monostate;
using std::optional;

using std::async;
using std::future;
using std::jthread;
using std::promise;
using std::thread;

namespace this_thread {
using std::this_thread::sleep_for;
using std::this_thread::sleep_until;
using std::this_thread::yield;
} // namespace this_thread

using std::atomic_flag;

using std::enable_shared_from_this;
using std::make_shared;
using std::make_unique;
using std::shared_ptr;
using std::unique_ptr;
using std::weak_ptr;

// [coroutine.traits], coroutine traits
using std::coroutine_traits;

// [coroutine.handle], coroutine handle
using std::coroutine_handle;

// [coroutine.handle.compare], comparison operators
using std::operator==;
using std::operator<=>;

using std::operator<;
using std::operator>;
using std::operator<=;
using std::operator>=;
using std::operator+;
using std::operator-;
using std::operator|;
using std::operator<<;
using std::operator>>;

// [coroutine.handle.hash], hash support
using std::hash;

// [coroutine.noop], no-op coroutines
using std::noop_coroutine;
using std::noop_coroutine_handle;
using std::noop_coroutine_promise;

// [coroutine.trivial.awaitables], trivial awaitables
using std::suspend_always;
using std::suspend_never;

#ifdef IS_CPP23_OR_LATER
// [expected.unexpected], class template unexpected
using std::unexpected;

// [expected.bad], class template bad_expected_access
using std::bad_expected_access;

// in-place construction of unexpected values
using std::unexpect;
using std::unexpect_t;

// [expected.expected], class template expected
using std::expected;

// stacktrace
using std::stacktrace;
#endif // _LIBCPP_STD_VER >= 23

using std::int16_t;
using std::int32_t;
using std::int64_t;
using std::int8_t;
using std::intmax_t;
using std::ptrdiff_t;
using std::size_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;
using std::uint8_t;
using std::uintmax_t;
using std::uintptr_t;

using std::bool_constant;
using std::false_type;
using std::index_sequence;
using std::integer_sequence;
using std::integral_constant;
using std::make_index_sequence;
using std::make_integer_sequence;
using std::true_type;

using std::add_const_t;
using std::add_pointer_t;
using std::add_volatile_t;
using std::common_type;
using std::common_type_t;
using std::conditional_t;
using std::decay_t;
using std::extent_v;
using std::invoke_result_t;
using std::is_array_v;
using std::is_assignable_v;
using std::is_const_v;
using std::is_constant_evaluated;
using std::is_copy_assignable_v;
using std::is_copy_constructible_v;
using std::is_default_constructible_v;
using std::is_empty_v;
using std::is_enum_v;
using std::is_function_v;
using std::is_lvalue_reference_v;
using std::is_move_assignable_v;
using std::is_move_constructible_v;
using std::is_nothrow_constructible_v;
using std::is_nothrow_copy_assignable_v;
using std::is_nothrow_copy_constructible_v;
using std::is_nothrow_destructible_v;
using std::is_nothrow_move_assignable_v;
using std::is_nothrow_move_constructible_v;
using std::is_nothrow_swappable_v;
using std::is_null_pointer_v;
using std::is_pointer_v;
using std::is_reference_v;
using std::is_rvalue_reference_v;
using std::is_standard_layout_v;
using std::is_trivial_v;
using std::is_trivially_copy_assignable_v;
using std::is_trivially_copy_constructible_v;
using std::is_trivially_copyable_v;
using std::is_trivially_default_constructible_v;
using std::is_trivially_destructible_v;
using std::is_trivially_move_assignable_v;
using std::is_trivially_move_constructible_v;
using std::is_void_v;
using std::is_volatile_v;
using std::make_signed_t;
using std::make_unsigned_t;
using std::remove_const_t;
using std::remove_cv_t;
using std::remove_cvref_t;
using std::remove_pointer_t;
using std::remove_reference_t;
using std::underlying_type_t;

} // namespace std