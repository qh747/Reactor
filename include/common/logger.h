#pragma once
#include <mutex>
#include <atomic>
#include <sstream>
#include <functional>
#include <unordered_map>
#include "common/utils.h"

namespace COMMON {

// 日志宏定义
#define LOG(level) Logger(level, StringHelper::GetFileName(__FILE__), __LINE__)

// 便捷宏定义
#define LOG_DEBUG LOG(LogLevel::DEBUG)
#define LOG_INFO LOG(LogLevel::INFO)
#define LOG_WARN LOG(LogLevel::WARN)
#define LOG_ERROR LOG(LogLevel::ERROR)
#define LOG_FATAL LOG(LogLevel::FATAL)

/**
 * @brief 日志级别
 */
enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

/**
 * @brief 日志类
 */
class Logger : public Noncopyable {
public:
    using LogPrintCb = std::function<void(const std::string&, int, const std::string&)>;
    using LogPrintMap = std::unordered_map<LogLevel, LogPrintCb>;

public:
    Logger(LogLevel level, const std::string& file, int line)
        : m_level(level), m_file(file), m_line(line) {
    }

    ~Logger() {
        flush();
    }

public:
    /**
     * @brief 设置最低日志级别
     * @param level 最低日志级别
     */
    static inline void SetLowestLevel(LogLevel level = LogLevel::DEBUG) {
        m_lowestLevel.store(level);
    }

    /**
     * @brief 设置是否将日志写入文件
     * @param enabled 是否将日志写入文件
     */
    static inline void enableWriteFile(bool enabled = true) {
        m_enableWriteFile.store(enabled);
    }

public:
    /**
     * @brief 支持各种类型的输入
     * @param value 输入值
     * @return 日志流式代理类
     */
    template <typename T>
    Logger& operator<<(const T& value) {
        m_buffer << value;
        return *this;
    }

    /**
     * @brief 支持 std::endl 等流操作符
     * @param manip 输入值
     * @return 日志流式代理类
     */
    Logger& operator<<(std::ostream& (*manip)(std::ostream&)) {
        manip(m_buffer);
        return *this;
    }

private:
    /**
     * @brief 刷新缓冲区内容到输出窗口
     */
    void flush();

private:
    // 输出日志所在行
    int m_line;

    // 日志级别
    LogLevel m_level;

    // 输出日志所在文件
    std::string m_file;

    // 日志内容缓冲区
    std::ostringstream m_buffer;

private:
    // 最低日志级别
    static std::atomic<LogLevel> m_lowestLevel;

    // 日志输出文件标志位
    static std::atomic<bool> m_enableWriteFile;

    // 日志打印互斥量
    static std::mutex LogPrintCbLock;

    // 日志打印回调函数map
    static LogPrintMap LogPrintCbMap;
};

}; // namespace COMMON