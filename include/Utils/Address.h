#pragma once
#include <memory>
#include <string>
#include <cstdint>
#include <netinet/in.h>
#include "Common/DataDef.h"
using namespace Common;

namespace Utils {

/**
 * @brief 网络地址基类
 */
class Address : public std::enable_shared_from_this<Address> {
public:
    using Ptr = std::shared_ptr<Address>;
    using WPtr = std::weak_ptr<Address>;

public:
    virtual ~Address() = default;

public:
    /**
     * @brief  获取地址类型
     * @return 地址类型
     */
    virtual Addr_t getAddrType() const = 0;

    /**
     * @brief  获取socket地址
     * @return 获取结果
     * @param  addr socket地址
     */
    virtual bool getSockAddr(sockaddr& addr) const = 0;

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
     * @brief  获取ip地址和端口
     * @return 获取结果
     * @param  ip ip地址
     * @param  port 端口
     */
    virtual bool getIpPort(std::string& ip, uint16_t& port) const = 0;

    /**
     * @brief  地址是否有效
     * @return 判断结果
     */
    virtual bool valid() const = 0;

    /**
     * @brief  打印ip地址和端口
     * @return ip地址和端口
     */
    virtual std::string printIpPort() const = 0;
};

/**
 * @brief IPv4网络地址类
 */
class IPv4Address : public Address {
public:
    using Ptr = std::shared_ptr<IPv4Address>;
    using WPtr = std::weak_ptr<IPv4Address>;

public:
    explicit IPv4Address(const std::string& ip, uint16_t port);
    explicit IPv4Address(const sockaddr_in& addr);
    ~IPv4Address() override = default;

public:
    /**
     * @brief  获取地址类型
     * @return 地址类型
     */
    inline Addr_t getAddrType() const override {
        return Addr_t::IPv4;
    }

    /**
     * @brief  获取socket地址（网络序）
     * @return 获取结果
     * @param  addr socket地址
     */
    bool getSockAddr(sockaddr& addr) const override;

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
     * @brief  获取ip地址和端口
     * @return 获取结果
     * @param  ip ip地址
     * @param  port 端口
     */
    bool getIpPort(std::string& ip, uint16_t& port) const override;

    /**
     * @brief  地址是否有效
     * @return 判断结果
     */
    bool valid() const override;

    /**
     * @brief  打印ip地址和端口
     * @return ip地址和端口
     */
    std::string printIpPort() const override;

private:
    // socket地址（网络序）
    sockaddr_in m_addr;
};

/**
 * @brief IPv6网络地址类
 */
class IPv6Address : public Address {
public:
    using Ptr = std::shared_ptr<IPv6Address>;
    using WPtr = std::weak_ptr<IPv6Address>;

public:
    explicit IPv6Address(const std::string& ip, uint16_t port);
    explicit IPv6Address(const sockaddr_in6& addr);
    ~IPv6Address() override = default;

public:
    /**
     * @brief  获取地址类型
     * @return 地址类型
     */
    inline Addr_t getAddrType() const override {
        return Addr_t::IPv6;
    }

    /**
     * @brief  获取socket地址（网络序）
     * @return 获取结果
     * @param  addr socket地址
     */
    bool getSockAddr(sockaddr& addr) const override;

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
     * @brief  获取ip地址和端口
     * @return 获取结果
     * @param  ip ip地址
     * @param  port 端口
     */
    bool getIpPort(std::string& ip, uint16_t& port) const override;

    /**
     * @brief  地址是否有效
     * @return 判断结果
     */
    bool valid() const override;

    /**
     * @brief  打印ip地址和端口
     * @return ip地址和端口
     */
    std::string printIpPort() const override;

private:
    // socket地址（网络序）
    sockaddr_in6 m_addr;
};

}; // namespace Utils