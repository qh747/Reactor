#pragma once
#include "Common/DataDef.h"
#include "Utils/Utils.h"
#include "Utils/Address.h"
using namespace Common;

namespace Utils {

/**
 * @brief Socket操作类
 */
class Socketop : public NoncopyableConstructable {
public:
    /**
     * @brief  创建套接字
     * @return 创建结果
     * @param  addrType 地址类型
     * @param  sockType 套接字类型
     * @param  isNonblock 是否为非阻塞套接字
     * @param  fd 创建成功的套接字描述符
     */
    static bool CreateSocket(Addr_t addrType, Socket_t sockType, bool isNonblock, int& fd);

    /**
     * @brief  关闭套接字
     * @return 关闭结果
     * @param  fd 套接字描述符
     */
    static bool CloseSocket(int fd);

    /**
     * @brief  绑定套接字
     * @return 绑定结果
     * @param  fd 套接字描述符
     * @param  addr 地址
     */
    static bool BindSocket(int fd, const Address::Ptr& addr);

    /**
     * @brief  监听套接字
     * @return 监听结果
     * @param  fd 套接字描述符
     * @param  backlog 监听队列长度
     */
    static bool ListenSocket(int fd, int backlog = SOMAXCONN);

    /**
     * @note   仅tcp socket使用
     * @brief  接受连接
     * @return 接受连接结果
     * @param  fd 套接字描述符
     * @param  addr 地址
     * @param  connfd 连接套接字描述符
     */
    static bool AcceptSocket(int fd, const Address::Ptr& addr, int& connfd);

    /**
     * @brief  连接socket
     * @return 连接socket结果
     * @param  fd 套接字描述符
     * @param  peerAddr 对端地址
     */
    static bool ConnectSocket(int fd, const Address::Ptr& peerAddr);

    /**
     * @brief  关闭socket
     * @return 关闭socket结果
     * @param  fd 套接字描述符
     * @param  type 关闭类型
     */
    static bool ShutdownSocket(int fd, SocketShutdown_t type);

public:
    /**
     * @brief  获取socket地址族类型
     * @return 获取socket地址族类型结果
     * @param  fd 套接字描述符
     * @param  addrType 地址族类型
     */
    static bool GetSocketFamilyType(int fd, Addr_t& addrType);

    /**
     * @brief  判断本地地址是否有效
     * @return 判断结果
     * @param  fd 套接字描述符
     */
    static bool IsLocalAddrValid(int fd);

    /**
     * @brief  获取本地地址
     * @return 获取本地地址结果
     * @param  fd 套接字描述符
     * @param  ipAddr 本地ip地址
     */
    static bool GetLocalIpAddr(int fd, std::string& ipAddr);

    /**
     * @brief  获取本地端口
     * @return 获取本地端口结果
     * @param  fd 套接字描述符
     * @param  port 本地端口
     */
    static bool GetLocalPort(int fd, uint16_t& port);

    /**
     * @brief  获取本地地址和端口
     * @return 获取本地端口结果
     * @param  fd 套接字描述符
     * @param  ipAddr 本地ip地址
     * @param  port 本地端口
     */
    static bool GetLocalIpAddrPort(int fd, std::string& ipAddr, uint16_t& port);

    /**
     * @brief  判断对端地址是否有效
     * @return 判断结果
     * @param  fd 套接字描述符
     */
    static bool IsRemoteAddrValid(int fd);

    /**
     * @brief  获取对端地址
     * @return 获取对端地址结果
     * @param  fd 套接字描述符
     * @param  ipAddr 对端ip地址
     */
    static bool GetRemoteIpAddr(int fd, std::string& ipAddr);

    /**
     * @brief  获取对端端口
     * @return 获取对端端口结果
     * @param  fd 套接字描述符
     * @param  port 对端端口
     */
    static bool GetRemotePort(int fd, uint16_t& port);

    /**
     * @brief  获取对端地址和端口
     * @return 获取对端端口结果
     * @param  fd 套接字描述符
     * @param  ipAddr 对端ip地址
     * @param  port 对端端口
     */
    static bool GetRemoteIpAddrPort(int fd, std::string& ipAddr, uint16_t& port);

public:
    /**
     * @brief  设置套接字地址可重用
     * @return 设置结果
     * @param  fd 套接字描述符
     * @param  enabled 是否可重用
     */
    static bool SetReuseAddr(int fd, bool enabled);

    /**
     * @brief  获取套接字地址是否可重用
     * @return 判断结果
     * @param  fd 套接字描述符
     */
    static bool IsReuseAddr(int fd);

    /**
     * @brief  设置套接字端口可重用
     * @return 设置结果
     * @param  fd 套接字描述符
     * @param  enabled 是否可重用
     */
    static bool SetReusePort(int fd, bool enabled);

    /**
     * @brief  获取套接字端口是否可重用
     * @return 判断结果
     * @param  fd 套接字描述符
     */
    static bool IsReusePort(int fd);

    /**
     * @brief  设置套接字是否禁用 Nagle 算法
     * @return 设置结果
     * @param  fd 套接字描述符
     * @param  enabled 是否禁用
     */
    static bool SetNodelay(int fd, bool enabled);

    /**
     * @brief  设置套接字是否启用 keepalive
     * @return 禁用结果
     * @param  fd 套接字描述符
     * @param  enabled 是否启用
     */
    static bool SetKeepalive(int fd, bool enabled);

    /**
     * @brief  判断套接字是否为阻塞
     * @return 判断结果
     * @param  fd 套接字描述符
     */
    static bool IsBlocking(int fd);

    /**
     * @brief  设置套接字为阻塞
     * @return 设置结果
     * @param  fd 套接字描述符
     * @param  enabled 是否为阻塞
     */
    static bool SetBlocking(int fd, bool enabled);

    /**
     * @brief  判断套接字是否为cloexec
     * @return 判断结果
     * @param  fd 套接字描述符
     */
    static bool IsCloexec(int fd);

    /**
     * @brief  设置套接字为cloexec
     * @return 设置结果
     * @param  fd 套接字描述符
     * @param  enabled 是否为cloexec
     */
    static bool SetCloexec(int fd, bool enabled);

public:
    /**
     * @brief  获取套接字错误码
     * @return 错误码
     * @param  fd 套接字描述符
     */
    static int GetSocketError(int fd);

public:
    /**
     * @brief  读取数据
     * @return 读取数据结果
     * @param  fd 套接字描述符
     * @param  buf 缓冲区
     * @param  len 缓冲区长度
     */
    static ssize_t Read(int fd, void* buf, size_t len);

    /**
     * @brief  读取数据
     * @return 读取数据结果
     * @param  fd 套接字描述符
     * @param  iov 缓冲区数组
     * @param  iovcnt 缓冲区数组长度
     */
    static ssize_t Readv(int fd, const struct iovec* iov, int iovcnt);

    /**
     * @brief  写入数据
     * @return 写入数据结果
     * @param  fd 套接字描述符
     * @param  buf 缓冲区
     * @param  len 缓冲区长度
     */
    static ssize_t Write(int fd, const void* buf, size_t len);
};

}; // namespace Utils