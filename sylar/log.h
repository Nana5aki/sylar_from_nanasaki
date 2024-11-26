/*
 * @Author: Nana5aki
 * @Date: 2024-11-25 22:53:56
 * @LastEditors: Nana5aki
 * @LastEditTime: 2024-11-26 23:35:29
 * @FilePath: /MySylar/sylar/log.h
 */

/**
 * 流程：
 * 1.初始化LogFormatter，LogAppender, Logger
 * 2.通过宏定义提供流式风格和格式化风格的日志接口。每次写日志时，通过宏自动生成对应的日志事件LogEvent，并且将日志事件和日志器Logger包装到一起，生成一个LogEventWrap对象
 * 3.日志接口执行结束后，LogEventWrap对象析构，在析构函数里调用Logger的log方法将日志事件进行输出
 */

#ifndef __SYLAR_LOG_H
#define __SYLAR_LOG_H

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace sylar {

/**
 * @brief: 日志级别
 */
class LogLevel {
public:
    /**
     * @brief: 日志级别枚举
     */
    enum Level {
        /// 致命情况，系统不可用
        FATAL,
        /// 高优先级情况，例如数据库系统崩溃
        ALERT,
        /// 严重错误，例如硬盘错误
        CRIT,
        /// 错误
        ERROR,
        /// 警告
        WARN,
        /// 正常但值得注意
        NOTICE,
        /// 一般信息
        INFO,
        /// 调试信息
        DEBUG,
        /// 未设置
        NOTSET,
    };

    /**
     * @brief: 日志级别转字符串
     * @param[in] level 日志级别
     * @return 字符串形式的日志级别
     */
    static const char* ToString(LogLevel::Level level);

    /**
     * @brief: 字符串转日志级别
     * @param[in] str 字符串
     * @return 日志级别
     * @note 不区分大小写
     */
    static LogLevel::Level FromString(const std::string& str);
};

/**
 * @brief: 日志事件
 */
class LogEvent {
public:
    using ptr = std::shared_ptr<LogEvent>;

    /**
     * @brief 构造函数
     * @param[in] logger_name 日志器名称
     * @param[in] level 日志级别
     * @param[in] file 文件名
     * @param[in] line 行号
     * @param[in] elapse 从日志器创建开始到当前的累计运行毫秒
     * @param[in] thead_id 线程id
     * @param[in] fiber_id 协程id
     * @param[in] time UTC时间
     * @param[in] thread_name 线程名称
     */
    LogEvent(const std::string& logger_name, LogLevel::Level level, const char* file, int32_t line, int64_t elapse, uint32_t thread_id,
             uint64_t fiber_id, time_t time, const std::string& thread_name);

    /**
     * @brief 获取日志级别
     */
    LogLevel::Level getLevel() const { return m_level; }

    /**
     * @brief 获取日志内容
     */
    std::string getContent() const { return m_ss.str(); }

    /**
     * @brief 获取文件名
     */
    std::string getFile() const { return m_file; }

    /**
     * @brief 获取行号
     */
    int32_t getLine() const { return m_line; }

    /**
     * @brief 获取累计运行毫秒数
     */
    int64_t getElapse() const { return m_elapse; }

    /**
     * @brief 获取线程id
     */
    uint32_t getThreadId() const { return m_threadId; }

    /**
     * @brief 获取协程id
     */
    uint64_t getFiberId() const { return m_fiberId; }

    /**
     * @brief 返回时间戳
     */
    time_t getTime() const { return m_time; }

    /**
     * @brief 获取线程名称
     */
    const std::string& getThreadName() const { return m_threadName; }

    /**
     * @brief 获取内容字节流，用于流式写入日志
     */
    std::stringstream& getSS() { return m_ss; }

    /**
     * @brief 获取日志器名称
     */
    const std::string& getLoggerName() const { return m_loggerName; }

    /**
     * @brief C prinf风格写入日志
     */
    void printf(const char* fmt, ...);

    /**
     * @brief C vprintf风格写入日志
     */
    void vprintf(const char* fmt, va_list ap);

private:
    /// 日志级别
    LogLevel::Level m_level;
    /// 日志内容，使用stringstream存储，便于流式写入日志
    std::stringstream m_ss;
    /// 文件名
    const char* m_file = nullptr;
    /// 行号
    int32_t m_line = 0;
    /// 从日志器创建开始到当前的耗时
    int64_t m_elapse = 0;
    /// 线程id
    uint32_t m_threadId = 0;
    /// 协程id
    uint64_t m_fiberId = 0;
    /// UTC时间戳
    time_t m_time;
    /// 线程名称
    std::string m_threadName;
    /// 日志器名称
    std::string m_loggerName;
};

/**
 * @brief 日志格式化
 */
class LogFormatter {
public:
    using ptr = std::shared_ptr<LogFormatter>;

    /**
     * @brief 构造函数
     * @param[in] pattern 格式模板
     * @details 模板参数说明：
     * - %%m 消息
     * - %%p 日志级别
     * - %%c 日志器名称
     * - %%d 日期时间，后面可跟一对括号指定时间格式，比如%%d{%%Y-%%m-%%d %%H:%%M:%%S}，这里的格式字符与C语言strftime一致
     * - %%r 该日志器创建后的累计运行毫秒数
     * - %%f 文件名
     * - %%l 行号
     * - %%t 线程id
     * - %%F 协程id
     * - %%N 线程名称
     * - %%% 百分号
     * - %%T 制表符
     * - %%n 换行
     *
     * 默认格式：%%d{%%Y-%%m-%%d %%H:%%M:%%S}%%T%%t%%T%%N%%T%%F%%T[%%p]%%T[%%c]%%T%%f:%%l%%T%%m%%n
     */
    LogFormatter(const std::string& pattern = "%d{%Y-%m-%d %H:%M:%S} [%rms]%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n");

    /**
     * @brief 初始化，解析格式模板，提取模板项
     */
    void init();

    /**
     * @brief 模板解析是否出错
     */
    bool isError() const { return m_error; }

    /**
     * @brief 对日志事件进行格式化，返回格式化日志文本
     * @param[in] event 日志事件
     * @return 格式化日志字符串
     */
    std::string format(LogEvent::ptr event);

    /**
     * @brief 对日志事件进行格式化，返回格式化日志流
     * @param[in] event 日志事件
     * @param[in] os 日志输出流
     * @return 格式化日志流
     */
    std::ostream& format(std::ostream& os, LogEvent::ptr event);

    /**
     * @brief 获取pattern，日志格式化模板
     */
    std::string getPattern() const { return m_pattern; }

public:
    /**
     * @brief 日志内容格式化项，虚基类，用于派生出不同的格式化项
     */
    class FormatItem {
    public:
        typedef std::shared_ptr<FormatItem> ptr;

        /**
         * @brief 析构函数
         */
        virtual ~FormatItem() {}

        /**
         * @brief 格式化日志事件
         */
        virtual void format(std::ostream& os, LogEvent::ptr event) = 0;
    };

private:
    /// 日志格式模板
    std::string m_pattern;
    /// 解析后的格式模板数组
    std::vector<FormatItem::ptr> m_items;
    /// 是否出错
    bool m_error = false;
};

LogFormatter::LogFormatter(const std::string &pattern)
    : m_pattern(pattern) {
    init();
}



};  // namespace sylar

#endif