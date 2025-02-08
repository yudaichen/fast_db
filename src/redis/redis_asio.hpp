/*#define BOOST_ASIO_HAS_CO_AWAIT*/
#pragma once
#include <filesystem>
#include <thread>

#include <coroutine>
// #include "asio.hpp"
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
//#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/write.hpp>
#include <iostream>

#include "utils/log.h"
#include <map>
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
using boost::asio::ip::tcp;
namespace this_coro = boost::asio::this_coro;

#if defined(BOOST_ASIO_ENABLE_HANDLER_TRACKING)
#define use_awaitable                                                          \
  boost::asio::use_awaitable_t(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif

using namespace boost;
using namespace boost::asio;

asio::awaitable<void> simplePingClient() {
  try {
    tcp::socket socket(co_await asio::this_coro::executor);

    co_await socket.async_connect(
        tcp::endpoint(asio::ip::make_address("101.43.58.72"), 6379),
        asio::use_awaitable);

    std::cout << "Connected to Redis server!" << std::endl;

    {
      std::string command_auth = "*2\r\n$4\r\nAUTH\r\n$9\r\nydc061588\r\n";
      co_await asio::async_write(socket, asio::buffer(command_auth),
                                 asio::use_awaitable);

      char reply_auth[1024];
      const std::size_t bytes_transferred_auth =
          co_await socket.async_read_some(asio::buffer(reply_auth),
                                          asio::use_awaitable);
      std::cout << "Response from Redis: "
                << std::string(reply_auth, bytes_transferred_auth) << std::endl;
    }

    {
      // 发送命令到 Redis 服务器
      std::string command = "PING\r\n";
      co_await asio::async_write(socket, asio::buffer(command),
                                 asio::use_awaitable);

      // 读取 Redis 服务器的响应
      char reply_ping[1024];
      const std::size_t bytes_transferred_pring =
          co_await socket.async_read_some(asio::buffer(reply_ping),
                                          asio::use_awaitable);

      LOG_INFO_R << "Response from Redis: "
                 << std::string(reply_ping, bytes_transferred_pring)
                 << std::endl;
    }

    // 关闭连接
    socket.close();
    std::cout << "Connection closed." << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Connection error: " << e.what() << std::endl;
  }
}

/*
asio::awaitable<void> syn_file(string_view path)
{

    std::promise<int> prom;
    auto fut = prom.get_future();
    auto fd = co_await boost::asio::open(path, O_RDONLY);
    prom.set_value(fd);
}
*/
void redis_asio_connect() {
  {
    try {
      boost::asio::io_context io_context(1);

      boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
      signals.async_wait([&](auto, auto) { io_context.stop(); });

      // 执行协程，并设置了一个callback函数来获取这个协程的返回值
      co_spawn(io_context, simplePingClient(), detached);

      io_context.run();
    } catch (std::exception &e) {
      std::printf("Exception: %s\n", e.what());
    }
  }
}

// 定义 SET 命令的可选参数结构体
struct SetOptions {
    bool nx = false;
    bool xx = false;
    int ex = -1;
    int px = -1;
};

// 生成 Redis 协议的数组格式
std::string generate_command(const std::string& command, const std::vector<std::string>& parts, const SetOptions& options = {}) {
    std::vector<std::string> all_parts = {command};
    all_parts.insert(all_parts.end(), parts.begin(), parts.end());

    if (options.nx) all_parts.push_back("NX");
    if (options.xx) all_parts.push_back("XX");
    if (options.ex != -1) {
        all_parts.push_back("EX");
        all_parts.push_back(std::to_string(options.ex));
    }
    if (options.px != -1) {
        all_parts.push_back("PX");
        all_parts.push_back(std::to_string(options.px));
    }

    std::ostringstream oss;
    oss << "*" << all_parts.size() << "\r\n";  // 数组长度
    for (const auto& part : all_parts) {
        oss << "$" << part.size() << "\r\n";  // 字符串长度
        oss << part << "\r\n";  // 字符串内容
    }
    return oss.str();
}

namespace RedisConnection {
    // PING 命令
    std::string ping() {
        return generate_command("PING", {});
    }

    // AUTH 命令
    std::string auth(const std::string& password) {
        return generate_command("AUTH", {password});
    }
}

namespace RedisString {
    // SET 命令
    std::string set(const std::string& key, const std::string& value, const SetOptions& options = {}) {
        return generate_command("SET", {key, value}, options);
    }

    // GET 命令
    std::string get(const std::string& key) {
        return generate_command("GET", {key});
    }

    // MSET 命令
    std::string mset(const std::map<std::string, std::string>& key_value_map) {
        std::vector<std::string> parts;
        for (const auto& [key, value] : key_value_map) {
            parts.push_back(key);
            parts.push_back(value);
        }
        return generate_command("MSET", parts);
    }

    // MGET 命令
    std::string mget(const std::vector<std::string>& keys) {
        return generate_command("MGET", keys);
    }

    // INCR 命令
    std::string incr(const std::string& key) {
        return generate_command("INCR", {key});
    }

    // DECR 命令
    std::string decr(const std::string& key) {
        return generate_command("DECR", {key});
    }

    // INCRBY 命令
    std::string incrby(const std::string& key, int increment) {
        return generate_command("INCRBY", {key, std::to_string(increment)});
    }

    // DECRBY 命令
    std::string decrby(const std::string& key, int decrement) {
        return generate_command("DECRBY", {key, std::to_string(decrement)});
    }

    // APPEND 命令
    std::string append(const std::string& key, const std::string& value) {
        return generate_command("APPEND", {key, value});
    }

    // STRLEN 命令
    std::string strlen(const std::string& key) {
        return generate_command("STRLEN", {key});
    }
}

namespace RedisHash {
    // HSET 命令（哈希表）
    std::string hset(const std::string& key, const std::map<std::string, std::string> & field_value_map) {
        std::vector<std::string> parts = {key};
        for (const auto& [field, value] : field_value_map) {
            parts.push_back(field);
            parts.push_back(value);
        }
        return generate_command("HSET", parts);
    }

    // HGET 命令（哈希表）
    std::string hget(const std::string& key, const std::string& field) {
        return generate_command("HGET", {key, field});
    }

    // HGETALL 命令（哈希表）
    std::string hgetall(const std::string& key) {
        return generate_command("HGETALL", {key});
    }

    // HDEL 命令（哈希表）
    std::string hdel(const std::string& key, const std::vector<std::string>& fields) {
        std::vector<std::string> parts = {key};
        parts.insert(parts.end(), fields.begin(), fields.end());
        return generate_command("HDEL", parts);
    }

    // HLEN 命令（哈希表）
    std::string hlen(const std::string& key) {
        return generate_command("HLEN", {key});
    }

    // HKEYS 命令（哈希表）
    std::string hkeys(const std::string& key) {
        return generate_command("HKEYS", {key});
    }

    // HVALS 命令（哈希表）
    std::string hvals(const std::string& key) {
        return generate_command("HVALS", {key});
    }

    // HMGET 命令
    std::string hmget(const std::string& key, const std::vector<std::string>& fields) {
        std::vector<std::string> parts = {key};
        parts.insert(parts.end(), fields.begin(), fields.end());
        return generate_command("HMGET", parts);
    }
}

namespace RedisList {
    // LPUSH 命令（列表）
    std::string lpush(const std::string& key, const std::vector<std::string>& values) {
        std::vector<std::string> parts = {key};
        parts.insert(parts.end(), values.begin(), values.end());
        return generate_command("LPUSH", parts);
    }

    // RPUSH 命令（列表）
    std::string rpush(const std::string& key, const std::vector<std::string>& values) {
        std::vector<std::string> parts = {key};
        parts.insert(parts.end(), values.begin(), values.end());
        return generate_command("RPUSH", parts);
    }

    // LRANGE 命令（列表）
    std::string lrange(const std::string& key, int start, int stop) {
        return generate_command("LRANGE", {key, std::to_string(start), std::to_string(stop)});
    }

    // LLEN 命令（列表）
    std::string llen(const std::string& key) {
        return generate_command("LLEN", {key});
    }

    // LPOP 命令（列表）
    std::string lpop(const std::string& key) {
        return generate_command("LPOP", {key});
    }

    // RPOP 命令（列表）
    std::string rpop(const std::string& key) {
        return generate_command("RPOP", {key});
    }
}

namespace RedisSet {
    // SADD 命令（集合）
    std::string sadd(const std::string& key, const std::vector<std::string>& members) {
        std::vector<std::string> parts = {key};
        parts.insert(parts.end(), members.begin(), members.end());
        return generate_command("SADD", parts);
    }

    // SMEMBERS 命令（集合）
    std::string smembers(const std::string& key) {
        return generate_command("SMEMBERS", {key});
    }

    // SISMEMBER 命令（集合）
    std::string sismember(const std::string& key, const std::string& member) {
        return generate_command("SISMEMBER", {key, member});
    }

    // SREM 命令（集合）
    std::string srem(const std::string& key, const std::vector<std::string>& members) {
        std::vector<std::string> parts = {key};
        parts.insert(parts.end(), members.begin(), members.end());
        return generate_command("SREM", parts);
    }

    // SDIFF 命令（集合）
    std::string sdiff(const std::vector<std::string>& keys) {
        return generate_command("SDIFF", keys);
    }

    // SINTER 命令（集合）
    std::string sinter(const std::vector<std::string>& keys) {
        return generate_command("SINTER", keys);
    }

    // SUNION 命令（集合）
    std::string sunion(const std::vector<std::string>& keys) {
        return generate_command("SUNION", keys);
    }
}

namespace RedisSortedSet {
    // ZADD 命令（有序集合）
    std::string zadd(const std::string& key, const std::map<std::string, double>& score_member_map) {
        std::vector<std::string> parts = {key};
        for (const auto& [member, score] : score_member_map) {
            parts.push_back(std::to_string(score));
            parts.push_back(member);
        }
        return generate_command("ZADD", parts);
    }

    // ZRANGE 命令（有序集合）
    std::string zrange(const std::string& key, int start, int stop, bool with_scores = false) {
        std::vector<std::string> parts = {key, std::to_string(start), std::to_string(stop)};
        if (with_scores) {
            parts.push_back("WITHSCORES");
        }
        return generate_command("ZRANGE", parts);
    }

    // ZREVRANGE 命令（有序集合）
    std::string zrevrange(const std::string& key, int start, int stop, bool with_scores = false) {
        std::vector<std::string> parts = {key, std::to_string(start), std::to_string(stop)};
        if (with_scores) {
            parts.push_back("WITHSCORES");
        }
        return generate_command("ZREVRANGE", parts);
    }

    // ZCARD 命令（有序集合）
    std::string zcard(const std::string& key) {
        return generate_command("ZCARD", {key});
    }

    // ZSCORE 命令（有序集合）
    std::string zscore(const std::string& key, const std::string& member) {
        return generate_command("ZSCORE", {key, member});
    }

    // ZREM 命令（有序集合）
    std::string zrem(const std::string& key, const std::vector<std::string>& members) {
        std::vector<std::string> parts = {key};
        parts.insert(parts.end(), members.begin(), members.end());
        return generate_command("ZREM", parts);
    }

    // ZRANGEBYSCORE 命令（有序集合）
    std::string zrangebyscore(const std::string& key, const std::string& min, const std::string& max, bool with_scores = false, int offset = -1, int count = -1) {
        std::vector<std::string> parts = {key, min, max};
        if (with_scores) {
            parts.push_back("WITHSCORES");
        }
        if (offset != -1 && count != -1) {
            parts.push_back("LIMIT");
            parts.push_back(std::to_string(offset));
            parts.push_back(std::to_string(count));
        }
        return generate_command("ZRANGEBYSCORE", parts);
    }

    // ZREVRANGEBYSCORE 命令（有序集合）
    std::string zrevrangebyscore(const std::string& key, const std::string& max, const std::string& min, bool with_scores = false, int offset = -1, int count = -1) {
        std::vector<std::string> parts = {key, max, min};
        if (with_scores) {
            parts.push_back("WITHSCORES");
        }
        if (offset != -1 && count != -1) {
            parts.push_back("LIMIT");
            parts.push_back(std::to_string(offset));
            parts.push_back(std::to_string(count));
        }
        return generate_command("ZREVRANGEBYSCORE", parts);
    }

    // ZCOUNT 命令（有序集合）
    std::string zcount(const std::string& key, const std::string& min, const std::string& max) {
        return generate_command("ZCOUNT", {key, min, max});
    }
}

namespace RedisKey {
    // DEL 命令
    std::string del(const std::vector<std::string>& keys) {
        return generate_command("DEL", keys);
    }

    // EXISTS 命令
    std::string exists(const std::vector<std::string>& keys) {
        return generate_command("EXISTS", keys);
    }

    // RENAME 命令
    std::string rename(const std::string& old_key, const std::string& new_key) {
        return generate_command("RENAME", {old_key, new_key});
    }

    // RENAMENX 命令
    std::string renamenx(const std::string& old_key, const std::string& new_key) {
        return generate_command("RENAMENX", {old_key, new_key});
    }

    // EXPIRE 命令
    std::string expire(const std::string& key, int seconds) {
        return generate_command("EXPIRE", {key, std::to_string(seconds)});
    }

    // PERSIST 命令
    std::string persist(const std::string& key) {
        return generate_command("PERSIST", {key});
    }
}