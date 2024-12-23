//
// Created by ydc on 24-12-23.
//

#ifndef MACRO_DEFINITION_H
#define MACRO_DEFINITION_H

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

#endif //MACRO_DEFINITION_H
