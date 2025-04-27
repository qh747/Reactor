#pragma once
#include <string>
#include <cstdint>
#include <netinet/in.h>

namespace NET {

/**
 * @brief 网络地址类型
 */
enum class AddrType {
    IPv4,
    IPv6
};

/**
 * @brief 网络地址基类
 */
class InetAddr {
public:
    virtual ~InetAddr() = default;

public:
    /**
     * @brief  获取地址类型
     * @return 地址类型
     */
    virtual AddrType getAddrType() const = 0;

    /**
     * @brief  获取socket地址
     * @return 获取结果
     * @param  addr socket地址
     */
    virtual bool getSockAddr(sockaddr_in& addr) const = 0;

    /**
     * @brief  获取ip地址
     * @return 获取结果
     * @param  ip ip地址
     */
    virtual bool getIpAddr(std::string& ip) const = 0;

    /**
     * @brief  获取端口
     * @return 获取结果
     * @param  port 端口
     */
    virtual bool getPort(uint16_t& port) const = 0;

    /**
     * @brief  地址是否有效
     * @return 判断结果
     */
    virtual bool valid() const = 0;
};

/**
 * @brief IPv4网络地址类
 */
class InetAddrV4 : public InetAddr {
public:
    explicit InetAddrV4(const std::string& ip, uint16_t port);
    explicit InetAddrV4(const sockaddr_in& addr);
    ~InetAddrV4() override = default;

public:
    /**
     * @brief  获取地址类型
     * @return 地址类型
     */
    inline AddrType getAddrType() const override {
        return AddrType::IPv4;
    }

    /**
     * @brief  获取socket地址（网络序）
     * @return 获取结果
     * @param  addr socket地址
     */
    bool getSockAddr(sockaddr_in& addr) const override;

    /**
     * @brief  获取ip地址（主机序）
     * @return 获取结果
     * @param  ip ip地址
     */
    bool getIpAddr(std::string& ip) const override;

    /**
     * @brief  获取端口（主机序）
     * @return 获取结果
     * @param  port 端口
     */
    bool getPort(uint16_t& port) const override;

    /**
     * @brief  地址是否有效
     * @return 判断结果
     */
    bool valid() const override;

private:
    // socket地址（网络序）
    sockaddr_in m_addr;
};

}; // namespace NET