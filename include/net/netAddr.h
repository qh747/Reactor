#pragma once
#include <string>
#include <cstdint>
#include <netinet/in.h>

namespace NET {

/**
 * @brief 网络地址类型
 */
enum class AddrType {
    IPV4,
    IPV6
};

/**
 * @brief 地址字节序类型
 */
enum AddrByteOrder {
    HOST,
    NETWORK
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
     * @param  order 网络字节序类型
     * @param  addr  socket地址
     */
    virtual bool getSockAddr(AddrByteOrder order, sockaddr_in& addr) const = 0;

    /**
     * @brief  获取ip地址
     * @return 获取结果
     * @param  order 网络字节序类型
     * @param  ip    ip地址
     */
    virtual bool getIpAddr(AddrByteOrder order, std::string& ip) const = 0;

    /**
     * @brief  获取端口
     * @return 获取结果
     * @param  order 网络字节序类型
     * @param  port  端口
     */
    virtual bool getPort(AddrByteOrder order, uint16_t& port) const = 0;

    /**
     * @note   格式：ip:port
     * @brief  获取ip地址和端口
     * @return ip地址和端口
     * @param  order 网络字节序类型
     */
    virtual bool getIpPort(AddrByteOrder order, std::string& ipPort) const = 0;

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
    explicit InetAddrV4(const std::string& ip, uint16_t port, AddrByteOrder order);
    explicit InetAddrV4(const sockaddr_in& addr, AddrByteOrder order);
    ~InetAddrV4() override = default;

public:
    /**
     * @brief  获取地址类型
     * @return 地址类型
     */
    inline AddrType getAddrType() const override {
        return AddrType::IPV4;
    }

    /**
     * @brief  获取socket地址
     * @return 获取结果
     * @param  order 网络字节序类型
     * @param  addr  socket地址
     */
    bool getSockAddr(AddrByteOrder order, sockaddr_in& addr) const override;

    /**
     * @brief  获取ip地址
     * @return 获取结果
     * @param  order 网络字节序类型
     * @param  ip    ip地址
     */
    bool getIpAddr(AddrByteOrder order, std::string& ip) const override;

    /**
     * @brief  获取端口
     * @return 获取结果
     * @param  order 网络字节序类型
     * @param  port  端口
     */
    bool getPort(AddrByteOrder order, uint16_t& port) const override;

    /**
     * @note   格式：ip:port
     * @brief  获取ip地址和端口
     * @return ip地址和端口
     * @param  order 网络字节序类型
     */
    bool getIpPort(AddrByteOrder order, std::string& ipPort) const override;

    /**
     * @brief  地址是否有效
     * @return 判断结果
     */
    bool valid() const override;

private:
    // socket地址，使用主机字节序保存
    sockaddr_in m_addr;
};

}; // namespace NET