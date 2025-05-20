#include <cerrno>
#include <cstring>
#include <utility>
#include <fcntl.h>
#include <unistd.h>
#include "Utils/Logger.h"
#include "Utils/Socketop.h"
#include "Net/Channel.h"
#include "Net/EventLoop.h"
#include "Net/Acceptor.h"

namespace Net {

/** ---------------------------------------------- Acceptor ------------------------------------------------------ */

Acceptor::Acceptor(EventLoop::WkPtr loop, const Address::Ptr& addr, const Socket_t& type, bool reuseport)
    : m_idleFd(::open("/dev/null", O_RDONLY | O_CLOEXEC)),
      m_isListening(false),
      m_addr(addr),
      m_ownerLoop(std::move(loop)) {
    // 创建监听套接字
    int fd = -1;
    if (!Socketop::CreateSocket(addr->getAddrType(), type, true, fd)) {
        LOG_FATAL << "Acceptor construct error. create socket failed. addr: " << addr->printIpPort();
    }

    m_sock = std::make_shared<Socket>(fd, type);
    m_sock->setReuseAddr(true);
    m_sock->setReusePort(reuseport);
    m_sock->bind(addr);

    // 创建channel
    m_channel = std::make_shared<Channel>(m_ownerLoop, fd);
}

Acceptor::~Acceptor() {
    m_channel->close();
    ::close(m_idleFd);
}

void Acceptor::listen() {
    if (this->isListening()) {
        LOG_WARN << "Acceptor listen warning. listen already. addr: " << m_addr->printIpPort();
        return;
    }

    if (!m_sock->listen()) {
        LOG_ERROR << "Acceptor listen error. listen failed. addr: " << m_addr->printIpPort();
        return;
    }

    auto weakSelf = this->weak_from_this();
    m_channel->setEventCb(Event_t::EvTypeRead, [weakSelf](Timestamp recvTime) {
        if (weakSelf.expired()) {
            LOG_ERROR << "Acceptor listen error. acceptor expired.";
            return;
        }

        weakSelf.lock()->handleRead(recvTime);
    });

    m_channel->open(Event_t::EvTypeRead);
    m_isListening = true;
}

/** ---------------------------------------------- TcpAcceptor ------------------------------------------------------ */

TcpAcceptor::TcpAcceptor(const EventLoop::WkPtr& loop, const Address::Ptr& addr, bool reuseport)
    : Acceptor(loop, addr, Socket_t::TCP, reuseport) {
}

void TcpAcceptor::handleRead(Timestamp recvTime) {
    if (!this->isListening() || nullptr == m_newConnCb) {
        LOG_ERROR << "TcpAcceptor handleRead error. acceptor not listening or new connection callback not set.";
        return;
    }

    Socket::Ptr connSock;
    if (!m_sock->accept(connSock)) {
        LOG_ERROR << "TcpAcceptor handleRead error. accept failed. addr: " << m_addr->printIpPort() << " errno: "
                  << errno << " error: " << strerror(errno);

        // 连接数量达到上限
        if (EMFILE == errno) {
            // 关闭连接socket
            ::close(m_idleFd);
            m_idleFd = ::accept(m_sock->getFd(), nullptr, nullptr);

            // 重置空闲文件描述符
            ::close(m_idleFd);
            m_idleFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
    else {
        // 新连接回调
        m_newConnCb(connSock, recvTime);
    }
}

} // namespace Net