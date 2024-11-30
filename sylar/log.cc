/*
 * @Author: Nana5aki
 * @Date: 2024-11-25 22:53:59
 * @LastEditors: Nana5aki
 * @LastEditTime: 2024-11-30 16:35:42
 * @FilePath: /MySylar/sylar/log.cc
 */

#include "log.h"

#include "config.h"
#include "util.h"

#include <cstdarg>
#include <functional>
#include <map>

namespace sylar {

const char* LogLevel::ToString(LogLevel::Level level) {
    switch (level) {
#define XX(name)         \
    case LogLevel::name: \
        return #name;
        XX(FATAL);
        XX(ALERT);
        XX(CRIT);
        XX(ERROR);
        XX(WARN);
        XX(NOTICE);
        XX(INFO);
        XX(DEBUG);
#undef XX
    default:
        return "NOTSET";
    }
    return "NOTSET";
}

LogLevel::Level LogLevel::FromString(const std::string& str) {
#define XX(level, v)            \
    if (str == #v) {            \
        return LogLevel::level; \
    }
    XX(FATAL, fatal);
    XX(ALERT, alert);
    XX(CRIT, crit);
    XX(ERROR, error);
    XX(WARN, warn);
    XX(NOTICE, notice);
    XX(INFO, info);
    XX(DEBUG, debug);

    XX(FATAL, FATAL);
    XX(ALERT, ALERT);
    XX(CRIT, CRIT);
    XX(ERROR, ERROR);
    XX(WARN, WARN);
    XX(NOTICE, NOTICE);
    XX(INFO, INFO);
    XX(DEBUG, DEBUG);
#undef XX

    return LogLevel::NOTSET;
}

LogEvent::LogEvent(const std::string& logger_name, LogLevel::Level level, const char* file,
                   int32_t line, int64_t elapse, uint32_t thread_id, uint64_t fiber_id, time_t time,
                   const std::string& thread_name)
    : m_level(level)
    , m_file(file)
    , m_line(line)
    , m_elapse(elapse)
    , m_threadId(thread_id)
    , m_fiberId(fiber_id)
    , m_time(time)
    , m_threadName(thread_name)
    , m_loggerName(logger_name) {
}

void LogEvent::printf(const char* fmt, ...) {
    va_list ap;
    // 获取从fmt参数后的所有参数
    va_start(ap, fmt);
    vprintf(fmt, ap);
    // 结束对可变参数的访问
    va_end(ap);
}

void LogEvent::vprintf(const char* fmt, va_list ap) {
    char* buf = nullptr;
    // 生成格式化字符串并存入buf中
    /**
     * vasprintf 是一个 C
     * 库函数，它可以通过可变参数创建一个格式化的字符串，并将其存储在动态分配的内存中 int
     * vasprintf(char **str, const char *format, va_list ap);
     * str：指向指针变量的指针，用于存储格式化字符串的结果
     * format：格式化字符串的格式，与 printf 函数中的用法相同
     * ap：指向使用 va_start 和 va_arg 函数处理的可变参数列表的指针
     */
    int len = vasprintf(&buf, fmt, ap);
    if (len != -1) {
        // 将生成的日志内容输入到日志流中
        m_ss << std::string(buf, len);
        free(buf);
    }
}

/**
 * @brief 日志内容格式化类
 * @details 写入日志内容
 */
class MessageFormatItem : public LogFormatter::FormatItem {
public:
    MessageFormatItem(const std::string& str) {
    }
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getContent();
    }
};

/**
 * @brief 日志级别格式化类
 * @details 写入日志级别
 */
class LevelFormatItem : public LogFormatter::FormatItem {
public:
    LevelFormatItem(const std::string& str) {
    }
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << LogLevel::ToString(event->getLevel());
    }
};

/**
 * @brief 日志器运行时间格式化类
 * @details 写入运行时间
 */
class ElapseFormatItem : public LogFormatter::FormatItem {
public:
    ElapseFormatItem(const std::string& str) {
    }
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getElapse();
    }
};

/**
 * @brief 日志器名字格式化类
 * @details 写入日志器名字
 */
class LoggerNameFormatItem : public LogFormatter::FormatItem {
public:
    LoggerNameFormatItem(const std::string& str) {
    }
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getLoggerName();
    }
};

/**
 * @brief 线程ID格式化类
 * @details 写入线程ID
 */
class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
    ThreadIdFormatItem(const std::string& str) {
    }
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getThreadId();
    }
};

/**
 * @brief 协程ID格式化类
 * @details 写入协程ID
 */
class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    FiberIdFormatItem(const std::string& str) {
    }
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getFiberId();
    }
};

/**
 * @brief 线程名称格式化类
 * @details 写入线程名称
 */
class ThreadNameFormatItem : public LogFormatter::FormatItem {
public:
    ThreadNameFormatItem(const std::string& str) {
    }
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getThreadName();
    }
};

/**
 * @brief 时间格式化类
 * @details 写入时间
 */
class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
    // 时间默认格式化为 %Y-%m-%d %H:%M:%S
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
        : m_format(format) {
        if (m_format.empty()) {
            m_format = "%Y-%m-%d %H:%M:%S";
        }
    }

    void format(std::ostream& os, LogEvent::ptr event) override {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        // 将时间信息转换为指定格式的字符串
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }

private:
    std::string m_format;
};

/**
 * @brief 文件名字格式化类
 * @details 写入文件名字
 */
class FileNameFormatItem : public LogFormatter::FormatItem {
public:
    FileNameFormatItem(const std::string& str) {
    }
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getFile();
    }
};

/**
 * @brief 行号格式化类
 * @details 写入行号
 */
class LineFormatItem : public LogFormatter::FormatItem {
public:
    LineFormatItem(const std::string& str) {
    }
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getLine();
    }
};

/**
 * @brief 换行格式化类
 * @details 写入换行符
 */
class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    NewLineFormatItem(const std::string& str) {
    }
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << std::endl;
    }
};

/**
 * @brief 任意内容格式化类
 * @details 写入一个任意内容字符串
 */
class StringFormatItem : public LogFormatter::FormatItem {
public:
    StringFormatItem(const std::string& str)
        : m_string(str) {
    }
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << m_string;
    }

private:
    std::string m_string;
};

/**
 * @brief 制表符格式化类
 * @details 写入一个制表符
 */
class TabFormatItem : public LogFormatter::FormatItem {
public:
    TabFormatItem(const std::string& str) {
    }
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << "\t";
    }
};

/**
 * @brief %格式化类
 * @details 写入一个%
 */
class PercentSignFormatItem : public LogFormatter::FormatItem {
public:
    PercentSignFormatItem(const std::string& str) {
    }
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << "%";
    }
};

/**
 * 简单的状态机判断，提取pattern中的常规字符和模式字符
 *
 * 解析的过程就是从头到尾遍历，根据状态标志决定当前字符是常规字符还是模式字符
 *
 * 一共有两种状态，即正在解析常规字符和正在解析模板转义字符
 *
 * 比较麻烦的是%%d，后面可以接一对大括号指定时间格式，比如%%d{%%Y-%%m-%%d
 * %%H:%%M:%%S}，这个状态需要特殊处理
 *
 * 一旦状态出错就停止解析，并设置错误标志，未识别的pattern转义字符也算出错
 *
 * @see LogFormatter::LogFormatter
 */
void LogFormatter::init() {
    // 按顺序存储解析到的pattern项
    // 每个pattern包括一个整数类型和一个字符串，类型为0表示该pattern是常规字符串，为1表示该pattern需要转义
    // 日期格式单独用下面的dataformat存储
    std::vector<std::pair<int, std::string>> patterns;
    // 临时存储常规字符串
    std::string tmp;
    // 日期格式字符串，默认把位于%d后面的大括号对里的全部字符都当作格式字符，不校验格式是否合法
    std::string dateformat;
    // 是否解析出错
    bool error = false;

    // 是否正在解析常规字符，初始时为true
    bool parsing_string = true;
    // 是否正在解析模板字符，%后面的是模板字符

    size_t i = 0;
    while (i < m_pattern.size()) {
        std::string c = std::string(1, m_pattern[i]);
        char c = m_pattern[i];
        if (c == "%") {
            if (parsing_string) {
                if (!tmp.empty()) {
                    patterns.push_back(std::make_pair(0, tmp));
                }
                tmp.clear();
                parsing_string = false;   // 在解析常规字符时遇到%，表示开始解析模板字符
                // parsing_pattern = true;
                i++;
                continue;
            } else {
                patterns.push_back(std::make_pair(1, c));
                parsing_string = true;   // 在解析模板字符时遇到%，表示这里是一个%转义
                // parsing_pattern = false;
                i++;
                continue;
            }
        } else {                    // not %
            if (parsing_string) {   // 持续解析常规字符直到遇到%，解析出的字符串作为一个常规字符串加入patterns
                // 这里是独立于模板字符之外的常规字符串，用于添加一些单位？例如ms等
                tmp += c;
                i++;
                continue;
            } else {   // 模板字符，直接添加到patterns中，添加完成后，状态变为解析常规字符，%d特殊处理
                patterns.push_back(std::make_pair(1, c));
                parsing_string = true;

                // 后面是对%d的特殊处理，如果%d后面直接跟了一对大括号，那么把大括号里面的内容提取出来作为dateformat
                if (c != "d") {
                    i++;
                    continue;
                }
                i++;
                if (i < m_pattern.size() && m_pattern[i] != '{') {
                    continue;
                }
                i++;
                while (i < m_pattern.size() && m_pattern[i] != '}') {
                    dateformat.push_back(m_pattern[i]);
                    i++;
                }
                if (m_pattern[i] != '}') {
                    // %d后面的大括号没有闭合，直接报错
                    std::cout << "[ERROR] LogFormatter::init() " << "pattern: [" << m_pattern
                              << "] '{' not closed" << std::endl;
                    error = true;
                    break;
                }
                i++;
                continue;
            }
        }
    }   // end while(i < m_pattern.size())

    if (error) {
        m_error = true;
        return;
    }

    // 模板解析结束之后剩余的常规字符也要算进去
    if (!tmp.empty()) {
        patterns.push_back(std::make_pair(0, tmp));
        tmp.clear();
    }

    static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)>>
        s_format_items = {
#define XX(str, C) {#str, [](const std::string& fmt) { return FormatItem::ptr(new C(fmt)); }}

            XX(m, MessageFormatItem),       // m:消息
            XX(p, LevelFormatItem),         // p:日志级别
            XX(c, LoggerNameFormatItem),    // c:日志器名称
                                            //        XX(d, DateTimeFormatItem),          //
                                            //        d:日期时间，单独分开处理
            XX(r, ElapseFormatItem),        // r:累计毫秒数
            XX(f, FileNameFormatItem),      // f:文件名
            XX(l, LineFormatItem),          // l:行号
            XX(t, ThreadIdFormatItem),      // t:编程号
            XX(F, FiberIdFormatItem),       // F:协程号
            XX(N, ThreadNameFormatItem),    // N:线程名称
            XX(%, PercentSignFormatItem),   // %:百分号
            XX(T, TabFormatItem),           // T:制表符
            XX(n, NewLineFormatItem),       // n:换行符
#undef XX
        };

    for (auto& v : patterns) {
        if (v.first == 0) {
            m_items.push_back(FormatItem::ptr(new StringFormatItem(v.second)));
        } else if (v.second == "d") {
            m_items.push_back(FormatItem::ptr(new DateTimeFormatItem(dateformat)));
        } else {
            auto it = s_format_items.find(v.second);
            if (it == s_format_items.end()) {
                std::cout << "[ERROR] LogFormatter::init() " << "pattern: [" << m_pattern << "] "
                          << "unknown format item: " << v.second << std::endl;
                error = true;
                break;
            } else {
                m_items.push_back(it->second(v.second));
            }
        }
    }

    if (error) {
        m_error = true;
        return;
    }
}

std::string LogFormatter::format(LogEvent::ptr event) {
    std::stringstream ss;
    for (auto& i : m_items) {
        i->format(ss, event);
    }
    return ss.str();
}

std::ostream& LogFormatter::format(std::ostream& os, LogEvent::ptr event) {
    for (auto& i : m_items) {
        i->format(os, event);
    }
    return os;
}

LogAppender::LogAppender(LogFormatter::ptr default_formatter)
    : m_defaultFormatter(default_formatter) {
}

void LogAppender::setFormatter(LogFormatter::ptr val) {
    MutexType::Lock lock(m_mutex);
    m_formatter = val;
}

LogFormatter::ptr LogAppender::getFormatter() {
    MutexType::Lock lock(m_mutex);
    return m_formatter ? m_formatter : m_defaultFormatter;
}

StdoutLogAppender::StdoutLogAppender()
    : LogAppender(LogFormatter::ptr(new LogFormatter)) {
}

void StdoutLogAppender::log(LogEvent::ptr event) {
    if (m_formatter) {
        m_formatter->format(std::cout, event);
    } else {
        m_defaultFormatter->format(std::cout, event);
    }
}

std::string StdoutLogAppender::toYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["type"] = "StdoutLogAppender";
    node["pattern"] = m_formatter->getPattern();
    std::stringstream ss;
    ss << node;
    return ss.str();
}

FileLogAppender::FileLogAppender(const std::string& file)
    : LogAppender(LogFormatter::ptr(new LogFormatter)) {
    m_filename = file;
    reopen();
    if (m_reopenError) {
        std::cout << "reopen file " << m_filename << " error" << std::endl;
    }
}

void FileLogAppender::log(LogEvent::ptr event) {
    uint64_t now = event->getTime();
    // 如果一个日志事件距离上次写日志超过3秒，那就重新打开一次日志文件
    if (now >= (m_lastTime + 3)) {
        reopen();
        if (m_reopenError) {
            std::cout << "reopen file " << m_filename << " error" << std::endl;
        }
        m_lastTime = now;
    }
    if (m_reopenError) {
        return;
    }
    MutexType::Lock lock(m_mutex);
    if (m_formatter) {
        if (!m_formatter->format(m_filestream, event)) {
            std::cout << "[ERROR] FileLogAppender::log() format error" << std::endl;
        }
    } else {
        if (!m_defaultFormatter->format(m_filestream, event)) {
            std::cout << "[ERROR] FileLogAppender::log() format error" << std::endl;
        }
    }
}

bool FileLogAppender::reopen() {
    MutexType::Lock lock(m_mutex);
    if (m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename, std::ios::app);
    m_reopenError = !m_filestream;
    return !m_reopenError;
}

std::string FileLogAppender::toYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["type"] = "FileLogAppender";
    node["file"] = m_filename;
    node["pattern"] = m_formatter ? m_formatter->getPattern() : m_defaultFormatter->getPattern();
    std::stringstream ss;
    ss << node;
    return ss.str();
}

Logger::Logger(const std::string& name)
    : m_name(name)
    , m_level(LogLevel::INFO)
    , m_createTime(GetElapsedMS()) {
}

void Logger::addAppender(LogAppender::ptr appender) {
    MutexType::Lock lock(m_mutex);
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
    MutexType::Lock lock(m_mutex);
    for (auto it = m_appenders.begin(); it != m_appenders.end(); it++) {
        if (*it == appender) {
            m_appenders.erase(it);
            break;
        }
    }
}

void Logger::clearAppenders() {
    MutexType::Lock lock(m_mutex);
    m_appenders.clear();
}

/**
 * 调用Logger的所有appenders将日志写一遍，
 * Logger至少要有一个appender，否则没有输出
 */
void Logger::log(LogEvent::ptr event) {
    if (event->getLevel() <= m_level) {
        for (auto& i : m_appenders) {
            i->log(event);
        }
    }
}

std::string Logger::toYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["name"] = m_name;
    node["level"] = LogLevel::ToString(m_level);
    for (auto& i : m_appenders) {
        node["appenders"].push_back(YAML::Load(i->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

LogEventWrap::LogEventWrap(Logger::ptr logger, LogEvent::ptr event)
    : m_logger(logger)
    , m_event(event) {
}

/**
 * @note LogEventWrap在析构时写日志
 */
LogEventWrap::~LogEventWrap() {
    m_logger->log(m_event);
}

LoggerManager::LoggerManager() {
    m_root.reset(new Logger("root"));
    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));
    m_loggers[m_root->getName()] = m_root;
    init();
}

/**
 * 如果指定名称的日志器未找到，那会就新创建一个，但是新创建的Logger是不带Appender的，
 * 需要手动添加Appender
 */
Logger::ptr LoggerManager::getLogger(const std::string& name) {
    MutexType::Lock lock(m_mutex);
    auto it = m_loggers.find(name);
    if (it != m_loggers.end()) {
        return it->second;
    }

    Logger::ptr logger(new Logger(name));
    m_loggers[name] = logger;
    return logger;
}

/**
 * @todo 实现从配置文件加载日志配置
 */
void LoggerManager::init() {
}

std::string LoggerManager::toYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    for (auto& i : m_loggers) {
        node.push_back(YAML::Load(i.second->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

};   // namespace sylar