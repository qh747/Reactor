#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include "Utils/Logger.h"
#include "Utils/Socketop.h"

namespace Utils {

bool Socketop::CreateSocket(Addr_t addrType, Socket_t sockType, bool isNonblock, int& fd) {
    int type = isNonblock ? (static_cast<int>(sockType) | SOCK_NONBLOCK | SOCK_CLOEXEC) : static_cast<int>(sockType) | SOCK_CLOEXEC;
    int protocol = Socket_t::TCP == sockType ? IPPROTO_TCP : IPPROTO_UDP;

    fd = ::socket(static_cast<int>(addrType), type, protocol);
    if (fd < 0) {
        LOG_ERROR << "Socket create error, errno: " << errno << ". error: " << strerror(errno);
        return false;
    }
    return true;
}

bool Socketop::CloseSocket(int fd) {
    if (::close(fd) < 0) {
        LOG_ERROR << "Socket close error. errno: " << errno << ". error: " << strerror(errno);
        return false;
    }
    return true;
}

bool Socketop::BindSocket(int fd, const Address::Ptr& addr) {
    if (fd < 0 || !addr->valid()) {
        LOG_ERROR << "Socket bind address error. invalid input param: " << fd;
        return false;
    }

    sockaddr sockAddr = {};
    addr->getSockAddr(sockAddr);
    if (::bind(fd, &sockAddr, static_cast<socklen_t>(sizeof(struct sockaddr_in))) < 0) {
        LOG_ERROR << "Socket bind address error. fd: " << fd << " errno: " << errno << ". error: " << strerror(errno);
        return false;
    }
    return true;
}

bool Socketop::ListenSocket(int fd, int backlog) {
    if (fd < 0) {
        LOG_ERROR << "Socket listen error. invalid input param: " << fd;
        return false;
    }

    if (::listen(fd, backlog) < 0) {
        LOG_ERROR << "Socket listen error. fd: " << fd << " errno: " << errno << ". error: " << strerror(errno);
        return false;
    }
    return true;
}

bool Socketop::AcceptSocket(int fd, const Address::Ptr& addr, int& connfd) {
    if (fd < 0 || !addr->valid()) {
        LOG_ERROR << "Socket accept error. invalid input param: " << fd;
        return false;
    }

    sockaddr sockAddr = {};
    addr->getSockAddr(sockAddr);
    socklen_t sockLen = sizeof(sockAddr);

    if ((connfd = ::accept(fd, &sockAddr, &sockLen) < 0)) {
        LOG_ERROR << "Socket accept error. fd: " << fd << " errno: " << errno << ". error: " << strerror(errno);
        ;
        return false;
    }
    return true;
}

bool Socketop::ConnectSocket(int fd, const Address::Ptr& peerAddr) {
    if (fd < 0 || !peerAddr->valid()) {
        LOG_ERROR << "Socket connect error. invalid input param: " << fd;
        return false;
    }

    sockaddr sockAddr = {};
    peerAddr->getSockAddr(sockAddr);

    if (::connect(fd, &sockAddr, sizeof(sockAddr)) < 0) {
        LOG_ERROR << "Socket connect error. fd: " << fd << " errno: " << errno << ". error: " << strerror(errno);
        return false;
    }
    return true;
}

bool Socketop::ShutdownSocket(int fd, SocketShutdown_t type) {
    if (fd < 0) {
        LOG_ERROR << "Socket shutdown error. invalid input param: " << fd;
        return false;
    }

    if (::shutdown(fd, static_cast<int>(type)) < 0) {
        LOG_ERROR << "Socket shutdown error. fd: " << fd << " errno: " << errno << ". error: " << strerror(errno);
        return false;
    }
    return true;
}

bool Socketop::GetSocketFamilyType(int fd, Addr_t& addrType) {
    sockaddr_storage addr;
    socklen_t len = sizeof(addr);

    if (-1 == ::getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &len)) {
        LOG_ERROR << "Socket get family type error. fd: " << fd << " errno: " << errno << ". error: " << strerror(errno);
        return false;
    }

    addrType = addr.ss_family == AF_INET ? Addr_t::IPv4 : Addr_t::IPv6;
    return true;
}

bool Socketop::IsLocalAddrValid(int fd) {
    if (fd < 0) {
        LOG_ERROR << "Socket check local address validity error. invalid input param: " << fd;
        return false;
    }

    sockaddr_storage addr;
    socklen_t len = sizeof(addr);

    if (::getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &len) == -1) {
        LOG_ERROR << "Socket check local address validity error. fd: " << fd << " errno: " << errno << ". error: " << strerror(errno);
        return false;
    }

    if (addr.ss_family == AF_INET) {
        sockaddr_in* ipv4Addr = reinterpret_cast<sockaddr_in*>(&addr);
        if (ipv4Addr->sin_addr.s_addr == INADDR_ANY || ipv4Addr->sin_addr.s_addr == INADDR_LOOPBACK) {
            return false;
        }
    }
    else if (addr.ss_family == AF_INET6) {
        sockaddr_in6* ipv6Addr = reinterpret_cast<sockaddr_in6*>(&addr);
        if (IN6_ARE_ADDR_EQUAL(&ipv6Addr->sin6_addr, &in6addr_any) || IN6_IS_ADDR_LOOPBACK(&ipv6Addr->sin6_addr)) {
            return false;
        }
    }
    else {
        LOG_ERROR << "Socket check local address validity error. Unknown address family. fd: " << fd;
        return false;
    }

    return true;
}

bool Socketop::GetLocalIpAddr(int fd, std::string& ipAddr) {
    if (fd < 0) {
        LOG_ERROR << "Socket get local ip addr error. invalid input param: " << fd;
        return false;
    }

    sockaddr_storage addr;
    socklen_t len = sizeof(addr);

    if (::getsockname(fd, reinterpret_cast<struct sockaddr*>(&addr), &len) == -1) {
        LOG_ERROR << "Socket get local ip addr error. fd: " << fd << " errno: " << errno << ". error: " << strerror(errno);
        return false;
    }

    char ip[INET6_ADDRSTRLEN];
    void* srcAddr = nullptr;
    int addrFamily = 0;

    if (addr.ss_family == AF_INET) {
        srcAddr = &(reinterpret_cast<sockaddr_in*>(&addr)->sin_addr);
        addrFamily = AF_INET;
    }
    else if (AF_INET6 == addr.ss_family) {
        srcAddr = &(reinterpret_cast<sockaddr_in6*>(&addr)->sin6_addr);
        addrFamily = AF_INET6;
    }
    else {
        LOG_ERROR << "Socket get local ip addr error. Unknown address family. fd: " << fd;
        return false;
    }

    if (::inet_ntop(addrFamily, srcAddr, ip, INET6_ADDRSTRLEN) == nullptr) {
        LOG_ERROR << "Socket get local ip addr error. errno: " << errno << ". error: " << strerror(errno);
        return false;
    }

    ipAddr = ip;
    return true;
}

bool Socketop::GetLocalPort(int fd, uint16_t& port) {
    if (fd < 0) {
        LOG_ERROR << "Socket get local port error. invalid input param: " << fd;
        return false;
    }

    sockaddr_storage addr;
    socklen_t len = sizeof(addr);

    if (::getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &len) == -1) {
        LOG_ERROR << "Socket get local port error. fd: " << fd << " errno: " << errno << ". error: " << strerror(errno);
        return false;
    }

    if (AF_INET == addr.ss_family) {
        port = ::ntohs(reinterpret_cast<sockaddr_in*>(&addr)->sin_port);
    }
    else if (AF_INET6 == addr.ss_family) {
        port = ::ntohs(reinterpret_cast<sockaddr_in6*>(&addr)->sin6_port);
    }
    else {
        LOG_ERROR << "Socket get local port error. Unknown address family. fd: " << fd;
        return false;
    }

    return true;
}

bool Socketop::GetLocalIpAddrPort(int fd, std::string& ipAddr, uint16_t& port) {
    if (fd < 0) {
        LOG_ERROR << "Socket get local ip addr and port error. invalid input param: " << fd;
        return false;
    }

    sockaddr_storage addr;
    socklen_t len = sizeof(addr);

    if (::getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &len) == -1) {
        LOG_ERROR << "Socket get local ip addr and port error. fd: " << fd << " errno: " << errno << ". error: " << strerror(errno);
        return false;
    }

    char ip[INET6_ADDRSTRLEN];
    void* srcAddr = nullptr;
    int addrFamily = 0;

    if (addr.ss_family == AF_INET) {
        srcAddr = &reinterpret_cast<sockaddr_in*>(&addr)->sin_addr;
        addrFamily = AF_INET;
        port = ::ntohs(reinterpret_cast<sockaddr_in*>(&addr)->sin_port);
    }
    else if (addr.ss_family == AF_INET6) {
        srcAddr = &reinterpret_cast<sockaddr_in6*>(&addr)->sin6_addr;
        addrFamily = AF_INET6;
        port = ::ntohs(reinterpret_cast<sockaddr_in6*>(&addr)->sin6_port);
    }
    else {
        LOG_ERROR << "Socket get local ip addr and port error. Unknown address family. fd: " << fd;
        return false;
    }

    if (::inet_ntop(addrFamily, srcAddr, ip, INET6_ADDRSTRLEN) == nullptr) {
        LOG_ERROR << "Socket get local ip addr and port error. errno: " << errno << ". error: " << strerror(errno);
        return false;
    }

    ipAddr = ip;
    return true;
}

bool Socketop::IsRemoteAddrValid(int fd) {
    if (fd < 0) {
        LOG_ERROR << "Socket check remote address validity error. invalid input param: " << fd;
        return false;
    }

    sockaddr_storage addr;
    socklen_t len = sizeof(addr);

    if (::getpeername(fd, reinterpret_cast<sockaddr*>(&addr), &len) == -1) {
        LOG_ERROR << "Socket check remote address validity error. fd: " << fd
                  << " errno: " << errno << ". error: " << strerror(errno);
        return false;
    }

    if (addr.ss_family == AF_INET) {
        sockaddr_in* ipv4Addr = reinterpret_cast<sockaddr_in*>(&addr);

        // 检查地址是否有效（非0.0.0.0且非回环地址127.0.0.1）
        if (ipv4Addr->sin_addr.s_addr == INADDR_ANY ||
            ipv4Addr->sin_addr.s_addr == htonl(INADDR_LOOPBACK)) {
            return false;
        }

        return true;
    }
    else if (addr.ss_family == AF_INET6) {
        sockaddr_in6* ipv6Addr = reinterpret_cast<sockaddr_in6*>(&addr);

        // 检查地址是否有效（非::且非::1）
        if (IN6_IS_ADDR_UNSPECIFIED(&ipv6Addr->sin6_addr) ||
            IN6_IS_ADDR_LOOPBACK(&ipv6Addr->sin6_addr)) {
            return false;
        }

        return true;
    }

    // 不支持其他地址族
    LOG_ERROR << "Socket check remote address validity error. Unknown address family. fd: " << fd;
    return false;
}

bool Socketop::GetRemoteIpAddr(int fd, std::string& ipAddr) {
    if (fd < 0) {
        LOG_ERROR << "Socket get remote ip addr error. invalid input param: " << fd;
        return false;
    }

    sockaddr_storage addr;
    socklen_t len = sizeof(addr);

    if (::getpeername(fd, reinterpret_cast<sockaddr*>(&addr), &len) == -1) {
        LOG_ERROR << "Socket get remote ip addr error. fd: " << fd << " errno: " << errno << ". error: " << strerror(errno);
        return false;
    }

    char ip[INET6_ADDRSTRLEN]; // 使用更大的缓冲区以适应IPv6地址
    void* srcAddr = nullptr;
    int addrFamily = 0;

    if (addr.ss_family == AF_INET) {
        srcAddr = &reinterpret_cast<sockaddr_in*>(&addr)->sin_addr;
        addrFamily = AF_INET;
    }
    else if (addr.ss_family == AF_INET6) {
        srcAddr = &reinterpret_cast<sockaddr_in6*>(&addr)->sin6_addr;
        addrFamily = AF_INET6;
    }
    else {
        LOG_ERROR << "Socket get remote ip addr error. Unknown address family. fd: " << fd;
        return false;
    }

    if (::inet_ntop(addrFamily, srcAddr, ip, INET6_ADDRSTRLEN) == nullptr) {
        LOG_ERROR << "Socket get remote ip addr error. errno: " << errno << ". error: " << strerror(errno);
        return false;
    }

    ipAddr = ip;
    return true;
}

bool Socketop::GetRemotePort(int fd, uint16_t& port) {
    if (fd < 0) {
        LOG_ERROR << "Socket get remote port error. invalid input param: " << fd;
        return false;
    }

    sockaddr_storage addr;
    socklen_t len = sizeof(addr);

    if (::getpeername(fd, reinterpret_cast<sockaddr*>(&addr), &len) == -1) {
        LOG_ERROR << "Socket get remote port error. fd: " << fd << " errno: " << errno << ". error: " << strerror(errno);
        return false;
    }

    if (addr.ss_family == AF_INET) {
        port = ::ntohs(reinterpret_cast<sockaddr_in*>(&addr)->sin_port);
    }
    else if (addr.ss_family == AF_INET6) {
        port = ::ntohs(reinterpret_cast<sockaddr_in6*>(&addr)->sin6_port);
    }
    else {
        LOG_ERROR << "Socket get remote port error. Unknown address family. fd: " << fd;
        return false;
    }

    return true;
}

bool Socketop::GetRemoteIpAddrPort(int fd, std::string& ipAddr, uint16_t& port) {
    if (fd < 0) {
        LOG_ERROR << "Socket get remote ip addr and port error. invalid input param: " << fd;
        return false;
    }

    sockaddr_storage addr;
    socklen_t len = sizeof(addr);

    if (::getpeername(fd, reinterpret_cast<sockaddr*>(&addr), &len) == -1) {
        LOG_ERROR << "Socket get remote ip addr and port error. fd: " << fd << " errno: " << errno << ". error: " << strerror(errno);
        return false;
    }

    char ip[INET6_ADDRSTRLEN]; // 使用更大的缓冲区以适应IPv6地址
    void* srcAddr = nullptr;
    int addrFamily = 0;

    if (addr.ss_family == AF_INET) {
        srcAddr = &reinterpret_cast<sockaddr_in*>(&addr)->sin_addr;
        addrFamily = AF_INET;
        port = ::ntohs(reinterpret_cast<sockaddr_in*>(&addr)->sin_port);
    }
    else if (addr.ss_family == AF_INET6) {
        srcAddr = &reinterpret_cast<sockaddr_in6*>(&addr)->sin6_addr;
        addrFamily = AF_INET6;
        port = ::ntohs(reinterpret_cast<sockaddr_in6*>(&addr)->sin6_port);
    }
    else {
        LOG_ERROR << "Socket get remote ip addr and port error. Unknown address family. fd: " << fd;
        return false;
    }

    if (::inet_ntop(addrFamily, srcAddr, ip, INET6_ADDRSTRLEN) == nullptr) {
        LOG_ERROR << "Socket get remote ip addr and port error. errno: " << errno << ". error: " << strerror(errno);
        return false;
    }

    ipAddr = ip;
    return true;
}

bool Socketop::SetReuseAddr(int fd, bool enabled) {
    if (fd < 0) {
        LOG_ERROR << "Socket set reuse addr error. fd invalid. fd: " << fd;
        return false;
    }

    int optval = enabled ? 1 : 0;
    if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        LOG_ERROR << "Socket set reuse addr error. errno: " << errno << ". error: " << strerror(errno);
        return false;
    }

    return true;
}

bool Socketop::IsReuseAddr(int fd) {
    if (fd < 0) {
        LOG_ERROR << "Socket is reuse addr error. fd invalid. fd: " << fd;
        return false;
    }

    int optVal;
    socklen_t optLen = sizeof(optVal);

    if (::getsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optVal, &optLen) == -1) {
        LOG_ERROR << "Socket is reuse addr error. fd: " << fd << " errno: " << errno << ". error: " << strerror(errno);
        return false;
    }

    return optVal != 0;
}

bool Socketop::SetReusePort(int fd, bool enabled) {
    if (fd < 0) {
        LOG_ERROR << "Socket set reuse port error. fd invalid. fd: " << fd;
        return false;
    }

#ifdef SO_REUSEPORT
    int optVal = enabled ? 1 : 0;
    if (::setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &optVal, sizeof(optVal)) < 0) {
        LOG_ERROR << "Socket set reuse port error. fd: " << fd << " errno: " << errno << ". error: " << strerror(errno);
        return false;
    }

    return true;
#else
    LOG_ERROR << "Socket set reuse port error. not supported on this platform. fd: " << m_fd;
    return false;
#endif
}

bool Socketop::IsReusePort(int fd) {
    if (fd < 0) {
        LOG_ERROR << "Socket is reuse port error. fd invalid. fd: " << fd;
        return false;
    }

#ifdef SO_REUSEPORT
    int optVal;
    socklen_t optLen = sizeof(optVal);

    if (::getsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &optVal, &optLen) == -1) {
        LOG_ERROR << "Socket is reuse port error. fd: " << fd << " errno: " << errno << ". error: " << strerror(errno);
        return false;
    }

    return optVal != 0;
#else
    LOG_ERROR << "Socket is reuse port error. not supported on this platform. fd: " << m_fd;
    return false;
#endif
}

bool Socketop::SetNodelay(int fd, bool enabled) {
    if (fd < 0) {
        LOG_ERROR << "Socket set nodelay error. invalid input param. fd: " << fd;
        return false;
    }

    int optVal = enabled ? 1 : 0;
    if (::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &optVal, sizeof(optVal)) < 0) {
        LOG_ERROR << "Socket set nodelay error. fd: " << fd << " errno: " << errno << ". error: " << strerror(errno);
        return false;
    }

    return true;
}

bool Socketop::SetKeepalive(int fd, bool enabled) {
    if (fd < 0) {
        LOG_ERROR << "Socket set keepalive error. invalid input param. fd: " << fd;
        return false;
    }

    int optval = enabled ? 1 : 0;
    if (::setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval)) < 0) {
        LOG_ERROR << "Socket set keepalive error. fd: " << fd << " errno: " << errno << ". error: " << strerror(errno);
        return false;
    }

    return true;
}

} // namespace Utils