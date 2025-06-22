#include "Utils/Logger.h"
#include "Utils/Socketop.h"
#include "Net/Channel.h"
#include "Net/EventLoop.h"
#include "Net/Connection.h"

namespace Net {

/** ---------------------------------------------- Connection ------------------------------------------------------ */

Connection::Connection(const EventLoopWkPtr& loop, const Socket::Ptr& sock)
    : m_sock(sock),
      m_channel(nullptr),
      m_ownerLoop(loop),
      m_connState(ConnState_t::ConnStateClosed) {

    if (loop.expired() || nullptr == sock || (nullptr != sock && (!sock->isLocalAddrValid() || !sock->isRemoteAddrValid()))) {
        LOG_FATAL << "Connection construct error. invalid input param.";
    }

    // 创建id
    std::string localIp;
    uint16_t localPort;
    sock->getLocalAddr()->getIpPort(localIp, localPort);

    std::string remoteIp;
    uint16_t remotePort;
    sock->getRemoteAddr()->getIpPort(remoteIp, remotePort);

    m_connId = StringHelper::GetUniqueId(localIp, localPort, remoteIp, remotePort);

    // 创建输入输出缓冲区
    m_inBuf = std::make_shared<Buffer>();
    m_outBuf = std::make_shared<Buffer>();

    // 设置默认回调函数
    m_connCb = [](const Connection::Ptr& conn, bool isConn) {
        LOG_WARN << "Connection connection callback function not regist. " << conn->getConnectionInfo();
    };

    m_readCb = [](const Connection::Ptr& conn, const Buffer::Ptr& buf, Timestamp recvTime) {
        buf->moveReadStartPos(buf->readableBytes());
        LOG_WARN << "Connection read callback function not regist. " << conn->getConnectionInfo();
    };
}

bool Connection::open() {
    if (ConnState_t::ConnStateClosed != m_connState) {
        LOG_WARN << "Connection open warning. connection already opened. " << this->getConnectionInfo();
        return true;
    }

    // 创建channel
    m_channel = std::make_shared<Channel>(m_ownerLoop, m_sock->getFd());

    // 设置channel事件回调
    auto weakSelf = this->weak_from_this();
    m_channel->setEventCb(Event_t::EvTypeRead, [weakSelf](Timestamp recvTime) {
        if (!weakSelf.expired()) {
            weakSelf.lock()->handleRead(recvTime);
        }
    });

    m_channel->setEventCb(Event_t::EvTypeWrite, [weakSelf](Timestamp recvTime) {
        if (!weakSelf.expired()) {
            weakSelf.lock()->handleWrite(recvTime);
        }
    });

    m_channel->setEventCb(Event_t::EvTypeClose, [weakSelf](Timestamp recvTime) {
        if (!weakSelf.expired()) {
            weakSelf.lock()->handleClose(recvTime);
        }
    });

    m_channel->setEventCb(Event_t::EvTypeError, [weakSelf](Timestamp recvTime) {
        if (!weakSelf.expired()) {
            weakSelf.lock()->handleError(recvTime);
        }
    });

    // 打开channel
    if (!m_channel->open(Event_t::EvTypeRead)) {
        LOG_ERROR << "Connection open error. open channel failed. " << this->getConnectionInfo();
        return false;
    }

    m_connState.store(ConnState_t::ConnStateConnected);
    return true;
}

bool Connection::reconnect() {
    if (ConnState_t::ConnStateConnected == m_connState) {
        // 连接已经建立
        LOG_WARN << "Connection connect warning. connection already connected. " << this->getConnectionInfo();
        return true;
    }

    // 执行连接建立回调函数
    if (nullptr != m_connCb && !m_ownerLoop.expired()) {
        auto loop = m_ownerLoop.lock();
        if (loop->isInCurrentThread()) {
            m_connCb(this->shared_from_this(), true);
        }
        else {
            auto weakSelf = this->weak_from_this();
            loop->executeTaskInLoop([weakSelf]() {
                if (!weakSelf.expired()) {
                    weakSelf.lock()->m_connCb(weakSelf.lock()->shared_from_this(), true);
                }
            });
        }
    }

    m_channel->setReadEnabled(true);
    m_connState.store(ConnState_t::ConnStateConnected);
    return true;
}

bool Connection::disconnect() {
    if (ConnState_t::ConnStateDisconnected == m_connState) {
        // 连接已经断开
        LOG_WARN << "Connection disconnect warning. connection already disconnected. " << this->getConnectionInfo();
        return true;
    }
    else if (ConnState_t::ConnStateConnected != m_connState) {
        // 连接未打开
        LOG_ERROR << "Connection disconnect error. connection not connected. " << this->getConnectionInfo();
        return false;
    }

    // 执行连接断开回调函数
    if (nullptr != m_connCb && !m_ownerLoop.expired()) {
        auto loop = m_ownerLoop.lock();
        if (loop->isInCurrentThread()) {
            m_connCb(this->shared_from_this(), false);
        }
        else {
            auto weakSelf = this->weak_from_this();
            loop->executeTaskInLoop([weakSelf]() {
                if (!weakSelf.expired()) {
                    weakSelf.lock()->m_connCb(weakSelf.lock()->shared_from_this(), false);
                }
            });
        }
    }

    m_channel->update(Event_t::EvTypeNone);
    m_connState.store(ConnState_t::ConnStateDisconnected);
    return true;
}

bool Connection::close(double delay) {
    if (ConnState_t::ConnStateClosed == m_connState) {
        LOG_WARN << "Connection close warning. connection already closed. " << this->getConnectionInfo();
        return true;
    }

    auto ownerLoop = m_ownerLoop.lock();
    if (delay > 0) {
        TimerId timerId;
        auto strongSelf = this->shared_from_this();
        ownerLoop->addTimerAfterSpecificTime(timerId, [strongSelf]() {
            strongSelf->m_connState = ConnState_t::ConnStateClosed;
            strongSelf->m_channel->close();
        }, delay);
    }
    else {
        if (!ownerLoop->isInCurrentThread()) {
            auto strongSelf = this->shared_from_this();
            ownerLoop->executeTaskInLoop([strongSelf]() {
                strongSelf->m_connState = ConnState_t::ConnStateClosed;
                strongSelf->m_channel->close();
            });
        }
        else {
            m_connState = ConnState_t::ConnStateClosed;
            m_channel->close();
        }
    }

    return true;
}

bool Connection::send(const void* data, std::size_t size) {
    if (ConnState_t::ConnStateConnected != m_connState) {
        // 连接未打开
        LOG_ERROR << "Connection write data error. connection not connected. " << this->getConnectionInfo();
        return false;
    }

    std::size_t writeSize = 0;
    std::size_t remainSize = size;
    std::size_t cachedSize = m_outBuf->readableBytes();

    // 写事件未打开，且输出缓冲区为空，直接写数据
    if (!m_channel->writeEnabled() && 0 == cachedSize) {
        writeSize = Socketop::Write(m_sock->getFd(), data, size);
        if (writeSize > 0) {
            // 写入成功, 调用回调函数
            remainSize -= writeSize;
            if (0 == remainSize && nullptr != m_writeCb) {
                if (nullptr != m_writeCb) {
                    auto weakSelf = this->weak_from_this();
                    m_ownerLoop.lock()->executeTask([weakSelf]() {
                        if (!weakSelf.expired()) {
                            auto strongSelf = weakSelf.lock();
                            strongSelf->m_writeCb(strongSelf);
                        }
                    });
                }
                return true;
            }
        }
        else {
            // 写入失败, 判断错误类型
            switch (errno) {
                case EWOULDBLOCK: {
                    LOG_WARN << "Connection write data warning. buffer blocked. " << this->getConnectionInfo();
                    break;
                }
                case EINTR:  {
                    LOG_WARN << "Connection write data warning. write event interrupted. " << this->getConnectionInfo();
                    break;
                }
                default: {
                    this->handleError(std::chrono::system_clock::now());
                    return false;
                }
            }

            writeSize = 0;
        }
    }

    // 数据写入缓存
    m_outBuf->write(static_cast<const uint8_t*>(data) + writeSize, size);
    if (!m_channel->writeEnabled()) {
        m_channel->setWriteEnabled(true);
    }

    return true;
}

std::string Connection::getConnectionInfo() const {
    if (nullptr == m_sock) {
        return "";
    }

    std::stringstream ss;
    ss << "fd: " << m_sock->getFd() << " local addr: " << m_sock->getLocalAddr()->printIpPort()
       << " remote addr: " << m_sock->getRemoteAddr()->printIpPort();
    return ss.str();
}

bool Connection::enableRead() {
    if (ConnState_t::ConnStateConnected != m_connState) {
        LOG_ERROR << "Connection enable read error. connection not opened. " << this->getConnectionInfo();
        return false;
    }

    if (m_channel->readEnabled()) {
        LOG_WARN << "Connection enable read warning. read event already enabled. " << this->getConnectionInfo();
        return true;
    }

    auto loop = m_ownerLoop.lock();
    if (loop->isInCurrentThread()) {
        m_channel->setReadEnabled(true);
    }
    else {
        auto weakSelf = this->weak_from_this();
        loop->executeTaskInLoop([weakSelf]() {
            if (!weakSelf.expired()) {
                weakSelf.lock()->m_channel->setReadEnabled(true);
            }
        });
    }

    return true;
}

bool Connection::disableRead() {
    if (ConnState_t::ConnStateConnected != m_connState) {
        LOG_ERROR << "Connection disable read error. connection not opened. " << this->getConnectionInfo();
        return false;
    }

    if (!m_channel->readEnabled()) {
        LOG_WARN << "Connection disable read warning. read event already disabled. " << this->getConnectionInfo();
        return true;
    }

    auto loop = m_ownerLoop.lock();
    if (loop->isInCurrentThread()) {
        m_channel->setReadEnabled(false);
    }
    else {
        auto weakSelf = this->weak_from_this();
        loop->executeTaskInLoop([weakSelf]() {
            if (!weakSelf.expired()) {
                weakSelf.lock()->m_channel->setReadEnabled(false);
            }
        });
    }
    return true;
}

/** ---------------------------------------------- TcpConnection ------------------------------------------------------ */

TcpConnection::TcpConnection(const EventLoopWkPtr& loop, const Socket::Ptr& sock)
    : Connection(loop, sock),
      m_highWaterMark(64 * 1024 * 1024) {

    if (Socket_t::TCP != m_sock->getType()) {
        LOG_FATAL << "TcpConnection construct error. invalid socket type. fd: " << m_sock->getFd();
    }

    // 设置tcp socket属性
    m_sock->setKeepaliveEnabled(true);
    LOG_DEBUG << "TcpConnection construct. " << this->getConnectionInfo();
}

TcpConnection::~TcpConnection() {
    LOG_DEBUG << "TcpConnection deconstruct. " << this->getConnectionInfo();
}

bool TcpConnection::send(const void* data, std::size_t size) {
    if (ConnState_t::ConnStateConnected != m_connState) {
        // 连接未打开
        LOG_ERROR << "Connection write data error. connection not connected. " << this->getConnectionInfo();
        return false;
    }

    std::size_t writeSize = 0;
    std::size_t remainSize = size;
    std::size_t cachedSize = m_outBuf->readableBytes();

    // 写事件未打开，且输出缓冲区为空，直接写数据
    if (!m_channel->writeEnabled() && 0 == cachedSize) {
        writeSize = Socketop::Write(m_sock->getFd(), data, size);
        if (writeSize > 0) {
            remainSize -= writeSize;
            if (0 == remainSize) {
                // 写入成功, 调用回调函数
                if (nullptr != m_writeCb) {
                    auto weakSelf = this->weak_from_this();
                    m_ownerLoop.lock()->executeTask([weakSelf]() {
                        if (!weakSelf.expired()) {
                            auto strongSelf = std::dynamic_pointer_cast<TcpConnection>(weakSelf.lock());
                            strongSelf->m_writeCb(strongSelf);
                        }
                    });
                }
                return true;
            }
        }
        else {
            // 写入失败, 判断错误类型
            switch (errno) {
                case EWOULDBLOCK: {
                    LOG_WARN << "Connection write data warning. buffer blocked. " << this->getConnectionInfo();
                    break;
                }
                case EINTR:  {
                    LOG_WARN << "Connection write data warning. write event interrupted. " << this->getConnectionInfo();
                    break;
                }
                default: {
                    this->handleError(std::chrono::system_clock::now());
                    return false;
                }
            }
            writeSize = 0;
        }
    }

    // 调用高水位回调函数
    auto waitWriteSize = cachedSize + remainSize;
    if (waitWriteSize >= m_highWaterMark && cachedSize < m_highWaterMark && nullptr != m_highWaterMarkCb) {
        auto weakSelf = this->weak_from_this();
        m_ownerLoop.lock()->executeTask([weakSelf, waitWriteSize]() {
            if (!weakSelf.expired()) {
                auto strongSelf = std::dynamic_pointer_cast<TcpConnection>(weakSelf.lock());
                strongSelf->m_highWaterMarkCb(strongSelf, waitWriteSize);
            }
        });
    }

    // 剩余未未写入数据写入缓存
    m_outBuf->write(static_cast<const uint8_t*>(data) + writeSize, size);
    if (!m_channel->writeEnabled()) {
        m_channel->setWriteEnabled(true);
    }
    return true;
}

bool TcpConnection::shutdown() {
    if (ConnState_t::ConnStateDisconnected == m_connState) {
        // 连接已经断开
        LOG_WARN << "TcpConnection shutdown warning. connection already shutdown. " << this->getConnectionInfo();
        return true;
    }
    else if (ConnState_t::ConnStateConnected != m_connState) {
        // 连接未打开
        LOG_ERROR << "TcpConnection shutdown error. connection not connected. " << this->getConnectionInfo();
        return false;
    }

    auto weakSelf = this->weak_from_this();
    auto shutdownCb = [weakSelf]() {
        if (!weakSelf.expired()) {
            auto strongSelf = std::dynamic_pointer_cast<TcpConnection>(weakSelf.lock());
            strongSelf->m_sock->shutdown(SocketShutdown_t::ShutdownWrite);
        }
    };

    auto loop = m_ownerLoop.lock();
    if (loop->isInCurrentThread()) {
        shutdownCb();
    }
    else {
        loop->executeTaskInLoop([shutdownCb]() {
            shutdownCb();
        });
    }

    m_connState.store(ConnState_t::ConnStateDisconnected);
    return true;
}

void TcpConnection::handleRead(Timestamp recvTime) {
    int errCode = 0;
    auto readSize= m_inBuf->readFd(m_sock->getFd(), errCode);

    if (readSize > 0) {
        // 调用读回调函数
        m_readCb(this->shared_from_this(), m_inBuf, recvTime);
    }
    else if (0 == readSize) {
        // 读到0字节，可能是对端关闭了连接
        LOG_WARN << "TcpConnection handleRead warning. read 0 bytes. " << this->getConnectionInfo() << " error: " << errCode;
        this->handleClose(recvTime);
    }
    else {
        // 读取数据失败
        LOG_ERROR << "TcpConnection handleRead error. read failed. " << this->getConnectionInfo() << " error: " << errCode;
        this->handleError(recvTime);
    }
}

void TcpConnection::handleWrite(Timestamp recvTime) {
    if (!m_channel->writeEnabled()) {
        LOG_WARN << "TcpConnection handleWrite warning. write event disabled. " << this->getConnectionInfo();
        return;
    }

    int errCode = 0;
    ssize_t writeSize = m_outBuf->writeFd(m_sock->getFd(), errCode);
    if (writeSize > 0) {
        if (0 == m_outBuf->readableBytes()) {
            // 写完数据后，关闭写事件
            m_channel->setWriteEnabled(false);

            // 调用写回调函数
            if (nullptr != m_writeCb) {
                auto weakSelf = this->weak_from_this();
                m_ownerLoop.lock()->executeTask([weakSelf]() {
                    if (!weakSelf.expired()) {
                        auto strongSelf = std::dynamic_pointer_cast<TcpConnection>(weakSelf.lock());
                        strongSelf->m_writeCb(strongSelf);
                    }
                });
            }

            // 关闭写事件
            if (ConnState_t::ConnStateDisconnected == m_connState) {
                m_sock->shutdown(SocketShutdown_t::ShutdownWrite);
            }
        }
    }
}

void TcpConnection::handleClose(Timestamp recvTime) {
    LOG_INFO << "TcpConnection handleClose. " << this->getConnectionInfo();

    auto weakSelf = this->weak_from_this();
    m_ownerLoop.lock()->executeTask([weakSelf]() {
        if (!weakSelf.expired()) {
            auto strongSelf = std::dynamic_pointer_cast<TcpConnection>(weakSelf.lock());

            if (nullptr != strongSelf->m_connCb) {
                strongSelf->m_connCb(strongSelf, false);
            }

            if (nullptr!= strongSelf->m_closeCb) {
                strongSelf->m_closeCb(strongSelf);
            }
        }
    });
}

void TcpConnection::handleError(Timestamp recvTime) {
    LOG_ERROR << "TcpConnection handleError. " << this->getConnectionInfo() << " error: " << Socketop::GetSocketError(m_sock->getFd());
}

} // namespace Net