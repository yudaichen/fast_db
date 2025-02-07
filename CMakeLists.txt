cmake_minimum_required(VERSION 3.28.3)
if (UNIX)
    set(CMAKE_C_COMPILER "/usr/bin/clang-20")
    set(CMAKE_CXX_COMPILER "/usr/bin/clang++-20")
    set(CMAKE_CXX_COMPILER_ID "Clang")
endif ()
set(APP_NAME fast_experience)
project(fast_experience)

#c++23 标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)  # 禁用编译器特定扩展

#用于指定 CMake 是否将扫描目标中C++源以查找模块依赖项
#https://cmake.org/cmake/help/latest/prop_tgt/CXX_SCAN_FOR_MODULES.html#prop_tgt:CXX_SCAN_FOR_MODULES
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

# 生成 compile_commands.json 文件
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# 根据构建类型设置优化标志
message(STATUS "Build type = ${CMAKE_BUILD_TYPE}")
#[[set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-Ofast -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -ggdb -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -ggdb -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -ggdb -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g -ggdb -DNDEBUG")]]


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
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wno-sign-compare -Wunused-result -Wno-deprecated-declarations -Wextra -Wno-unused-parameter -Wno-unused-private-field -Wno-macro-redefined -Wno-macro-redefined-pthread -fcolor-diagnostics")
            # 使用 Clang 时，设置使用 lld 链接器
            set(CMAKE_CXX_LINKER "lld-20")
            set(CMAKE_C_LINKER "lld-20")
            set(CMAKE_LINKER "lld-20")
        else ()
            message(FATAL_ERROR "Please use clang version 17.0 and above")
        endif ()

        # 设置 Clang 使用 libc++ 库
        #[[        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
                set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
                set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++")
                set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lc++abi")]]

        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -z noexecstack")
        #set(CMAKE_EXE_LINKER_FLAGS PRIVATE "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc -z noexecstack")

    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # 使用 GNU 时，设置使用 Gold 链接器
        set(CMAKE_CXX_LINKER "ld.gold")
        set(CMAKE_C_LINKER "ld.gold")
        message(STATUS "Using GNU with gold as the linker.")
    else ()
        # 检查是否使用 MSVC 编译器
        if (MSVC)
            # 添加 /bigobj 编译选项
            add_compile_options(/bigobj)
            add_compile_definitions(_WIN32_WINNT=0x0601)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /FORCE:MULTIPLE")
        else ()
            message(WARNING "Unsupported compiler. Default linker will be used.")
        endif()

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
    find_package(Threads REQUIRED)
    # 寻找第三方包

    # doctest与glog冲突
    add_definitions(-DDOCTEST_CONFIG_NO_SHORT_MACRO_NAMES)

    if (UNIX)
        list(APPEND CMAKE_PREFIX_PATH "/mnt/f/runtime/dep_lib/ubuntu24/boost/boost_1_87_0")
        add_definitions(-DBOOST_ASIO_HAS_FILE)
        add_definitions(-DBOOST_ASIO_HAS_IO_URING)
        add_definitions(-DBOOST_ASIO_DISABLE_EPOLL)
        add_definitions(-DBOOST_ASIO_HAS_CO_AWAIT)


        add_definitions(-DASIO_HAS_FILE)
        add_definitions(-DASIO_HAS_IO_URING)
        add_definitions(-DASIO_DISABLE_EPOLL)
    else ()
        list(APPEND CMAKE_PREFIX_PATH "F:/runtime/dep_lib/win/boost_1_87_0")
    endif ()
    #find_package(Boost 1.87.0 REQUIRED COMPONENTS ALL)
    find_package(Boost 1.87.0 REQUIRED)
    # 判断是否找到 Boost
    if (Boost_FOUND)
        message(STATUS "Boost found (version ${Boost_VERSION})")

        # 输出头文件路径
        message(STATUS "Boost include directories: ${Boost_INCLUDE_DIRS}")

        # 输出库文件路径 (所有找到的库)
        message(STATUS "Boost libraries: ${Boost_LIBRARIES}")

        if (Boost_SYSTEM_FOUND)
            message(STATUS "Boost.System found.")
        endif ()
        if (Boost_FILESYSTEM_FOUND)
            message(STATUS "Boost.Filesystem found.")
        endif ()
        if (Boost_CHARCONV_FOUND)
            message(STATUS "Boost.Thread found.")
        endif ()
        if (Boost_THREAD_FOUND)
            message(STATUS "Boost.Thread found.")
        endif ()
        if (Boost_CONTEXT_FOUND)
            message(STATUS "Boost.Thread found.")
        endif ()

        # 将 Boost 头文件包含到项目中
        include_directories(${Boost_INCLUDE_DIRS})

        # 添加可执行文件并链接 Boost 库 (链接所有找到的库)
        #target_link_libraries(my_program ${Boost_LIBRARIES})

    else ()
        message(FATAL_ERROR "Boost not found. Please set BOOST_ROOT or ensure Boost is installed correctly.")
    endif ()
    find_package(nlohmann_json REQUIRED)
    find_package(yaml-cpp CONFIG REQUIRED)
    find_package(glog REQUIRED)
    find_package(SQLite3 REQUIRED)
    find_package(OpenSSL REQUIRED)
    #[[find_package(reflectcpp REQUIRED)]]
    #[[ find_package(fmt)]]

    if (UNIX)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(IO_URING REQUIRED liburing)
    endif ()
endfunction()

function(fn_build)
    if (UNIX)
        #ubuntu 库文件目录usr
        include_directories(/usr/include)
        include_directories(/usr/local/include)

    else ()


    endif ()
    #项目内头文件
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

    # 本项目 内置头文件目录
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/doctest)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/doctest/doctest)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)


    # 头文件
    file(GLOB_RECURSE CURRENT_HEADERS
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp

            # doctest
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/doctest/doctest/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/doctest/doctest/*.hpp
    )

    # 源文件
    file(GLOB_RECURSE SOURCE_FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.ui
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.qrc

            # doctest
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/doctest/doctest/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/doctest/doctest/*.c
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/doctest/doctest/*.cc
    )

    # 使用test文件
    if (USE_TEST)
        #测试文件
        #[[  set(TEST_FILE
                  ${CMAKE_CURRENT_SOURCE_DIR}/run/test_main.cpp

                  ${CMAKE_CURRENT_SOURCE_DIR}/test/std_coroutine.cpp
                  ${CMAKE_CURRENT_SOURCE_DIR}/test/test_utils.cpp
                  ${CMAKE_CURRENT_SOURCE_DIR}/test/test_blog_create.cpp
                  ${CMAKE_CURRENT_SOURCE_DIR}/test/test_reflect.cpp
                  ${CMAKE_CURRENT_SOURCE_DIR}/test/test_cppcoro.cpp
                  ${CMAKE_CURRENT_SOURCE_DIR}/test/test_beast.cpp
                  ${CMAKE_CURRENT_SOURCE_DIR}/test/test_boost_mysql.cpp

          )]]

        file(GLOB TEST_FILE ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)

        file(GLOB_RECURSE TEST_RAFT
                ${CMAKE_CURRENT_SOURCE_DIR}/test/raft/*.h
                ${CMAKE_CURRENT_SOURCE_DIR}/test/raft/*.cc
                ${CMAKE_CURRENT_SOURCE_DIR}/test/raft_test.cpp
        )

        message("test file ${TEST_FILE}")
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
            ${CMAKE_CURRENT_SOURCE_DIR}/run/test_main.cpp
    )

    # 预编译头文件
    if (UES_PRECOMPILE)
        target_precompile_headers(
                ${APP_NAME} PRIVATE
                #[[ <boost/asio.hpp>
                    <nlohmann/json.hpp>
                    <yaml-cpp/yaml.h>
                    <glog/logging.h>
                    <doctest.h>]]
                ${CMAKE_CURRENT_SOURCE_DIR}/src/db/boost_head.h
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

    if (UNIX)
        # 添加库文件搜索路径
        link_directories(/usr/lib/x86_64-linux-gnu)
        target_link_directories(${APP_NAME} PRIVATE /usr/local/lib)

        #libstdc++exp.a 包含了所有实验性功能：
        #以前，一些实验性功能分散在 libstdc++fs.a（文件系统 TS）和 libstdc++_libbacktrace.a（用于 std::stacktrace）等库中。
        #现在，所有这些实验性功能都合并到了 libstdc++exp.a 中
        target_link_libraries(
                ${APP_NAME}
                pthread
                Threads::Threads

                /mnt/f/runtime/dep_lib/ubuntu24/boost/boost_1_87_0/stage/lib/libboost_context.a
                /mnt/f/runtime/dep_lib/ubuntu24/boost/boost_1_87_0/stage/lib/libboost_charconv.a
                /mnt/f/runtime/dep_lib/ubuntu24/boost/boost_1_87_0/stage/lib/libboost_thread.a
                /mnt/f/runtime/dep_lib/ubuntu24/boost/boost_1_87_0/stage/lib/libboost_fiber.a
                /mnt/f/runtime/dep_lib/ubuntu24/boost/boost_1_87_0/stage/lib/libboost_system.a
                /mnt/f/runtime/dep_lib/ubuntu24/boost/boost_1_87_0/stage/lib/libboost_date_time.a
                /mnt/f/runtime/dep_lib/ubuntu24/boost/boost_1_87_0/stage/lib/libboost_timer.a
                /mnt/f/runtime/dep_lib/ubuntu24/boost/boost_1_87_0/stage/lib/libboost_coroutine.a
                /mnt/f/runtime/dep_lib/ubuntu24/boost/boost_1_87_0/stage/lib/libboost_cobalt.a
                /mnt/f/runtime/dep_lib/ubuntu24/boost/boost_1_87_0/stage/lib/libboost_json.a
                /mnt/f/runtime/dep_lib/ubuntu24/boost/boost_1_87_0/stage/lib/libboost_container.a
                /mnt/f/runtime/dep_lib/ubuntu24/boost/boost_1_87_0/stage/lib/libboost_filesystem.a
                /mnt/f/runtime/dep_lib/ubuntu24/boost/boost_1_87_0/stage/lib/libboost_url.a

                nlohmann_json::nlohmann_json
                SQLite::SQLite3
                yaml-cpp::yaml-cpp
                glog::glog
                OpenSSL::Crypto
                OpenSSL::SSL
                #[[ fmt::fmt]]
                #[[ reflectcpp::reflectcpp]]

                stdc++
                stdc++exp
                libquadmath.so #[=[boost charconv 强制使用，无法剔除]=]
                ${IO_URING_LIBRARIES}
        )
    elseif (WIN32)
        target_link_libraries(
                ${APP_NAME}

                nlohmann_json::nlohmann_json
                SQLite::SQLite3
                yaml-cpp::yaml-cpp
                glog::glog
                OpenSSL::Crypto
                OpenSSL::SSL

                F:/runtime/dep_lib/win/boost_1_87_0/stage/lib/boost_context-vc143-mt-gd-x64-1_87.lib
                F:/runtime/dep_lib/win/boost_1_87_0/stage/lib/boost_charconv-vc143-mt-gd-x64-1_87.lib
                F:/runtime/dep_lib/win/boost_1_87_0/stage/lib/boost_thread-vc143-mt-gd-x64-1_87.lib
                F:/runtime/dep_lib/win/boost_1_87_0/stage/lib/boost_fiber-vc143-mt-gd-x64-1_87.lib
                F:/runtime/dep_lib/win/boost_1_87_0/stage/lib/boost_system-vc143-mt-gd-x64-1_87.lib
                F:/runtime/dep_lib/win/boost_1_87_0/stage/lib/boost_date_time-vc143-mt-gd-x64-1_87.lib
                F:/runtime/dep_lib/win/boost_1_87_0/stage/lib/boost_timer-vc143-mt-gd-x64-1_87.lib
                F:/runtime/dep_lib/win/boost_1_87_0/stage/lib/boost_coroutine-vc143-mt-gd-x64-1_87.lib
                F:/runtime/dep_lib/win/boost_1_87_0/stage/lib/boost_cobalt-vc143-mt-gd-x64-1_87.lib
                F:/runtime/dep_lib/win/boost_1_87_0/stage/lib/boost_json-vc143-mt-gd-x64-1_87.lib
                F:/runtime/dep_lib/win/boost_1_87_0/stage/lib/boost_container-vc143-mt-gd-x64-1_87.lib
                F:/runtime/dep_lib/win/boost_1_87_0/stage/lib/boost_filesystem-vc143-mt-gd-x64-1_87.lib
                F:/runtime/dep_lib/win/boost_1_87_0/stage/lib/boost_url-vc143-mt-gd-x64-1_87.lib
        )
    endif ()
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
    note("debug:   cd cmake/debug  conan install ../.. -of . --build missing -s build_type=Debug")
    note("release: cd cmake/debug  conan install ../.. -of . --build missing -s build_type=Release\n")
    note("-of   conan_toolchain.cmake 文件输出目录

    clang版本的编译：
    ./bootstrap.sh --with-toolset=clang

    ./b2 toolset=clang cxxflags="-stdlib=libc++" linkflags="-stdlib=libc++ -lc++abi" --build-type=complete stage threading=multi cxxstd=20 --layout=versioned

    编辑 project-config.jam 文件
    在 project-config.jam 文件中，指定使用 Clang：
    using clang : : clang++-20 : <cxxflags>"-stdlib=libc++" <linkflags>"-stdlib=libc++ -lc++abi" ;

    ./b2 toolset=clang cxxflags="-stdlib=libc++" linkflags="-stdlib=libc++ -lc++abi" --build-type=complete stage threading=multi cxxstd=20 --layout=versioned


/bootstrap.sh --with-libraries=cobalt
./b2 --with-libraries=cobalt link=shared threading=multi cxxstd=20")
    note("*****************************end conan*****************************\n\n\n\n")

endfunction()


fn_modules()
fn_find_package()
fn_build()
fn_optimize()

message("\n\n** Build Summary **")
message("  Prefix:           ${CMAKE_INSTALL_PREFIX}")
message("  Build Type:       ${CMAKE_BUILD_TYPE}")
message("  Architecture:     ${CMAKE_SYSTEM_PROCESSOR}")
message("  System:           ${CMAKE_SYSTEM_NAME}")
message("  Boost Include:    ${Boost_INCLUDE_DIRS}")
message("  Boost Libraries:  ${Boost_LIBRARY_DIRS}")
message("  Zlib Include:     ${ZLIB_INCLUDE_DIRS}")
message("  Zlib Library:     ${ZLIB_LIBRARY_RELEASE}")