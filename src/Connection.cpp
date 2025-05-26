#include "Utils/Logger.h"
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

    // 创建输入输出缓冲区
    m_inBuf = std::make_shared<Buffer>();
    m_outBuf = std::make_shared<Buffer>();

    // 设置默认回调函数
    int fd = m_sock->getFd();
    std::string localAddrInfo = m_sock->getLocalAddr()->printIpPort();
    std::string remoteAddrInfo = m_sock->getRemoteAddr()->printIpPort();

    m_connCb = [fd, localAddrInfo, remoteAddrInfo](Connection::Ptr conn, bool isConn) {
        LOG_WARN << "Connection connection callback function not regist. fd: " << fd << " local addr: " << localAddrInfo
                 << " remote addr: " << remoteAddrInfo;
    };

    m_closeCb = [fd, localAddrInfo, remoteAddrInfo](Connection::Ptr conn) {
        LOG_WARN << "Connection close callback function not regist. fd: " << fd << " local addr: " << localAddrInfo
                 << " remote addr: " << remoteAddrInfo;
    };

    m_readCb = [fd, localAddrInfo, remoteAddrInfo](Connection::Ptr conn, Buffer::Ptr buf, Timestamp recvTime) {
        LOG_WARN << "Connection read callback function not regist. fd: " << fd << " local addr: " << localAddrInfo
                 << " remote addr: " << remoteAddrInfo;
    };

    m_writeCb = [fd, localAddrInfo, remoteAddrInfo](Connection::Ptr conn) {
        LOG_WARN << "Connection write callback function not regist. fd: " << fd << " local addr: " << localAddrInfo
                 << " remote addr: " << remoteAddrInfo;
    };
}

bool Connection::open() {
    if (ConnState_t::ConnStateClosed != m_connState) {
        LOG_WARN << "Connection open warning. connection already opened. fd: " << m_sock->getFd() << " local addr: "
                 << m_sock->getLocalAddr()->printIpPort() << " remote addr: " << m_sock->getRemoteAddr()->printIpPort();
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
    if (!m_channel->open(Event_t::EvTypeAll)) {
        LOG_ERROR << "Connection open error. open channel failed. fd: " << m_sock->getFd() << " local addr: "
                  << m_sock->getLocalAddr()->printIpPort() << " remote addr: " << m_sock->getRemoteAddr()->printIpPort();
        return false;
    }

    m_connState.store(ConnState_t::ConnStateOpened);
    return true;
}

bool Connection::close() {
    if (ConnState_t::ConnStateClosed == m_connState) {
        LOG_WARN << "Connection close warning. connection already closed. fd: " << m_sock->getFd() << " local addr: "
                 << m_sock->getLocalAddr()->printIpPort() << " remote addr: " << m_sock->getRemoteAddr()->printIpPort();
        return true;
    }

    m_connState.store(ConnState_t::ConnStateClosed);
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
}

void TcpConnection::handleRead(Timestamp recvTime) {

}

void TcpConnection::handleWrite(Timestamp recvTime) {

}

void TcpConnection::handleClose(Timestamp recvTime) {

}

void TcpConnection::handleError(Timestamp recvTime) {

}

} // namespace Net