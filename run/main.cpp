#include <iostream>
#include "redis/redis_asio.hpp"
#include <dbg.h>
#include "utils/log.h"

int main(int argc, char **argv) {
    std::string name = "conan_test";
    Log::instance(argv[0]);
    std::string c = "1,2,3,4,5,6,7";

    std::string message = "hello";
    dbg(message);
    LOG_INFO << message;
    //redis_asio_connect();

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
