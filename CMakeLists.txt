cmake_minimum_required(VERSION 3.28.3)
set(CMAKE_C_COMPILER "/usr/bin/clang-19")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++-19")
set(CMAKE_CXX_COMPILER_ID "Clang")
set(APP_NAME fast_experience)
project(${APP_NAME})

#c++23 标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # 禁用编译器特定扩展

#用于指定 CMake 是否将扫描目标中C++源以查找模块依赖项
#https://cmake.org/cmake/help/latest/prop_tgt/CXX_SCAN_FOR_MODULES.html#prop_tgt:CXX_SCAN_FOR_MODULES
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

# 根据构建类型设置优化标志
message(STATUS "Build type = ${CMAKE_BUILD_TYPE}")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-Ofast -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g -DNDEBUG")


# 使用test文件
set(USE_TEST ON)
set(UES_PRECOMPILE ON)
set(USE_OPTIMIZE ON)
set(USE_MODULES ON)

#使用模块编译
function(fn_modules)
    # 检测编译器
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

        if (NOT CMAKE_GENERATOR STREQUAL "Ninja")
            message(FATAL_ERROR "This project requires the Ninja generator. Refers to https://cmake.org/cmake/help/latest/manual/cmake-cxxmodules.7.html#generator-support")
        endif ()

        execute_process(COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE clang_full_version_string)
        string(REGEX REPLACE ".*clang version ([0-9]+\\.[0-9]+).*" "\\1" CLANG_VERSION_STRING ${clang_full_version_string})
        message(STATUS "Using Clang with Clang-${CLANG_VERSION_STRING} as the build.")
        if (CLANG_VERSION_STRING VERSION_GREATER_EQUAL 17)
            set(CMAKE_CXX_FLAGS -Wall -Werror -Wno-sign-compare -Wunused-result -Wno-deprecated-declarations -Wextra -Wno-unused-parameter -Wno-unused-private-field -pthread -fcolor-diagnostics)
            # 使用 Clang 时，设置使用 lld 链接器
            set(CMAKE_CXX_LINKER "lld-19")
            set(CMAKE_C_LINKER "lld-19")
        else()
            message(FATAL_ERROR "Please use clang version 17.0 and above")
        endif()
        set(CMAKE_EXE_LINKER_FLAGS PRIVATE -static-libstdc++ -static-libgcc -z noexecstack)

    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # 使用 GNU 时，设置使用 Gold 链接器
        set(CMAKE_CXX_LINKER "ld.gold")
        set(CMAKE_C_LINKER "ld.gold")
        message(STATUS "Using GNU with gold as the linker.")
    else ()
        message(WARNING "Unsupported compiler. Default linker will be used.")
    endif ()
endfunction()


function(fn_optimize)
    # 设置优化选项
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        message(STATUS "Setting Release build optimizations")
        if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            #启用链接时优化（LTO）
            target_compile_options(${APP_NAME} PRIVATE -O3 -flto)
            target_link_options(${APP_NAME} PRIVATE -flto)

            # 对GCC和Clang，启用分割调试信息
            set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -Wl,--gdb-index")
        elseif (MSVC)
            # 启用优化02  启用链接时代码生成LTCG
            target_compile_options(${APP_NAME} PRIVATE /O2 /LTCG)
        endif ()
    elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "Debug build, no optimizations")
        # 可以在调试模式下添加其他选项
    endif ()

    MESSAGE(STATUS "C++ Compilation flags: " ${CMAKE_CXX_FLAGS})

    # 编译期性能优化
    find_program(CCACHE_PROGRAM ccache)
    if (CCACHE_PROGRAM)
        message(STATUS "Set up ccache ...")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    endif ()
endfunction()

function(fn_find_package)
    # 寻找第三方包
    find_package(Threads REQUIRED)
    find_package(Boost REQUIRED)
    find_package(nlohmann_json REQUIRED)
    find_package(yaml-cpp CONFIG REQUIRED)
    find_package(glog REQUIRED)
endfunction()

function(fn_build)
    #ubuntu 库文件目录
    include_directories(/usr/include)

    #项目内头文件
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

    # 本项目 内置头文件目录
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cppcoro/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/doctest/doctest)


    # 头文件
    file(GLOB_RECURSE CURRENT_HEADERS
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp

            # cppcoro
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cppcoro/include/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cppcoro/include/*.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cppcoro/lib/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cppcoro/lib/*.hpp

            # doctest
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/doctest/doctest/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/doctest/doctest/*.hpp)

    # 源文件
    file(GLOB_RECURSE SOURCE_FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.ui
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.qrc

            # cppcoro
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cppcoro/include/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cppcoro/include/*.c
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cppcoro/lib/*.cc

            # doctest
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/doctest/doctest/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/doctest/doctest/*.c
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/doctest/doctest/*.cc
    )

    # 使用test文件
    if (USE_TEST)
        #测试文件
        set(TEST_FILE
               #[[
                ${CMAKE_CURRENT_SOURCE_DIR}/test/std_coroutine.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/test/raft_test.cpp]]
                ${CMAKE_CURRENT_SOURCE_DIR}/run/test_main.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/test/test_utils.cpp

        )

        file(GLOB_RECURSE TEST_RAFT
                ${CMAKE_CURRENT_SOURCE_DIR}/test/raft/*.h
                ${CMAKE_CURRENT_SOURCE_DIR}/test/raft/*.cc

        )

       #[[
        # gtest
        add_test(test_all ${APP_NAME})
        ]]


    else ()
        #可执行文件
        set(RUN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/run/main.cpp)
    endif ()

    add_executable(${APP_NAME}
            ${CURRENT_HEADERS}
            ${SOURCE_FILES}
            ${RUN_FILE}
            ${TEST_FILE}
            ${TEST_RAFT}
    )

    # 预编译头文件
    if (UES_PRECOMPILE)
        target_precompile_headers(
                ${APP_NAME} PRIVATE
                <boost/asio.hpp>
                <nlohmann/json.hpp>
                <yaml-cpp/yaml.h>
                <glog/logging.h>
        )
    endif ()

    if (USE_MODULES)
        # 模块文件
        file(GLOB_RECURSE MODULES_FILES
                CONFIGURE_DEPENDS
                ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cppm
        )
        #设置模块代码
        target_sources(${APP_NAME}
                PUBLIC
                FILE_SET core_cxx_modules TYPE CXX_MODULES FILES
                ${MODULES_FILES}
        )

    endif ()

    # 添加库文件搜索路径
    link_directories(/usr/lib/x86_64-linux-gnu)
    target_link_directories(${APP_NAME} PRIVATE /usr/local/lib)
    target_link_libraries(
            ${APP_NAME}
            pthread
            ${Boost_LIBRARIES}
            glog
            #[[gtest]]
            gflags
            nlohmann_json
            libyaml-cpp.so
    )
endfunction()

function(help_conan_1)

    note("\n\n\n\n*****************************use conan1.******************************\n")
    note("debug:     conan install . -s build_type=Debug --install-folder=./cmake/debug")
    note("release:   conan install . -s build_type=Debug --install-folder=./cmake/release\n
    libgtest-dev ccache libgrpc++-dev libbenchmark-dev librange-v3-dev libzmq3-dev nlohmann-json3-dev libleveldb-dev libfmt-dev libyaml-cpp-dev liburing-dev")
    note("\n*****************************end conan*****************************\n\n\n\n")

endfunction()

function(help_conan_2)

    note("\n\n\n\n*****************************use conan2.******************************\n")
    note("debug:   cd cmake/debug  conan install ../.. -of . --build missing")
    note("release: cd cmake/debug  conan install ../.. -of . --build missing\n")
    note("-of   conan_toolchain.cmake 文件输出目录")
    note("*****************************end conan*****************************\n\n\n\n")

endfunction()



fn_modules()
fn_optimize()
fn_find_package()
fn_build()