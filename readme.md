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

| 功能类型 | 命名空间 | 命令 | 描述 |
| --- | --- | --- | --- |
| 连接与认证 | `RedisConnection` | `ping` | 测试与 Redis 服务器的连接是否正常 |
|  |  | `auth` | 使用指定密码进行认证 |
| 字符串操作 | `RedisString` | `set` | 设置键值对，支持 `NX`、`XX`、`EX`、`PX` 等选项 |
|  |  | `get` | 获取指定键的值 |
|  |  | `mset` | 同时设置多个键值对 |
|  |  | `mget` | 同时获取多个键的值 |
|  |  | `incr` | 将键存储的数字值加 1 |
|  |  | `decr` | 将键存储的数字值减 1 |
|  |  | `incrby` | 将键存储的数字值增加指定增量 |
|  |  | `decrby` | 将键存储的数字值减少指定减量 |
|  |  | `append` | 将指定值追加到键的现有值末尾 |
|  |  | `strlen` | 获取键对应值的字符串长度 |
| 哈希表操作 | `RedisHash` | `hset` | 设置哈希表中指定字段的值 |
|  |  | `hget` | 获取哈希表中指定字段的值 |
|  |  | `hgetall` | 获取哈希表中所有字段和值 |
|  |  | `hdel` | 删除哈希表中一个或多个指定字段 |
|  |  | `hlen` | 获取哈希表中字段的数量 |
|  |  | `hkeys` | 获取哈希表中所有字段 |
|  |  | `hvals` | 获取哈希表中所有值 |
|  |  | `hmget` | 获取哈希表中一个或多个字段的值 |
| 列表操作 | `RedisList` | `lpush` | 将一个或多个值插入到列表头部 |
|  |  | `rpush` | 将一个或多个值插入到列表尾部 |
|  |  | `lrange` | 获取列表中指定范围的元素 |
|  |  | `llen` | 获取列表的长度 |
|  |  | `lpop` | 移除并返回列表的第一个元素 |
|  |  | `rpop` | 移除并返回列表的最后一个元素 |
| 集合操作 | `RedisSet` | `sadd` | 向集合中添加一个或多个成员 |
|  |  | `smembers` | 获取集合中所有成员 |
|  |  | `sismember` | 判断成员是否存在于集合中 |
|  |  | `srem` | 从集合中移除一个或多个成员 |
|  |  | `sdiff` | 返回给定多个集合的差集 |
|  |  | `sinter` | 返回给定多个集合的交集 |
|  |  | `sunion` | 返回给定多个集合的并集 |
| 有序集合操作 | `RedisSortedSet` | `zadd` | 向有序集合中添加一个或多个成员，并为其关联分数 |
|  |  | `zrange` | 根据索引范围获取有序集合中的成员，可包含分数 |
|  |  | `zrevrange` | 根据索引范围逆序获取有序集合中的成员，可包含分数 |
|  |  | `zcard` | 获取有序集合的成员数量 |
|  |  | `zscore` | 获取有序集合中成员的分数 |
|  |  | `zrem` | 从有序集合中移除一个或多个成员 |
|  |  | `zrangebyscore` | 根据分数范围获取有序集合中的成员，可包含分数，支持偏移量和数量限制 |
|  |  | `zrevrangebyscore` | 根据分数范围逆序获取有序集合中的成员，可包含分数，支持偏移量和数量限制 |
|  |  | `zcount` | 统计有序集合中指定分数范围内的成员数量 |
| 键操作 | `RedisKey` | `del` | 删除一个或多个键 |
|  |  | `exists` | 检查一个或多个键是否存在 |
|  |  | `rename` | 重命名键 |
|  |  | `renamenx` | 仅当新键不存在时，重命名键 |
|  |  | `expire` | 为键设置过期时间（秒） |
|  |  | `persist` | 移除键的过期时间，使键永久存在 |
| 发布订阅 | `RedisPubSub` | `subscribe` | 订阅一个或多个频道 |
|  |  | `unsubscribe` | 取消订阅一个或多个频道 |
|  |  | `publish` | 向指定频道发布消息 |
| 流操作 | `RedisStream` | `xadd` | 向流中添加消息 |
|  |  | `xread` | 从流中读取消息 |
|  |  | `xlen` | 获取流的长度 |
| 经纬度操作（地理空间索引） | `RedisGeo` | `geoadd` | 添加地理位置 |
|  |  | `geodist` | 计算两个地理位置之间的距离 |
|  |  | `georadius` | 根据给定的经纬度和半径，查找指定范围内的地理位置 | 