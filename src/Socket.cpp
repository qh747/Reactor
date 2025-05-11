#include <unistd.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include "Utils/Logger.h"
#include "Net/Socket.h"

namespace Net {

/** ---------------------------------------------- Socket ------------------------------------------------------ */

Socket::Socket(int fd, const Socket_t& type)
    : m_fd(fd), m_type(type) {
}

Socket::~Socket() {
    ::close(m_fd);
}

bool Socket::bind(const Address::Ptr& addr) {
    // 输入参数校验
    if (nullptr == addr || !addr->valid()) {
        LOG_ERROR << "Socket bind address error. invalid address.";
        return false;
    }

    std::string ipAddr;
    addr->getIpAddr(ipAddr);

    uint16_t port;
    addr->getPort(port);

    // socket地址已绑定
    if (this->isLocalAddrValid()) {
        std::string oldIpAddr;
        m_localAddr->getIpAddr(oldIpAddr);

        uint16_t oldPort;
        m_localAddr->getPort(oldPort);

        LOG_WARN << "Socket bind address warning. address already bound. new ip addr: " << ipAddr << " port: " << port
                 << " old ip addr: " << oldIpAddr << " port: " << oldPort;
        return true;
    }

    sockaddr_in sockAddr = {};
    addr->getSockAddr(sockAddr);
    if (::bind(m_fd, reinterpret_cast<const sockaddr*>(&sockAddr), static_cast<socklen_t>(sizeof(struct sockaddr_in))) < 0) {
        LOG_ERROR << "Socket bind address error. ip addr: " << ipAddr << " port: " << port;
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

    if (::listen(m_fd, SOMAXCONN) < 0) {
        std::string ipAddr;
        m_localAddr->getIpAddr(ipAddr);

        uint16_t port;
        m_localAddr->getPort(port);

        LOG_ERROR << "Socket listen error. ip addr: " << ipAddr << " port: " << port;
        return false;
    }
    return true;
}

void Socket::setReuseAddr(bool enabled) const {
    int optval = enabled ? 1 : 0;
    ::setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

void Socket::setReusePort(bool enabled) const {
#ifdef SO_REUSEPORT
    int optval = enabled ? 1 : 0;
    ::setsockopt(m_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
#endif
}

/** ---------------------------------------------- TcpSocket ------------------------------------------------------ */

TcpSocket::TcpSocket(int fd)
    : Socket(fd, Socket_t::SocketTCP) {
}

bool TcpSocket::accept(Socket::Ptr& peerSock) const {
    if (!this->isLocalAddrValid()) {
        LOG_ERROR << "Socket accept error. local address not set.";
        return false;
    }

    sockaddr_in sockAddr = {};
    if (!m_localAddr->getSockAddr(sockAddr)) {
        LOG_ERROR << "Socket accept error. invalid local address.";
        return false;
    }

    socklen_t sockLen = sizeof(sockAddr);
    int fd = ::accept(m_fd, reinterpret_cast<sockaddr*>(&sockAddr), &sockLen);
    if (fd < 0) {
        std::string ipAddr;
        m_localAddr->getIpAddr(ipAddr);

        uint16_t port;
        m_localAddr->getPort(port);

        LOG_ERROR << "Socket accept error. ip addr: " << ipAddr << " port: " << port;
        return false;
    }

    peerSock = std::make_shared<Socket>(fd, m_type);
    return true;
}

bool TcpSocket::connect(const Address::Ptr& peerAddr) {
    if (nullptr == peerAddr || !peerAddr->valid()) {
        LOG_ERROR << "Socket connect error. invalid peer address.";
        return false;
    }

    sockaddr_in sockAddr = {};
    if (!peerAddr->getSockAddr(sockAddr)) {
        LOG_ERROR << "Socket connect error. invalid peer address.";
        return false;
    }

    socklen_t sockLen = sizeof(sockAddr);
    if (::connect(m_fd, reinterpret_cast<const sockaddr*>(&sockAddr), sockLen) < 0) {
        std::string ipAddr;
        peerAddr->getIpAddr(ipAddr);

        uint16_t port;
        peerAddr->getPort(port);
        LOG_ERROR << "Socket connect error. remote ip addr: " << ipAddr << " port: " << port;
        return false;
    }

    m_peerAddr = peerAddr;
    return true;
}

void TcpSocket::shutdown(int how) const {
    ::shutdown(m_fd, how);
}

void TcpSocket::setNoDelayEnabled(bool enabled) const {
    int optval = enabled ? 1 : 0;
    ::setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof(optval)));
}

void TcpSocket::setKeepaliveEnabled(bool enabled) const {
    int optval = enabled ? 1 : 0;
    ::setsockopt(m_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval));
}

/** ---------------------------------------------- UdpSocket ------------------------------------------------------ */

UdpSocket::UdpSocket(int fd)
    : Socket(fd, Socket_t::SocketUDP) {
}

bool UdpSocket::bindRemoteSock(const Address::Ptr& peerAddr) {
    if (nullptr == peerAddr || !peerAddr->valid()) {
        LOG_ERROR << "Socket bind remote address error. invalid peer address.";
        return false;
    }

    sockaddr_in sockAddr = {};
    if (!peerAddr->getSockAddr(sockAddr)) {
        LOG_ERROR << "Socket bind remote address error. invalid peer address.";
        return false;
    }

    socklen_t sockLen = sizeof(sockAddr);
    if (::connect(m_fd, reinterpret_cast<const sockaddr*>(&sockAddr), sockLen) < 0) {
        std::string ipAddr;
        peerAddr->getIpAddr(ipAddr);

        uint16_t port;
        peerAddr->getPort(port);
        LOG_ERROR << "Socket bind remote address error. remote ip addr: " << ipAddr << " port: " << port;
        return false;
    }

    m_peerAddr = peerAddr;
    return true;
}

} // namespace Net