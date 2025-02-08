# ubuntu24 安装：

## clang安装

https://apt.llvm.org/


编辑 APT 源列表。你可以使用文本编辑器（如 nano 或 vim）来编辑 /etc/apt/sources.list 文件，或者在 /etc/apt/sources.list.d/ 目录中创建一个新的文件。例如，使用 nano 创建一个名为 llvm.list 的文件：

    sudo nano /etc/apt/sources.list.d/llvm.list
添加以下内容到文件中：

    # Noble (24.04) - Last update : Fri, 20 Dec 2024 01:33:14 UTC / Revision: 20241219100923+5f096fd22160
    deb http://apt.llvm.org/noble/ llvm-toolchain-noble main
    deb-src http://apt.llvm.org/noble/ llvm-toolchain-noble main
    # 18
    deb http://apt.llvm.org/noble/ llvm-toolchain-noble-18 main
    deb-src http://apt.llvm.org/noble/ llvm-toolchain-noble-18 main
    # 19
    deb http://apt.llvm.org/noble/ llvm-toolchain-noble-19 main
    deb-src http://apt.llvm.org/noble/ llvm-toolchain-noble-19 main
    保存并退出编辑器。对于 nano，你可以按 CTRL + O 保存，然后按 CTRL + X 退出。

更新 APT 包索引：

    sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 15CF4D18AF4F7421
    sudo apt update
    sudo apt install clang-19 libclang-common-19-dev

## gcc-14 g++-14
    
    apt install gcc-14 g++-14
    sudo update-alternatives —install /usr/bin/gcc gcc /usr/bin/gcc-14 14
    sudo update-alternatives —install /usr/bin/g++ g++ /usr/bin/g++-14 14


## [redis_asio.hpp](src/redis/redis_asio.hpp)
已经实现的客户端命令

| 功能类型 | 命令 | 描述 |
| --- | --- | --- |
| **连接与认证** | `PING` | 测试与 Redis 服务器的连接是否正常 |
|  | `AUTH` | 使用指定密码进行认证 |
| **字符串操作** | `SET` | 设置键值对，支持 `NX`、`XX`、`EX`、`PX` 等选项 |
|  | `GET` | 获取指定键的值 |
|  | `MSET` | 同时设置多个键值对 |
|  | `MGET` | 同时获取多个键的值 |
|  | `INCR` | 将键存储的数字值加 1 |
|  | `DECR` | 将键存储的数字值减 1 |
|  | `INCRBY` | 将键存储的数字值增加指定增量 |
|  | `DECRBY` | 将键存储的数字值减少指定减量 |
|  | `APPEND` | 将指定值追加到键的现有值末尾 |
|  | `STRLEN` | 获取键对应值的字符串长度 |
| **哈希表操作** | `HSET` | 设置哈希表中指定字段的值 |
|  | `HGET` | 获取哈希表中指定字段的值 |
|  | `HGETALL` | 获取哈希表中所有字段和值 |
|  | `HDEL` | 删除哈希表中一个或多个指定字段 |
|  | `HLEN` | 获取哈希表中字段的数量 |
|  | `HKEYS` | 获取哈希表中所有字段 |
|  | `HVALS` | 获取哈希表中所有值 |
|  | `HMGET` | 获取哈希表中一个或多个字段的值 |
| **列表操作** | `LPUSH` | 将一个或多个值插入到列表头部 |
|  | `RPUSH` | 将一个或多个值插入到列表尾部 |
|  | `LRANGE` | 获取列表中指定范围的元素 |
|  | `LLEN` | 获取列表的长度 |
|  | `LPOP` | 移除并返回列表的第一个元素 |
|  | `RPOP` | 移除并返回列表的最后一个元素 |
| **集合操作** | `SADD` | 向集合中添加一个或多个成员 |
|  | `SMEMBERS` | 获取集合中所有成员 |
|  | `SISMEMBER` | 判断成员是否存在于集合中 |
|  | `SREM` | 从集合中移除一个或多个成员 |
|  | `SDIFF` | 返回给定多个集合的差集 |
|  | `SINTER` | 返回给定多个集合的交集 |
|  | `SUNION` | 返回给定多个集合的并集 |
| **有序集合操作** | `ZADD` | 向有序集合中添加一个或多个成员，并为其关联分数 |
|  | `ZRANGE` | 根据索引范围获取有序集合中的成员，可包含分数 |
|  | `ZREVRANGE` | 根据索引范围逆序获取有序集合中的成员，可包含分数 |
|  | `ZCARD` | 获取有序集合的成员数量 |
|  | `ZSCORE` | 获取有序集合中成员的分数 |
|  | `ZREM` | 从有序集合中移除一个或多个成员 |
|  | `ZRANGEBYSCORE` | 根据分数范围获取有序集合中的成员，可包含分数，支持偏移量和数量限制 |
|  | `ZREVRANGEBYSCORE` | 根据分数范围逆序获取有序集合中的成员，可包含分数，支持偏移量和数量限制 |
| **键操作** | `DEL` | 删除一个或多个键 |
|  | `EXISTS` | 检查一个或多个键是否存在 |
|  | `RENAME` | 重命名键 |
|  | `RENAMENX` | 仅当新键不存在时，重命名键 |
|  | `EXPIRE` | 为键设置过期时间（秒） | 
