#pragma once
#include <memory>
#include "Common/DataDef.h"
#include "Utils/Utils.h"
#include "Utils/Address.h"
using namespace Utils;
using namespace Common;

namespace Net {

/**
 * @brief Socket封装类
 */
class Socket : public Noncopyable, public std::enable_shared_from_this<Socket> {
public:
    using Ptr = std::shared_ptr<Socket>;
    using WPtr = std::weak_ptr<Socket>;

public:
    Socket(int fd, const Socket_t& type);
    ~Socket();

public:
    /**
     * @brief  绑定地址
     * @return 绑定结果
     * @param  addr socket地址
     */
    bool bind(const Address::Ptr& addr);

    /**
     * @brief  监听
     * @return 监听结果
     */
    bool listen() const;

    /**
     * @note   仅client socket使用
     * @brief  连接
     * @return 连接结果
     * @param  peerAddr 对端地址
     */
    bool connect(const Address::Ptr& peerAddr);

    /**
     * @brief 复用地址
     * @param enabled 是否启用
     */
    void setReuseAddr(bool enabled) const;

    /**
     * @brief 是否复用地址
     * @param enabled 判断结果
     */
    bool getReuseAddr() const;

    /**
     * @brief 复用端口
     * @param enabled 是否启用
     */
    void setReusePort(bool enabled) const;

    /**
     * @brief 是否复用端口
     * @param enabled 判断结果
     */
    bool getReusePort() const;

public:
    /**
     * @brief  获取socket fd
     * @return socket fd
     */
    inline int getFd() const {
        return m_fd;
    }

    /**
     * @brief  获取socket类型
     * @return socket类型
     */
    inline Socket_t getType() const {
        return m_type;
    }

    /**
     * @brief  判断本端地址是否有效
     * @return 判断结果
     */
    inline bool isLocalAddrValid() const {
        return nullptr != m_localAddr && m_localAddr->valid();
    }

    /**
     * @brief  获取socket本地地址
     * @return socket本地地址
     */
    inline std::string getLocalIpAddr() const {
        std::string ipAddr;
        if (this->isLocalAddrValid()) {
            m_localAddr->getIpAddr(ipAddr);
        }
        return ipAddr;
    }

    /**
     * @brief  获取socket本地端口
     * @return socket本地端口
     */
    inline uint16_t getLocalPort() const {
        uint16_t port = 0;
        if (this->isLocalAddrValid()) {
            m_localAddr->getPort(port);
        }
        return port;
    }

    /**
     * @brief  判断对端地址是否有效
     * @return 判断结果
     */
    inline bool isRemoteAddrValid() const {
        return nullptr != m_peerAddr && m_peerAddr->valid();
    }

    /**
     * @brief  获取socket对端地址
     * @return socket对端地址
     */
    inline std::string getRemoteIpAddr() const {
        std::string ipAddr;
        if (this->isRemoteAddrValid()) {
            m_peerAddr->getIpAddr(ipAddr);
        }
        return ipAddr;
    }

    /**
     * @brief  获取socket对端端口
     * @return socket对端端口
     */
    inline uint16_t getRemotePort() const {
        uint16_t port = 0;
        if (this->isRemoteAddrValid()) {
            m_localAddr->getPort(port);
        }
        return port;
    }

public:
    /**
     * @note   仅server socket使用
     * @brief  接受连接
     * @return 接受结果
     * @param  peerSock 对端socket
     */
    bool accept(Socket::Ptr& peerSock) const;

    /**
     * @note  仅tcp socket使用
     * @brief 关闭socket
     * @param type 关闭类型
     */
    void shutdown(SocketShutdown_t type) const;

    /**
     * @note  仅tcp socket使用
     * @brief 设置是否启用nagle算法
     * @param enabled 是否启用
     */
    void setNoDelayEnabled(bool enabled) const;

    /**
     * @note  仅tcp socket使用
     * @brief 设置是否启用心跳机制
     * @param enabled 是否启用
     */
    void setKeepaliveEnabled(bool enabled) const;

private:
    // socket fd
    const int m_fd;

    // socket type
    const Socket_t m_type;

    // socket local address
    Address::Ptr m_localAddr;

    // socket peer address
    Address::Ptr m_peerAddr;
};

}; // namespace Net