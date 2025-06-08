#include "Utils/Logger.h"
#include "Net/EventLoop.h"
#include "App/TcpServer.h"

namespace App {

TcpServer::TcpServer(Address::Ptr addr, const ThreadInitCb& cb, unsigned int numWorkThreads, bool reuseport)
    : m_isStarted(false),
      m_addr(std::move(addr)),
      m_workLoopThreadPool(std::make_shared<EventLoopThreadPool>(numWorkThreads, cb)) {

    // 创建acceptor
    EventLoop::WkPtr mainLoop;
    m_workLoopThreadPool->getMainEventLoop(mainLoop);
    m_acceptor = std::make_shared<TcpAcceptor>(mainLoop, m_addr, reuseport);
}

TcpServer::~TcpServer() {
    if (m_isStarted) {
        this->shutdown();
    }
}

void TcpServer::run() {
    if (m_isStarted) {
        LOG_WARN << "Tcp server run warning. already started. server info: " << m_addr->printIpPort();
        return;
    }
    else {
        m_isStarted = true;
    }

    EventLoop::WkPtr mainLoop;
    m_workLoopThreadPool->getMainEventLoop(mainLoop);

    auto weakSelf = this->weak_from_this();
    mainLoop.lock()->executeTask([weakSelf]() {
        auto strongSelf = std::dynamic_pointer_cast<TcpServer>(weakSelf.lock());
        strongSelf->m_acceptor->setNewConnCb([weakSelf](Socket::Ptr& connSock, Timestamp recvTime) {
            auto strongSelf = weakSelf.lock();
            strongSelf->onNewConnection(connSock, recvTime);
        });

        strongSelf->m_acceptor->listen();
    });
}

void TcpServer::shutdown() {
    if (!m_isStarted) {
        LOG_WARN << "Tcp server shutdown warning. already shutdown. server info: " << m_addr->printIpPort();
        return;
    }
    else {
        m_isStarted = false;
    }

    // 关闭tcp服务管理的所有连接
    for (auto& pair : m_connMap) {
        auto conn = pair.second;
        conn->getOwnerLoop().lock()->executeTask([conn]() {
            conn->close(0);
        });
    }
    m_connMap.clear();
}

void TcpServer::onNewConnection(Socket::Ptr& connSock, Timestamp recvTime) {
    // 获取工作线程
    EventLoop::WkPtr workLoop;
    m_workLoopThreadPool->getNextWorkEventLoop(workLoop);

    // 保存新连接
    auto conn = std::make_shared<TcpConnection>(workLoop, connSock);
    m_connMap[conn->getConnectionId()] = conn;

    // 设置新连接回调函数
    conn->setConnectCallback(m_connCb);
    conn->setMessageCallback(m_readCb);
    conn->setWriteCompleteCallback(m_writeCb);

    EventLoop::WkPtr mainLoop;
    m_workLoopThreadPool->getMainEventLoop(mainLoop);

    auto weakSelf = this->weak_from_this();
    conn->setCloseCallback([weakSelf, mainLoop](const Connection::Ptr& conn)  {
        // 关闭连接
        conn->close(0);

        // 将connection移除放在主线程中执行
        std::string connId = conn->getConnectionId();
        mainLoop.lock()->executeTask([weakSelf, connId]() {
            weakSelf.lock()->m_connMap.erase(connId);
        });
    });

    // 启动新连接
    workLoop.lock()->executeTask([conn]() {
        conn->open();
    });
}

} // namespace App