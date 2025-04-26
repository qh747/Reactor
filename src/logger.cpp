#include <fstream>
#include <iostream>
#include <unistd.h>
#include "common/logger.h"

namespace COMMON {

#define CURRANT_TIME TimeHelper::GetCurrentTime()

std::mutex Logger::LogPrintCbLock;
Logger::LogPrintMap Logger::LogPrintCbMap;
std::atomic<bool> Logger::m_enableWriteFile{true};
std::atomic<LogLevel> Logger::m_lowestLevel{LogLevel::DEBUG};

class LogFileWriter : public Noncopyable {
public:
    static LogFileWriter& Instance() {
        static LogFileWriter instance;
        return instance;
    }

    void write(const std::string& str) {
        m_logFile << str;
    }

private:
    LogFileWriter() {
        // 获取可执行程序路径
        char buf[1024];
        ssize_t count = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
        if (-1 == count) {
            throw std::runtime_error("Create log file error. get exec path failed");
        }

        buf[count] = '\0';

        // 获取可执行程序所在目录
        std::string exeDir = DirHelper::GetDirectory(buf);
        if (exeDir.empty()) {
            throw std::runtime_error("Create log file error. get exec dir failed");
        }

        // 检查是否存在log目录
        std::string logDir = exeDir + "/log";
        if (!DirHelper::CheckDirectoryExists(logDir)) {
            if (!DirHelper::CreateDirectory(logDir)) {
                throw std::runtime_error("Create log file error. create log dir failed");
            }
        }

        // 创建日志文件
        std::string logFilePath = logDir + "/log_" + TimeHelper::GetCurrentData() + ".log";
        m_logFile.open(logFilePath, std::ios::app);
        if (!m_logFile.is_open()) {
            throw std::runtime_error("Create log file error. open log file failed");
        }
    }

    ~LogFileWriter() {
        m_logFile.close();
    }

private:
    std::ofstream m_logFile;
};

void Logger::flush() {
    if (m_level < m_lowestLevel) {
        return;
    }

    if (!LogPrintCbMap.empty()) {
        // 除第一次日志打印情况外都进入次逻辑
        LogPrintCbMap[m_level](m_file, m_line, m_buffer.str());
    }
    else {
        // 第一次日志打印进入此逻辑
        LogPrintCbMap[LogLevel::DEBUG] = [](const std::string& file, int line, const std::string& str) {
            std::lock_guard<std::mutex> lock(LogPrintCbLock);

            std::stringstream ss;
            ss << "[DEBUG] " << CURRANT_TIME << " " << file << ":" << line << " " << str << std::endl;

            if (m_enableWriteFile) {
                LogFileWriter::Instance().write(ss.str());
            }

            std::cout << ss.str();
        };

        LogPrintCbMap[LogLevel::INFO] = [](const std::string& file, int line, const std::string& str) {
            std::lock_guard<std::mutex> lock(LogPrintCbLock);

            std::stringstream ss;
            ss << "[INFO]  " << CURRANT_TIME << " " << file << ":" << line << " " << str << std::endl;

            if (m_enableWriteFile) {
                LogFileWriter::Instance().write(ss.str());
            }

            std::cout << ss.str();
        };

        LogPrintCbMap[LogLevel::WARN] = [](const std::string& file, int line, const std::string& str) {
            std::lock_guard<std::mutex> lock(LogPrintCbLock);

            std::stringstream ss;
            ss << "[WARN]  " << CURRANT_TIME << " " << file << ":" << line << " " << str << std::endl;

            if (m_enableWriteFile) {
                LogFileWriter::Instance().write(ss.str());
            }

            std::cout << ss.str();
        };

        LogPrintCbMap[LogLevel::ERROR] = [](const std::string& file, int line, const std::string& str) {
            std::lock_guard<std::mutex> lock(LogPrintCbLock);

            std::stringstream ss;
            ss << "[ERROR] " << CURRANT_TIME << " " << file << ":" << line << " " << str << std::endl;

            if (m_enableWriteFile) {
                LogFileWriter::Instance().write(ss.str());
            }

            std::cout << ss.str();
        };

        LogPrintCbMap[LogLevel::FATAL] = [](const std::string& file, int line, const std::string& str) {
            std::lock_guard<std::mutex> lock(LogPrintCbLock);

            std::stringstream ss;
            ss << "[FATAL] " << CURRANT_TIME << " " << file << ":" << line << " " << str << std::endl;

            if (m_enableWriteFile) {
                LogFileWriter::Instance().write(ss.str());
            }

            std::cout << ss.str();
            std::exit(EXIT_FAILURE);
        };

        std::stringstream ss;
        ss << m_file << ":" << m_line << " " << m_buffer.str();
        LogPrintCbMap[m_level](m_file, m_line, m_buffer.str());
    }
}

} // namespace COMMON