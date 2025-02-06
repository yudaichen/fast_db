module;

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>
//import stl;
export module string_util;

export namespace fast::util {
class StringUtil {
public:
  /// @brief 是否是整型数据
  static bool isInteger(const std::string &str) {
    for (std::size_t i = 0; i < str.length(); i++) {
      if (str[i] > '9' || str[i] < '0') {
        return false;
      }
    }
    return true;
  }

  /// @brief 转成小写字母
  static std::string toLower(std::string &str) {
    for (std::string::size_type i = 0; i < str.length(); ++i)
      if (str[i] >= 'A' && str[i] <= 'Z') {
        str[i] += 0x20;
      }
    return str;
  }

  /// @brief 转成大写字母
  static std::string toUpper(std::string &str) {
    for (std::string::size_type i = 0; i < str.length(); ++i)
      if (str[i] >= 'a' && str[i] <= 'z') {
        str[i] -= 0x20;
      }
    return str;
  }

  /// @brief 字符是不是字母
  static bool charIsLetter(char c) {
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
      return true;
    }
    return false;
  }

  /**
   * @brief 向左修剪
   * @param str 源字符串
   * @param token 条件字符串
   * @return 裁剪后的字符串
   */
  static std::string TrimLeft(const std::string &str,
                              const std::string &token = " ") {
    std::string t = str;
    t.erase(0, t.find_first_not_of(token));
    return t;
  }

  /**
   * @brief 向右修剪
   * @param str 源字符串
   * @param token 条件字符串
   * @return 裁剪后的字符串
   */
  static std::string TrimRight(const std::string &str,
                               const std::string &token = " ") {
    std::string t = str;
    t.erase(t.find_last_not_of(token) + 1);
    return t;
  }

  /**
   * @brief 根据字符同时 左右修剪，默认整理空格
   * @param str 源字符串
   * @param token 条件字符串
   * @return 裁剪后的字符串
   */
  static std::string Trim(const std::string &str,
                          const std::string &token = " ") {
    std::string t = str;
    t.erase(0, t.find_first_not_of(token));
    t.erase(t.find_last_not_of(token) + 1);
    return t;
  }

  /**
   * @brief 转大写
   * @param str 源字符串
   * @return 转换后的字符串
   */
  static std::string ToLower(const std::string &str) {
    std::string t = str;
    std::transform(t.begin(), t.end(), t.begin(), ::tolower);
    return t;
  }

  /**
   * @brief 转小写
   * @param str 源字符串
   * @return 转换后的字符串
   */
  static std::string ToUpper(const std::string &str) {
    std::string t = str;
    std::transform(t.begin(), t.end(), t.begin(), ::toupper);
    return t;
  }

  /**
   * @brief 当前的字符是否以字串开头
   * @param str 源字符串
   * @param subStr 子串
   * @return bool
   */
  static bool StartsWith(const std::string &str, const std::string &subStr) {
    return str.find(subStr) == 0;
  }

  /**
   * @brief 当前的字符是以字串结尾
   * @param str 源字符串
   * @param subStr 字串
   * @return bool
   */
  static bool EndsWith(const std::string &str, const std::string &substr) {
    return str.rfind(substr) == (str.length() - substr.length());
  }

  /**
   * @brief 忽略大小写匹配是否相等
   * @param str1 比较1
   * @param str2 比较2
   * @return bool
   */
  static bool EqualsIgnoreCaseSample(const std::string &str1,
                                     const std::string &str2) {
    return ToLower(str1) == ToLower(str2);
  }

  /// @brief 两字符串是否相等，忽略大小写
  static bool EqualsIgnoreCase(const std::string &s1, const std::string &s2) {
    if (s1.length() != s2.length()) {
      return false;
    }

    for (std::string::size_type i = 0; i < s1.length(); ++i) {
      if (s1[i] == s2[i]) {
        continue;
      }
      if (!charIsLetter(s1[i]) || !charIsLetter(s2[i])) {
        return false;
      }
      if (0x20 != abs(s1[i] - s2[i])) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief 分割字符串
   * @param str 源字串
   * @param delimiter 分隔符
   * @return std::vector
   */
  static std::vector<std::string> Split(const std::string &str,
                                        const std::string &delimiter) {
    char *save = nullptr;
#ifdef _WIN32
    char *token =
        strtok_s(const_cast<char *>(str.c_str()), delimiter.c_str(), &save);
#else
    char *token =
        strtok_r(const_cast<char *>(str.c_str()), delimiter.c_str(), &save);
#endif
    std::vector<std::string> result;
    while (token != nullptr) {
      result.emplace_back(token);
#ifdef _WIN32
      token = strtok_s(nullptr, delimiter.c_str(), &save);
#else
      token = strtok_r(nullptr, delimiter.c_str(), &save);
#endif
    }
    return result;
  }

  static void Strtok(const std::string &str, char separator,
                     std::vector<std::string> &arr) {
    std::size_t pos    = 0;
    std::size_t newPos = 0;

    while (std::string::npos != pos) {
      pos = str.find_first_of(separator, newPos);
      if (std::string::npos == pos) { // 结束了
        if (pos > newPos) {
          arr.push_back(str.substr(newPos, pos - newPos));
        }
        break;
      } else {
        if (pos > newPos) {
          arr.push_back(str.substr(newPos, pos - newPos));
        }
        newPos = pos + 1;
      }
    }
  }

  static std::vector<std::string> Strtok(const std::string &str,
                                         char separator) {
    std::size_t pos    = 0;
    std::size_t newPos = 0;
    std::vector<std::string> arr;

    while (std::string::npos != pos) {
      pos = str.find_first_of(separator, newPos);
      if (std::string::npos == pos) { // 结束了
        if (pos > newPos) {
          arr.push_back(str.substr(newPos, pos - newPos));
        }
        break;
      } else {
        if (pos > newPos) {
          arr.push_back(str.substr(newPos, pos - newPos));
        }
        newPos = pos + 1;
      }
    }
    return arr;
  }

  /**
   * @brief 字串是否包含在源串中
   * @param str 源串
   * @param token 查找串
   * @return bool
   */
  static bool Contains(const std::string &str, const std::string &token) {
    return str.find(token) != std::string::npos;
  }

  // 类型转换 简化调用
  template <typename To, typename From> static To cast(const From &from) {
    return converter<To, From>::convert(from);
  }

private:
  // 模板特化定义
  template <typename To, typename From> struct converter {};

  // 转换到int类型
  template <typename From> struct converter<int, From> {
    static int convert(const std::string &from) {
      return std::atoi(from.c_str());
    }

    static int convert(const char *from) { return std::atoi(from); }
  };

  // 转换到long类型
  template <typename From> struct converter<long, From> {
    static long convert(const std::string &from) {
      return std::atol(from.c_str());
    }

    static long convert(const char *from) { return std::atol(from); }
  };

  // 转换到long long类型
  template <typename From> struct converter<long long, From> {
    static long long convert(const std::string &from) {
      return std::atoll(from.c_str());
    }

    static long long convert(const char *from) { return std::atoll(from); }
  };

  // 转换到double类型
  template <typename From> struct converter<double, From> {
    static double convert(const std::string &from) {
      return std::atof(from.c_str());
    }

    static double convert(const char *from) { return std::atof(from); }
  };

  // 转换到float类型
  template <typename From> struct converter<float, From> {
    static float convert(const std::string &from) {
      return static_cast<float>(std::atof(from.c_str()));
    }

    static float convert(const char *from) {
      return static_cast<float>(std::atof(from));
    }
  };

  // 转换到bool类型
  template <typename From> struct converter<bool, From> {
    static bool convert(int from) { return from > 0; }

    static bool convert(const std::string from) {
      return std::atoi(from.c_str()) > 0;
    }
  };

  // 转换到string类型
  template <typename From> struct converter<std::string, From> {
    static std::string convert(int from) { return std::to_string(from); }

    static std::string convert(double from) { return std::to_string(from); }

    static std::string convert(float from) { return std::to_string(from); }

    static std::string convert(const std::string &from) { return from; }

    static std::string convert(const char *from) { return from; }

    static std::string convert(char from) { return std::string(&from); }
  };
}; // StringUtil

class CheckStr {
  enum type {
    phone,
    tel,
    card,
    pwd,
    postal,
    QQ,
    email,
    money,
    URL,
    IP,
    date,
    number,
    english,
    chinese,
    lower,
    upper,
    HTML
  };

  static std::string checkStrType(type type) {
    switch (type) {
    case phone: // 手机号码
      return R"(^1[3|4|5|6|7|8|9][0-9]{9}$)";
    case tel: // 座机
      return R"(^(0\d{2,3}-\d{7,8})(-\d{1,4})?$)";
    case card: // 身份证
      return R"((^\d{15}$)|(^\d{18}$)|(^\d{17}(\d|X|x)$))";
    case pwd: // 密码以字母开头，长度在6~18之间，只能包含字母、数字和下划线
      return "^[a-zA-Z]\\w{5,17}$";
    case postal: // 邮政编码
      return R"([1-9]\d{5}(?!\d))";
    case QQ: // QQ号
      return "^[1-9][0-9]{4,9}$";
    case email: // 邮箱
      return R"(^[\w-]+(\.[\w-]+)*@[\w-]+(\.[\w-]+)+$)";
    case money: // 金额(小数点2位)
      return R"(^\d*(?:\.\d{0,2})?$)";
    case URL: // 网址
      return R"((http|ftp|https):\/\/[\w\-_]+(\.[\w\-_]+)+([\w\-\.,@?^=%&:/~\+#]*[\w\-\@?^=%&/~\+#])?)";
    case IP: // IP
      return R"(((?:(?:25[0-5]|2[0-4]\d|[01]?\d?\d)\.){3}(?:25[0-5]|2[0-4]\d|[01]?\d?\d)))";
    case date: // 日期时间
      return R"(^(\d{4})\-(\d{2})\-(\d{2}) (\d{2})(?:\:\d{2}|:(\d{2}):(\d{2}))$/.test(str) || ^(\d{4})\-(\d{2})\-(\d{2})$)";
    case number: // 数字
      return "^[0-9]$";
    case english: // 英文
      return "^[a-zA-Z]+$";
    case chinese: // 中文
      return R"(^[\\u4E00-\\u9FA5]+$)";
    case lower :   //小写
        return "^[a-z]+$";
    case upper :   //大写
        return "^[A-Z]+$";
    case HTML :    //HTML标记
        return R"(<("[^"]*"|'[^']*'|[^'">])*>)";
    default:
        return "";
            }
        }

    };//CheckStr
}