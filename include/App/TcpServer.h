#pragma once
#include <map>
#include <atomic>
#include <memory>
#include "Utils/Utils.h"
#include "Net/Acceptor.h"
#include "Net/Connection.h"
#include "Thread/EventLoopThreadPool.h"
using namespace Net;
using namespace Utils;
using namespace Thread;

namespace App {

class TcpServer : public Noncopyable, public std::enable_shared_from_this<TcpServer> {
public:
    using Ptr = std::shared_ptr<TcpServer>;
    using WkPtr = std::weak_ptr<TcpServer>;
    using ConnCb = TcpConnection::ConnCb;
    using ReadableCb = TcpConnection::ReadableCb;
    using WriteableCb = TcpConnection::WriteableCb;
    // key = connection id, value = connection ptr
    using ConnectionMap = std::map<std::string, Connection::Ptr>;

public:
    explicit TcpServer(Address::Ptr addr, const ThreadInitCb& cb = nullptr, unsigned int numWorkThreads = 4, bool reuseport = true);
    ~TcpServer();

public:
    /**
     * @brief 启动服务
     */
    void run();

    /**
     * @brief 关闭服务
     */
    void shutdown() const;

public:
    /**
     * @brief 设置新连接回调函数
     * @param cb 回调函数
     */
    inline void setNewConnCb(const ConnCb& cb) {
        m_newConnCb = cb;
    }

    /**
     * @brief 设置数据读取回调函数
     * @param cb 回调函数
     */
    inline void setReadCb(const ReadableCb& cb) {
        m_readCb = cb;
    }

    /**
     * @brief 设置数据写入回调函数
     * @param cb 回调函数
     */
    inline void setWriteCb(const WriteableCb& cb) {
        m_writeCb = cb;
    }

private:
    /**
     * @brief 新连接处理函数
     * @param connSock 连接套接字
     * @param recvTime 接收时间
     */
    void onNewConnection(Socket::Ptr& connSock, Timestamp recvTime);

private:
    // 服务启动状态
    std::atomic_bool m_isStarted;

    // 本端地址
    Address::Ptr m_addr;

    // 事件循环线程池对象
    EventLoopThreadPool::Ptr m_workLoopThreadPool;

    // 接收器对象
    TcpAcceptor::Ptr m_acceptor;

    // 连接管理map
    ConnectionMap m_connMap;

    // 新连接回调函数
    ConnCb m_newConnCb;

    // 数据读取回调函数
    ReadableCb m_readCb;

    // 数据写入回调函数
    WriteableCb m_writeCb;
};

}; // namespace App