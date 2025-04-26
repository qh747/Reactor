#pragma once
#include <string>
#include <cstdint>

namespace COMMON {

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

/**
 * @brief 地址帮助类
 */
class AddrHelper : public NoncopyableConstructable {
public:
    /**
     * @brief  从网络字节序转换为主机字节序
     * @return 转换结果
     * @param  ipAddrNet  网络字节序ip地址
     * @param  ipAddrHost 主机字节序ip地址
     */
    static bool IpAddrFromNetToHost(const std::string& ipAddrNet, std::string& ipAddrHost);

    /**
     * @brief  从主机字节序转换为网络字节序
     * @return 转换结果
     * @param  ipAddrHost 主机字节序ip地址
     * @param  ipAddrNet  网络字节序ip地址
     */
    static bool IpAddrFromHostToNet(const std::string& ipAddrHost, std::string& ipAddrNet);

    /**
     * @brief  从网络字节序转换为主机字节序
     * @return 转换结果
     * @param  portNet  网络字节序端口
     * @param  portHost 主机字节序端口
     */
    static bool PortFromNetToHost(uint16_t portNet, uint16_t& portHost);

    /**
     * @brief  从主机字节序转换为网络字节序
     * @return 转换结果
     * @param  portHost 主机字节序端口
     * @param  portNet  网络字节序端口
     */
    static bool PortFromHostToNet(uint16_t portHost, uint16_t& portNet);
};

}; // namespace COMMON