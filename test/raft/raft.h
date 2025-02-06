#pragma once

#include "objfactory.h"
#include "thread_pool.h"

#include <boost/functional/hash.hpp>
#include <boost/serialization/access.hpp>

namespace raft {
struct Log
{
    int         index     = -1;    // 日志记录的索引
    int         term      = 0;     // 日志记录的任期
    bool        is_server = false; // 是否是服务器自己的日志
    std::string content;           // 内容
};

enum State
{
    None = 0,
    Leader = 1,    // 领导
    Candidate = 2, // 候选人
    Folower = 3,   // 跟随者
};

class server : public noncopyable
{
public:
    [[nodiscard]] std::shared_ptr<objfactory<server>> get_m_factory() const { return m_factory; }
    void set_m_factory(const std::shared_ptr<objfactory<server>> &m_factory) { this->m_factory = m_factory; }
    [[nodiscard]] int get_m_id() const { return m_id; }
    void set_m_id(const int m_id) { this->m_id = m_id; }
    [[nodiscard]] bool is_m_is_stop() const { return m_is_stop; }
    void set_m_is_stop(const bool m_is_stop) { this->m_is_stop = m_is_stop; }
    [[nodiscard]] int get_m_heartbeat() const { return m_heartbeat; }
    void set_m_heartbeat(const int m_heartbeat) { this->m_heartbeat = m_heartbeat; }
    [[nodiscard]] int get_m_vote_count() const { return m_vote_count; }
    void set_m_vote_count(const int m_vote_count) { this->m_vote_count = m_vote_count; }
    [[nodiscard]] State get_m_state() const { return m_state; }
    void set_m_state(const State m_state) { this->m_state = m_state; }
    [[nodiscard]] int get_m_term() const { return m_term; }
    void set_m_term(const int m_term) { this->m_term = m_term; }
    [[nodiscard]] int get_m_votedfor() const { return m_votedfor; }
    void set_m_votedfor(const int m_votedfor) { this->m_votedfor = m_votedfor; }
    [[nodiscard]] std::vector<Log> get_m_log_vec() const { return m_log_vec; }
    void set_m_log_vec(const std::vector<Log> &m_log_vec) { this->m_log_vec = m_log_vec; }
    [[nodiscard]] int get_m_commit_index() const { return m_commit_index; }
    void set_m_commit_index(const int m_commit_index) { this->m_commit_index = m_commit_index; }
    [[nodiscard]] int get_m_last_applied() const { return m_last_applied; }
    void set_m_last_applied(const int m_last_applied) { this->m_last_applied = m_last_applied; }
    [[nodiscard]] std::vector<int> get_m_next_index_vec() const { return m_next_index_vec; }

    void set_m_next_index_vec(const std::vector<int> &m_next_index_vec)
    {
        this->m_next_index_vec = m_next_index_vec;
    }

    [[nodiscard]] std::vector<int> get_m_match_index_vec() const { return m_match_index_vec; }

    void set_m_match_index_vec(const std::vector<int> &m_match_index_vec)
    {
        this->m_match_index_vec = m_match_index_vec;
    }

    [[nodiscard]] std::map<long long, std::queue<std::string>> get_m_print_map() const { return m_print_map; }

    void set_m_print_map(const std::map<long long, std::queue<std::string>> &m_print_map)
    {
        this->m_print_map = m_print_map;
    }

    friend void swap(server &lhs, server &rhs)
    {
        using std::swap;
        swap(lhs.m_factory, rhs.m_factory);
        swap(lhs.m_id, rhs.m_id);
        swap(lhs.m_is_stop, rhs.m_is_stop);
        swap(lhs.m_heartbeat, rhs.m_heartbeat);
        swap(lhs.m_vote_count, rhs.m_vote_count);
        swap(lhs.m_state, rhs.m_state);
        swap(lhs.m_term, rhs.m_term);
        swap(lhs.m_votedfor, rhs.m_votedfor);
        swap(lhs.m_log_vec, rhs.m_log_vec);
        swap(lhs.m_commit_index, rhs.m_commit_index);
        swap(lhs.m_last_applied, rhs.m_last_applied);
        swap(lhs.m_next_index_vec, rhs.m_next_index_vec);
        swap(lhs.m_match_index_vec, rhs.m_match_index_vec);
        swap(lhs.m_print_map, rhs.m_print_map);
    }


    [[nodiscard]] server(std::shared_ptr<objfactory<server>> m_factory, int m_id, bool m_is_stop, int m_heartbeat,
        int m_vote_count, State m_state, int m_term, int m_votedfor, std::vector<Log> m_log_vec, int m_commit_index,
        int m_last_applied, std::vector<int> m_next_index_vec, std::vector<int> m_match_index_vec,
        std::map<long long, std::queue<std::string>> m_print_map)
        : m_factory(std::move(m_factory)),
          m_id(m_id),
          m_is_stop(m_is_stop),
          m_heartbeat(m_heartbeat),
          m_vote_count(m_vote_count),
          m_state(m_state),
          m_term(m_term),
          m_votedfor(m_votedfor),
          m_log_vec(std::move(m_log_vec)),
          m_commit_index(m_commit_index),
          m_last_applied(m_last_applied),
          m_next_index_vec(std::move(m_next_index_vec)),
          m_match_index_vec(std::move(m_match_index_vec)),
          m_print_map(std::move(m_print_map))
    {
    }

private:
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive &ar, const unsigned version)
    {
        ar & m_factory;
        ar & m_id;
        ar & m_is_stop;
        ar & m_heartbeat;
        ar & m_vote_count;
        ar & m_state;
        ar & m_term;
        ar & m_votedfor;
        ar & m_log_vec;
        ar & m_commit_index;
        ar & m_last_applied;
        ar & m_next_index_vec;
        ar & m_match_index_vec;
        ar & m_print_map;
    }

    std::mutex                          m_mutex;
    std::shared_ptr<objfactory<server>> m_factory;

    int  m_id         = 0;    // server_id
    bool m_is_stop    = true; // 停服
    int  m_heartbeat  = 0;    // 心跳超时计数
    int  m_vote_count = 0;    // 拥有的投票数

    // 需要持久化的数据
    State            m_state    = State::None; // 状态
    int              m_term     = 0;           // 任期
    int              m_votedfor = 0;           // 给谁投票
    std::vector<Log> m_log_vec;                // 日志

    // 临时数据
    int m_commit_index = 0; // 自己的提交进度索引
    int m_last_applied = 0; // 自己的保存进度索引

    // 只属于leader的临时数据
    std::vector<int> m_next_index_vec;  // 所有serve将要同步的进度索引
    std::vector<int> m_match_index_vec; // 所有server已经同步的进度索引

public:
    server() = delete;
    server(int id, std::shared_ptr<objfactory<server>> factory);
    ~server() = default;

    int                     key() const { return m_id; }
    bool                    IsLeader() const { return m_state == State::Leader; }
    int                     Term() const { return m_term; }
    State                   GetState() const { return m_state; }
    bool                    IsStop() const { return m_is_stop; }
    const std::vector<Log> &LogVec() const { return m_log_vec; }
    const std::vector<Log>  ApplyLogVec();

    int AddLog(const std::string &str); // 添加日志

    void Start();
    void Stop();
    void ReStart();

private:
    void Update();   // 定时器
    void Election(); // 选举

    // 请求投票
    struct VoteArgs // 参数
    {
        int term           = 0;  // 候选人的任期
        int candidate_id   = 0;  // 候选人的id
        int last_log_index = -1; // 候选人最新log的index
        int last_log_term  = 0;  // 候选人最新log的任期
    };

    struct VoteReply
    {
        int  term         = 0;     // 返回的任期
        bool vote_granted = false; // 是否投票
    };

    void RequestVote(const VoteArgs &args);
    void ReplyVote(const VoteReply &reply);

    // 追加条目（可作心跳）
    struct AppendEntriesArgs
    {
        int              term          = 0; // 领导的任期
        int              leader_id     = 0; // 领导的id
        int              pre_log_index = 0; // 跟随者的同步进度索引
        int              pre_log_term  = 0; // 跟随者的同步进度任期
        int              commit_index  = 0; // 领导的最新提交索引
        std::vector<Log> log_vec;           // 要同步的日志
    };

    struct AppendEntriesReply
    {
        int  id           = 0;     // 返回的id
        int  term         = 0;     // 返回的任期
        int  log_count    = 0;     // 要同步的日志数量
        bool success      = false; // 是否同步成功
        int  commit_index = 0;     // 返回的最新提交索引
    };

    void RequestAppendEntries(const AppendEntriesArgs &args);
    void ReplyAppendEntries(const AppendEntriesReply &reply);

    void ToLeader();
    void ToFollower(int term, int votedfor);

    // Print
private:
    std::map<long long, std::queue<std::string>> m_print_map;

public:
    void Print();
    void AddPrint(const std::string &str);
    void PrintAllLog();
    void PrintAllApplyLog();
};

}