#pragma once
#include <string>
#include <cstdint>
#include "Common/DataDef.h"
using namespace Common;

namespace Utils {

/**
 * @brief 禁止构造基类
 */
class Nonconstructable {
public:
    Nonconstructable() = delete;
    ~Nonconstructable() = delete;
};

/**
 * @brief 禁止拷贝基类
 */
class Noncopyable {
public:
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator=(const Noncopyable&) = delete;

protected:
    Noncopyable() = default;
    ~Noncopyable() = default;
};

/**
 * @brief 禁止构造和拷贝基类
 */
class NoncopyableConstructable : public Noncopyable, public Nonconstructable {
};

/**
 * @brief 时间帮助类
 */
class TimeHelper : public NoncopyableConstructable {
public:
    /**
     * @note   格式：YYYY-MM-DD HH:MM:SS.fff
     * @brief  获取当前时间
     * @return 字符串格式当前时间
     */
    static std::string GetCurrentTime();

    /**
     * @note   格式：YYYY-MM-DD
     * @brief  获取当前日期
     * @return 字符串格式当前日期
     */
    static std::string GetCurrentData();
};

/**
 * @brief 字符串帮助类
 */
class StringHelper : public NoncopyableConstructable {
public:
    /**
     * @brief  获取文件名称
     * @return 文件名称
     * @param  path 文件路径
     */
    static const char* GetFileName(const char* path);

    /**
     * @brief  获取唯一ID
     * @return 唯一ID
     * @param  localIp 本端IP
     * @param  localPort 本端端口
     * @param  peerIp 对端IP
     * @param  peerPort 对端端口
     */
    static std::string GetUniqueId(const std::string& localIp, uint16_t localPort, const std::string& peerIp, uint16_t peerPort);

    /**
     * @brief  获取事件类型字符串
     * @return 事件类型字符串
     */
    static std::string EventTypeToString(Event_t event);

    /**
     * @brief  获取状态类型字符串
     * @return 状态类型字符串
     */
    static std::string StateTypeToString(State_t state);

    /**
     * @brief  获取poller控制类型字符串
     * @return poller控制类型字符串
     */
    static std::string PollerCtrlTypeToString(PollerCtrl_t type);
};

/**
 * @brief 目录帮助类
 */
class DirHelper : public NoncopyableConstructable {
public:
    /**
     * @brief  从完整路径中提取目录
     * @return 目录
     * @param  path 完整路径
     */
    static std::string GetDirectory(const std::string& path);

    /**
     * @brief  检查目录是否存在
     * @return 检查结果
     * @param  path 目录路径
     */
    static bool CheckDirectoryExists(const std::string& path);

    /**
     * @brief  创建目录
     * @return 创建结果
     * @param  path 目录路径
     */
    static bool CreateDirectory(const std::string& path);
};

}; // namespace Utils