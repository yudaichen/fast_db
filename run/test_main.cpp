

//#include "gtest/gtest.h"
#include "utils/log.h"
//#include "dbg.h"
#include "redis/redis_asio.hpp"
#include <coroutine>

#include "doctest.h"

import stl;


int main(int argc, char **argv) {
    Log::instance(argv[0]);

    /*redis_asio_connect();
    std::map<int, std::map<std::string, double>> nestedMap{
            {1, {{"a", 1.1}, {"b", 2.2}}},
            {2, {{"c", 3.3}, {"d", 4.4}}},
            {3, {{"e", 5.5}, {"f", 6.6}}}
    };


    dbg(nestedMap);*/
    //Set HTTP listener address and port
    //Load config file
    //drogon::app().loadConfigFile("../config.json");
    //Run HTTP framework,the method will block in the internal event loop
    /*testing::InitGoogleTest(&argc, argv);

    std::cout << "__cplusplus: " << __cplusplus << std::endl;
    std::cout << "__cpp_concepts: " << __cpp_concepts << std::endl;

    int result =  RUN_ALL_TESTS();
    if (result == 0) {
        // 处理内存分配失败的情况
    } else {
        // 使用分配的内存
    }*/

    doctest::Context context;
    context.applyCommandLine(argc, argv);

    int res = context.run(); // run doctest

    // important - query flags (and --exit) rely on the user doing this
    if (context.shouldExit()) {
        // propagate the result of the tests
        return res;
    }


    return 0;
}
