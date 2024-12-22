#ifndef VALUES_H
#define VALUES_H

import stl;
#include <iostream>
#include <utility>
#include "redis/redis_asio.hpp"

// 基本类型的定义
class Value
{
public:
    virtual             ~Value() = default;
    virtual void        print() const = 0;
    virtual std::string type() const = 0;
};

using ValuePtr = std::shared_ptr<Value>;

// List 类型
class ListValue : public Value
{
public:
    std::list<std::string> items;

    void add(const std::string &item)
    {
        items.push_back(item);
    }

    void remove(const std::string &item)
    {
        items.remove(item);
    }

    void print() const override
    {
        std::cout << "List: ";
        for (const auto &item : items) {
            std::cout << item << " ";
        }
        std::cout << std::endl;
    }

    std::string type() const override { return "List"; }
};

// Set 类型
class SetValue : public Value
{
public:
    std::unordered_set<std::string> items;

    void add(const std::string &item)
    {
        items.insert(item);
    }

    void remove(const std::string &item)
    {
        items.erase(item);
    }

    void print() const override
    {
        std::cout << "Set: ";
        if (items.empty()) {
            std::cout << "(empty)";
        } else {
            for (const auto &item : items) {
                std::cout << item << " ";
            }
        }
        std::cout << std::endl;
    }


    std::string type() const override { return "Set"; }
};

// Map 类型
class MapValue : public Value
{
public:
    std::unordered_map<std::string, std::string> items;

    // 设置哈希表中的键值
    void set(const std::string &key, const std::string &value)
    {
        items[key] = value;
    }

    // 获取哈希表中的键值
    std::string get(const std::string &key) const
    {
        auto it = items.find(key);
        if (it != items.end()) {
            return it->second;
        }
        throw std::out_of_range("Key not found in map: " + key); // 如果键不存在，则抛出异常
    }

    void print() const override
    {
        std::cout << "Map: ";
        for (const auto &[key, value] : items) {
            std::cout << key << ":" << value << " ";
        }
        std::cout << std::endl;
    }

    std::string type() const override { return "Map"; }
};


// Geo 类型
class GeoValue : public Value
{
public:
    struct Point
    {
        double lat, lon;

        // 重载 '<' 操作符，使得可以在 set/map 中排序
        bool operator<(const Point &other) const
        {
            // 比较纬度，如果纬度相同再比较经度
            if (lat != other.lat)
                return lat < other.lat;
            return lon < other.lon;
        }

        // 重载 '==' 操作符，用于 std::unordered_set 中的查找
        bool operator==(const Point &other) const
        {
            return lat == other.lat && lon == other.lon;
        }
    };

    // 为 std::unordered_set 定义哈希函数
    struct PointHasher
    {
        std::size_t operator()(const Point &point) const
        {
            std::size_t lat_hash = std::hash<double>{}(point.lat);
            std::size_t lon_hash = std::hash<double>{}(point.lon);
            return lat_hash ^ (lon_hash << 1); // 将经纬度的哈希值结合起来
        }
    };


    std::unordered_set<Point, PointHasher> points;

    void add(double lat, double lon)
    {
        points.insert({lat, lon});
    }

    void print() const override
    {
        std::cout << "Geo: ";
        for (const auto &point : points) {
            std::cout << "(" << point.lat << ", " << point.lon << ") ";
        }
        std::cout << std::endl;
    }

    std::string type() const override { return "Geo"; }
};

// String 类型
class StringValue : public Value
{
public:
    std::string value;

    StringValue(std::string val)
        : value(std::move(val))
    {
    }

    void print() const override
    {
        std::cout << "String: " << value << std::endl;
    }

    std::string type() const override { return "String"; }
};

// Number 类型
class NumberValue : public Value
{
public:
    double value;

    NumberValue(double val)
        : value(val)
    {
    }

    void print() const override
    {
        std::cout << "Number: " << value << std::endl;
    }

    std::string type() const override { return "Number"; }
};

// 扩展 Slot 类，支持哈希表操作
class Slot
{
public:
    std::unordered_map<std::string, ValuePtr>                              data;
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> expiration_times;

    // 设定键的值
    void set_value(const std::string &key, ValuePtr value)
    {
        data[key] = value;
    }

    // 获取键的值
    ValuePtr get_value(const std::string &key)
    {
        // 检查过期时间
        auto it = expiration_times.find(key);
        if (it != expiration_times.end() && std::chrono::steady_clock::now() > it->second) {
            data.erase(key); // 删除过期的键
            expiration_times.erase(it);
            return nullptr;
        }

        auto value_it = data.find(key);
        return value_it != data.end() ? value_it->second : nullptr;
    }

    // 删除键值对
    void remove(const std::string &key)
    {
        data.erase(key);
        expiration_times.erase(key);
    }

    // 设置过期时间
    void set_expiration(const std::string &key, int ttl_seconds)
    {
        expiration_times[key] = std::chrono::steady_clock::now() + std::chrono::seconds(ttl_seconds);
    }

    // 获取哈希表字段的值
    bool hset(const std::string &key, const std::string &field, const std::string &value)
    {
        auto it = data.find(key);
        if (it != data.end()) {
            // 通过 dynamic_cast 转换为 MapValue 类型
            if (const auto map_value = std::dynamic_pointer_cast<MapValue>(it->second)) {
                map_value->set(field, value); // 设置字段的值
                return true;
            } else {
                std::cerr << "Error: The key does not refer to a MapValue." << std::endl;
            }
        } else {
            std::cerr << "Error: The key does not exist." << std::endl;
        }
        return false;
    }

    std::string hget(const std::string &key, const std::string &field)
    {
        auto it = data.find(key);
        if (it != data.end()) {
            // 通过 dynamic_cast 转换为 MapValue 类型
            const auto map_value = std::dynamic_pointer_cast<MapValue>(it->second);
            if (map_value) {
                const auto element = map_value.get();
                return element->get(field); // 获取字段的值
            } else {
                std::cerr << "Error: The key does not refer to a MapValue." << std::endl;
            }
        } else {
            std::cerr << "Error: The key does not exist." << std::endl;
        }
        return ""; // 如果未找到或类型不匹配，返回空字符串
    }

    void print() const
    {
        for (const auto &[key, value] : data) {
            std::cout << "Key: " << key << ", Type: " << value->type() << " -> ";
            value->print();
        }
    }
};

// RESP 协议解析和封装
class RespProtocol
{
public:
    // 根据 RESP 协议解析客户端请求
    static std::vector<std::string> parse_request(const std::string &data)
    {
        std::vector<std::string> parsed;
        size_t                   pos = 0;
        size_t                   end_pos;

        while ((end_pos = data.find("\r\n", pos)) != std::string::npos) {
            parsed.push_back(data.substr(pos + 1, end_pos - pos - 1)); // 去掉前面的 "+"
            pos = end_pos + 2;
        }

        return parsed;
    }

    // 构造一个响应
    static std::string build_response(const std::string &response)
    {
        return "+" + response + "\r\n"; // RESP 的简化格式
    }
};


// 在 RedisSession 类中扩展命令支持
class RedisSession : public std::enable_shared_from_this<RedisSession>
{
public:
    RedisSession(asio::ip::tcp::socket socket, std::unordered_map<std::string, Slot> &slots)
        : socket(std::move(socket)),
          slots(slots)
    {
        // 注册命令与处理函数的映射
        command_map_["SET"] = [this](const std::vector<std::string> &command) {
            return set_value(command[1], command[2]);
        };
        command_map_["GET"] = [this](const std::vector<std::string> &command) {
            return get_value(command[1]);
        };
        command_map_["DEL"] = [this](const std::vector<std::string> &command) {
            return delete_value(command[1]);
        };
        command_map_["HSET"] = [this](const std::vector<std::string> &command) {
            return hset_value(command[1], command[2], command[3]);
        };
        command_map_["HGET"] = [this](const std::vector<std::string> &command) {
            return hget_value(command[1], command[2]);
        };
        command_map_["EXPIRE"] = [this](const std::vector<std::string> &command) {
            return expire_value(command[1], std::stoi(command[2]));
        };

    }

    void start()
    {
        read_request();
    }

private:
    asio::awaitable<void> read_request()
    {
        while (true) {
            try {
                asio::streambuf buffer;
                size_t          length = co_await asio::async_read_until(socket, buffer, "\r\n", asio::use_awaitable);
                std::string     data(asio::buffer_cast<const char *>(buffer.data()), length);
                buffer.consume(length);

                std::vector<std::string> command = RespProtocol::parse_request(data);
                co_await process_command(command);
            } catch (const std::exception &e) {
                std::cerr << "Error: " << e.what() << std::endl;
                break;
            }
        }
    }

    asio::awaitable<void> process_command(const std::vector<std::string> &command)
    {
        if (command.empty()) {
            co_await send_response("-ERR empty command\r\n");
            co_return;
        }

        const std::string &cmd = command[0];
        if (auto it = command_map_.find(cmd); it != command_map_.end() && command.size() >= 2) {
            co_await it->second(command); // 执行对应的命令处理
        } else {
            co_await send_response("-ERR unknown command\r\n");
        }
    }

    // SET 命令处理
    asio::awaitable<void> set_value(const std::string &key, const std::string &value)
    {
        auto value_ptr = std::make_shared<StringValue>(value);
        slots["default"].set_value(key, value_ptr);
        co_await send_response("+OK\r\n");
    }

    // GET 命令处理
    asio::awaitable<void> get_value(const std::string &key)
    {
        auto value_ptr = slots["default"].get_value(key);
        if (value_ptr) {
            co_await send_response(
                "$" + std::to_string(value_ptr->type().size()) + "\r\n" + value_ptr->type() + "\r\n");
        } else {
            co_await send_response("$-1\r\n");
        }
    }

    // DEL 命令处理
    asio::awaitable<void> delete_value(const std::string &key)
    {
        slots["default"].remove(key);
        co_await send_response(":1\r\n");
    }

    // HSET 命令处理
    asio::awaitable<void> hset_value(const std::string &key, const std::string &field, const std::string &value)
    {
        if (slots["default"].hset(key, field, value)) {
            co_await send_response("+OK\r\n");
        } else {
            co_await send_response("-ERR failed to set hash field\r\n");
        }
    }

    // HGET 命令处理
    asio::awaitable<void> hget_value(const std::string &key, const std::string &field)
    {
        std::string value = slots["default"].hget(key, field);
        if (!value.empty()) {
            co_await send_response("$" + std::to_string(value.size()) + "\r\n" + value + "\r\n");
        } else {
            co_await send_response("$-1\r\n");
        }
    }

    // EXPIRE 命令处理
    asio::awaitable<void> expire_value(const std::string &key, int ttl_seconds)
    {
        slots["default"].set_expiration(key, ttl_seconds);
        co_await send_response(":1\r\n");
    }

    // 发送响应
    asio::awaitable<void> send_response(const std::string &response)
    {
        co_await asio::async_write(socket, asio::buffer(response), asio::use_awaitable);
    }

    asio::ip::tcp::socket                  socket;
    std::unordered_map<std::string, Slot> &slots;

    // 使用 unordered_map 映射命令到处理函数
    std::unordered_map<std::string, std::function<asio::awaitable<void>(const std::vector<std::string> &)>>
    command_map_;

};

// 网络服务器类，负责处理客户端请求
class RedisServer
{
public:
    RedisServer(asio::io_context &io_context, int port)
        : io_context(io_context),
          acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
    {
        start_accept();
    }

    void start_accept()
    {
        acceptor.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
            if (!ec) {
                std::make_shared<RedisSession>(std::move(socket), slots)->start();
            }
            start_accept();
        });
    }

private:
    asio::io_context &                    io_context;
    asio::ip::tcp::acceptor               acceptor;
    std::unordered_map<std::string, Slot> slots; // 各个槽的哈希表
};

#endif //VALUES_H