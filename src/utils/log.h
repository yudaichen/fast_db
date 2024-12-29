#pragma once
#define GOOGLE_LOG
#ifdef GOOGLE_LOG

#include <glog/logging.h>

#endif

#include <clocale>
#include <ctime>
#include <fstream>
#include <iostream>

// 打印堆栈
#define BOOST_STACKTRACE_USE_ADDR2LINE
#include <boost/stacktrace.hpp>

import small_utils;
import stl;

inline fast::util::OnceToken tokenCreateStackTrace([] {}, nullptr);

#ifdef GOOGLE_LOG
using namespace google;

#define LOG_DEBUG_R LOG(INFO)
#define LOG_INFO_R LOG(INFO)
#define LOG_WARNING_R LOG(WARNING)
#define LOG_ERROR_R LOG(ERROR)
#define LOG_FATAL_R LOG(FATAL)
#define LOG_STACK_R                                                            \
  while (true) {                                                               \
    boost::stacktrace::stacktrace st = boost::stacktrace::stacktrace();        \
    std::ostringstream oss;                                                    \
    for (const auto &frame : st) {                                             \
      oss << "Function: " << frame.name() << '\n';                             \
      oss << "Source File: " << frame.source_file() << '\n';                   \
      oss << "Source Line: " << frame.source_line() << '\n';                   \
      oss << "------------------------\n";                                     \
    }                                                                          \
    LOG(ERROR) << "\n" << oss.str();                                           \
    break;                                                                     \
  }
#define LOG_STACK2_R                                                           \
  while (true) {                                                               \
    std::stacktrace st = std::stacktrace::current();                           \
    std::stringstream ss;                                                      \
    ss << "\nStack trace start:\n";                                            \
    int i = 0;                                                                 \
    for (const auto &frame : st) {                                             \
      ss << std::format("{:4}: {} info \n {}:{}\n\n", i++,                     \
                        frame.description(), frame.source_file(),              \
                        frame.source_line());                                  \
    }                                                                          \
    ss << "Stack trace end...";                                                \
    LOG(ERROR) << ss.str();                                                    \
    break;                                                                     \
  }

#else
#define LOG_DEBUG std::cout
#define LOG_INFO std::cout
#define LOG_WARNING std::cout
#define LOG_ERROR std::cout
#define LOG_FATAL std::cout
#define LOG_STACK std::cout << boost::stacktrace::stacktrace()
#endif

class Log {

public:
  static void instance(const char *slogName = nullptr) {
#ifdef GOOGLE_LOG
    static int g_instance = 0;

    // 只初始化一次
    if (g_instance == 1) {
      return;
    }

    char gLogFileName[256], gLogName[256];
    if (slogName == nullptr) {
      strcpy(gLogName, "unknown");
    } else {
      strncpy(gLogName, slogName, sizeof(gLogName));
    }
    google::InitGoogleLogging(gLogName);

    g_instance        = 1;
    const char *slash = strrchr(gLogName, '/');

#ifdef _WIN32
    if (!slash)
      slash = strrchr(gLogName, '\\');
#endif

    time_t timestamp = time(NULL);
    tm *tm_time      = localtime(&timestamp);

    // 日志文件名定
    sprintf(gLogFileName, "log_[%s]_%d%02d%02d", slash ? slash + 1 : gLogName,
            1900 + tm_time->tm_year, 1 + tm_time->tm_mon, tm_time->tm_mday);

    // 文件输出日志最小级别。
    google::SetLogDestination(google::GLOG_INFO, gLogFileName);

    // 控制台输出日志最小级别。
    google::SetStderrLogging(google::GLOG_INFO);

    // 不同级别设置不同路径
    // google::SetLogDestination(google::GLOG_INFO, "./log_error");
    // google::SetLogDestination(google::GLOG_WARNING, "./log_warning");
    // google::SetLogDestination(google::GLOG_FATAL, "./log_");

    // 强制退出时打印堆栈
    google::InstallFailureSignalHandler(); // 配置安装程序崩溃失败信号处理器

    // 后缀名称
    google::SetLogFilenameExtension(".log");

    // 控制台输出颜色
    FLAGS_colorlogtostderr = true;
#ifdef WIN32
    void (*writer)(const char *data, size_t size) = &SignalHandler;
    google::InstallFailureWriter(
        writer); // 安装配置程序失败信号的信息打印过程，设置回调函数
    FLAGS_colorlogtostdout = true;
#else
    FLAGS_alsologtostderr = true;
#endif
    // 输出文件名包含时间戳（多文件）
    // FLAGS_timestamp_in_logfile_name = false;

    // 磁盘满停止输出日志
    FLAGS_stop_logging_if_full_disk = true;

    // 实时输出日志
    FLAGS_logbufsecs                = 0; // Set log output speed(s)

    // 文件最大输出
    // FLAGS_max_log_size = 1024;  // Set max log file size
#ifdef _WIN32
    std::setlocale(LC_ALL, "zh_CN.UTF-8");
    std::setlocale(LC_NUMERIC, "C");
    std::setlocale(LC_TIME, "zh_CN.UTF-8");
#endif
#endif
  }

  // 扑捉到程序崩溃或者中断时，把相应的信息打印到log文件和输出到屏幕。
  static void SignalHandler(const char *data, size_t size) {
#ifdef GOOGLE_LOG
    std::string glog_file = "./error.log";
    std::ofstream fs(glog_file, std::ios::app);
    std::string str = std::string(data, size);
    fs << str;
    fs.close();
    LOG(INFO) << str;
#endif
  }

  ~Log() {
#ifdef GOOGLE_LOG
    google::ShutdownGoogleLogging();
#endif
  }
};