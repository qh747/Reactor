#include "Utils/Logger.h"
#include "Utils/Socketop.h"
#include "Net/Socket.h"

namespace Net {

/** ---------------------------------------------- Socket ------------------------------------------------------ */

Socket::Socket(int fd, const Socket_t& type)
    : m_fd(fd), m_type(type) {
}

Socket::~Socket() {
    Socketop::CloseSocket(m_fd);
}

bool Socket::bind(const Address::Ptr& addr) {
    // 输入参数校验
    if (nullptr == addr || !addr->valid()) {
        LOG_ERROR << "Socket bind address error. invalid address.";
        return false;
    }

    // socket地址已绑定
    if (this->isLocalAddrValid()) {
        LOG_WARN << "Socket bind address warning. address already bound. new addr: " << addr->printIpPort()
            << " old addr: " << m_localAddr->printIpPort();
        return true;
    }

    if (!Socketop::BindSocket(m_fd, addr)) {
        LOG_ERROR << "Socket bind address error. addr: " << addr->printIpPort();
        return false;
    }

    m_localAddr = addr;
    return true;
}

bool Socket::listen() const {
    if (!this->isLocalAddrValid()) {
        LOG_ERROR << "Socket listen error. local address not set.";
        return false;
    }

    if (!Socketop::ListenSocket(m_fd)) {
        LOG_ERROR << "Socket listen error. addr: " << m_localAddr->printIpPort();
        return false;
    }
    return true;
}

bool Socket::accept(Socket::Ptr& connSock) const {
    if (!this->isLocalAddrValid()) {
        LOG_ERROR << "Socket accept error. local address not set.";
        return false;
    }
    else if (Socket_t::TCP != m_type) {
        LOG_ERROR << "Socket accept error. invalid socket type.";
        return false;
    }

    int connfd = 0;
    Address::Ptr peerAddr;
    if (!Socketop::AcceptSocket(m_fd, peerAddr, connfd)) {
        LOG_ERROR << "Socket accept error. addr: " << m_localAddr->printIpPort();
        return false;
    }

    // 创建socket
    connSock = std::make_shared<Socket>(connfd, m_type);
    connSock->m_localAddr = this->m_localAddr;
    connSock->m_peerAddr = peerAddr;

    // 设置socket属性
    Socketop::SetCloexec(connfd, Socketop::IsCloexec(m_fd));
    Socketop::SetBlocking(connfd, Socketop::IsBlocking(m_fd));
    return true;
}

bool Socket::connect(const Address::Ptr& peerAddr) {
    if (nullptr == peerAddr || !peerAddr->valid()) {
        LOG_ERROR << "Socket connect error. invalid peer address.";
        return false;
    }

    if (!Socketop::ConnectSocket(m_fd, peerAddr)) {
        LOG_ERROR << "Socket connect error. remote addr: " << peerAddr->printIpPort();
        return false;
    }

    m_peerAddr = peerAddr;
    return true;
}

void Socket::setReuseAddr(bool enabled) const {
    if (!Socketop::SetReuseAddr(m_fd, enabled)) {
        LOG_ERROR << "Socket set reuse addr error. fd: " << m_fd;
    }
}

bool Socket::isReuseAddr() const {
    return Socketop::IsReuseAddr(m_fd);
}

void Socket::setReusePort(bool enabled) const {
    if (!Socketop::SetReusePort(m_fd, enabled)) {
        LOG_ERROR << "Socket set reuse port error. fd: " << m_fd;
    }
}

bool Socket::isReusePort() const {
    return Socketop::IsReusePort(m_fd);
}

void Socket::shutdown(SocketShutdown_t type) const {
    if (Socket_t::TCP != m_type) {
        LOG_ERROR << "Socket shutdown error. invalid socket type. fd: " << m_fd;
    }
    else if (!Socketop::ShutdownSocket(m_fd, type)) {
        LOG_ERROR << "Socket shutdown error. fd: " << m_fd;
    }
}

void Socket::setNoDelayEnabled(bool enabled) const {
    if (Socket_t::TCP != m_type) {
        LOG_ERROR << "Socket set no delay enabled error. invalid socket type. fd: " << m_fd;
        return;
    }

    if (!Socketop::SetNodelay(m_fd, enabled)) {
        LOG_ERROR << "Socket set no delay enabled error. fd: " << m_fd;
    }
}

void Socket::setKeepaliveEnabled(bool enabled) const {
    if (Socket_t::TCP != m_type) {
        LOG_ERROR << "Socket set keepalive enabled error. invalid socket type. fd: " << m_fd;
        return;
    }

    if (!Socketop::SetKeepalive(m_fd, enabled)) {
        LOG_ERROR << "Socket set keepalive enabled error. fd: " << m_fd;
    }
}

} // namespace Net