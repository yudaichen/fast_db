//
// Created by ydc on 24-12-23.
//

#ifndef MACRO_DEFINITION_H
#define MACRO_DEFINITION_H
import stl;
#include <cstddef>
#include <limits>

#if defined(_MSVC_LANG)
#if _MSVC_LANG >= 202302L // MSVC for C++23
#define IS_CPP23_OR_LATER
#elif _MSVC_LANG >= 202002L // MSVC for C++20
#define IS_CPP20_OR_LATER
#elif _MSVC_LANG >= 201703L
#define IS_CPP17
#elif _MSVC_LANG >= 201402L
#define IS_CPP14
#elif _MSVC_LANG >= 201103L
#define IS_CPP11
#else
#define IS_CPP98
#endif
#elif defined(__cplusplus)
#if __cplusplus > 202002L // GCC, Clang for experimental C++23 and later
#define IS_CPP23_OR_LATER
#elif __cplusplus >= 202002L
#define IS_CPP20_OR_LATER
#elif __cplusplus >= 201703L
#define IS_CPP17
#elif __cplusplus >= 201402L
#define IS_CPP14
#elif __cplusplus >= 201103L
#define IS_CPP11
#else
#define IS_CPP98
#endif
#else
#define IS_CPP98 // Unknown compiler
#endif

inline int fast_db_macro_version() {
#ifdef IS_CPP23_OR_LATER
  std::cout << "C++23 or later" << std::endl;
#elif defined(IS_CPP20_OR_LATER)
  std::cout << "C++20" << std::endl;
#elif defined(IS_CPP17)
  std::cout << "C++17" << std::endl;
#elif defined(IS_CPP14)
  std::cout << "C++14" << std::endl;
#elif defined(IS_CPP11)
  std::cout << "C++11" << std::endl;
#else
  std::cout << "C++98/03 or unknown" << std::endl;
#endif

  std::cout << "__cplusplus: " << __cplusplus << std::endl;
#ifdef _MSVC_LANG
  std::cout << "_MSVC_LANG: " << _MSVC_LANG << std::endl;
#endif
#ifdef __GNUC__
  std::cout << "__GNUC__: " << __GNUC__ << std::endl;
  std::cout << "__GNUC_MINOR__: " << __GNUC_MINOR__ << std::endl;
#endif
#ifdef __clang__
  std::cout << "__clang_major__: " << __clang_major__ << std::endl;
  std::cout << "__clang_minor__: " << __clang_minor__ << std::endl;
#endif

  return 0;
}



/// \name System Detection
/// \{
#if defined(_WIN64)
    #define PLATFORM_IS_WIN true
    #define PLATFORM_IS_WIN32 false
    #define PLATFORM_IS_WIN64 true
    #define PLATFORM_IS_LINUX false
    #define PLATFORM_IS_APPLE false
    #define PLATFORM_IS_UNIX false
    #define PLATFORM_IS_POSIX false
#elif defined(_WIN32)
    #define PLATFORM_IS_WIN true
    #define PLATFORM_IS_WIN32 true
    #define PLATFORM_IS_WIN64 false
    #define PLATFORM_IS_LINUX false
    #define PLATFORM_IS_APPLE false
    #define PLATFORM_IS_UNIX false
    #define PLATFORM_IS_POSIX false
#elif defined(__linux__)
    #define PLATFORM_IS_WIN false
    #define PLATFORM_IS_WIN32 false
    #define PLATFORM_IS_WIN64 false
    #define PLATFORM_IS_LINUX true
    #define PLATFORM_IS_APPLE false
    #define PLATFORM_IS_UNIX true
    #define PLATFORM_IS_POSIX false
#elif defined(__APPLE__)
    #define PLATFORM_IS_WIN false
    #define PLATFORM_IS_WIN32 false
    #define PLATFORM_IS_WIN64 false
    #define PLATFORM_IS_LINUX false
    #define PLATFORM_IS_APPLE true
    #define PLATFORM_IS_UNIX true
    #define PLATFORM_IS_POSIX false
#elif defined(__unix__)
    #define PLATFORM_IS_WIN false
    #define PLATFORM_IS_WIN32 false
    #define PLATFORM_IS_WIN64 false
    #define PLATFORM_IS_LINUX false
    #define PLATFORM_IS_APPLE false
    #define PLATFORM_IS_UNIX true
    #define PLATFORM_IS_POSIX false
#elif defined(_POSIX_VERSION)
    #define PLATFORM_IS_WIN false
    #define PLATFORM_IS_WIN32 false
    #define PLATFORM_IS_WIN64 false
    #define PLATFORM_IS_LINUX false
    #define PLATFORM_IS_APPLE false
    #define PLATFORM_IS_UNIX false
    #define PLATFORM_IS_POSIX true
#else
    #error UNKNOWN SYSTEM
#endif
/// \}

/// \name Compiler Detection
/// \{
#if defined(__GNUC__)
    #define PLATFORM_IS_GCC true
    #define PLATFORM_IS_CLANG false
    #define PLATFORM_IS_MSVC false
#elif defined(__clang__)
    #define PLATFORM_IS_GCC false
    #define PLATFORM_IS_CLANG true
    #define PLATFORM_IS_MSVC false
#elif defined(_MSC_VER)
    #define PLATFORM_IS_GCC false
    #define PLATFORM_IS_CLANG false
    #define PLATFORM_IS_MSVC true
#else
    #error UNKNOWN_COMPILER
#endif
/// \}

namespace platform {
    [[maybe_unused]] constexpr size_t bitness = std::numeric_limits<size_t>::digits;
    [[maybe_unused]] constexpr bool is_x64 = bitness == 64;
    [[maybe_unused]] constexpr bool is_x32 = bitness == 32;

    [[maybe_unused]] constexpr bool is_win = PLATFORM_IS_WIN;
    [[maybe_unused]] constexpr bool is_win32 = PLATFORM_IS_WIN32;
    [[maybe_unused]] constexpr bool is_win64 = PLATFORM_IS_WIN64;

    [[maybe_unused]] constexpr bool is_linux = PLATFORM_IS_LINUX;
    [[maybe_unused]] constexpr bool is_apple = PLATFORM_IS_APPLE;
    [[maybe_unused]] constexpr bool is_unix = PLATFORM_IS_UNIX;
    [[maybe_unused]] constexpr bool is_posix = PLATFORM_IS_POSIX;

    [[maybe_unused]] constexpr bool is_gcc = PLATFORM_IS_GCC;
    [[maybe_unused]] constexpr bool is_clang = PLATFORM_IS_CLANG;
    [[maybe_unused]] constexpr bool is_msvc = PLATFORM_IS_MSVC;
} // namespace platform

#ifndef COMMON_NO_COMMON_INCLUDES
    #if PLATFORM_IS_WIN
        #include <Windows.h>
    #endif
#endif

#if PLATFORM_IS_MSVC
    #define COMMON_FORCE_INLINE __forceinline
#else
    #define COMMON_FORCE_INLINE inline __attribute__((always_inline))
#endif
#endif // MACRO_DEFINITION_H
