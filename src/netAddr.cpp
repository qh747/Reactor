#include <cstring>
#include <stdexcept>
#include <arpa/inet.h>
#include "common/utils.h"
#include "common/logger.h"
#include "net/netAddr.h"
using namespace COMMON;

namespace NET {

InetAddrV4::InetAddrV4(const std::string& ip, uint16_t port, AddrByteOrder order) : m_addr() {
    m_addr.sin_family = AF_INET;
    
    if (AddrByteOrder::NETWORK == order) {
        std::string ipHost;
        if (!AddrHelper::IpAddrFromNetToHost(ip, ipHost)) {
           throw std::runtime_error("Ip addr from net to host error");
        }

        uint16_t portHost;
        if (!AddrHelper::PortFromNetToHost(port, portHost)) {
            throw std::runtime_error("Port from net to host error");
        }

        m_addr.sin_addr.s_addr = inet_addr(ipHost.c_str());
        m_addr.sin_port = portHost;
    }
    else {
        m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
        m_addr.sin_port = port;
    }
}

InetAddrV4::InetAddrV4(const sockaddr_in& addr, AddrByteOrder order) : m_addr(addr) {
    if (AddrByteOrder::NETWORK == order) {
        m_addr.sin_addr.s_addr = ntohl(m_addr.sin_addr.s_addr);
        m_addr.sin_port = ntohs(m_addr.sin_port);
    }
}

bool InetAddrV4::getSockAddr(AddrByteOrder order, sockaddr_in& addr) const {
    if (!this->valid()) {
        LOG_ERROR << "Get socket address error. invalid address";
        return false;
    }

    addr = m_addr;
    if (AddrByteOrder::NETWORK == order) {
        addr.sin_addr.s_addr = htonl(addr.sin_addr.s_addr);
        addr.sin_port = htons(addr.sin_port);
    }

    return true;
}

bool InetAddrV4::getIpAddr(AddrByteOrder order, std::string& ip) const {
    if (!this->valid()) {
        LOG_ERROR << "Get ip address error. invalid address";
        return false;
    }

    char ipHost[INET_ADDRSTRLEN];
    const char* result = inet_ntop(AF_INET, &(m_addr.sin_addr), ipHost, INET_ADDRSTRLEN);
    if (result == nullptr) {
        LOG_ERROR << "Get ip address error. error info: " << std::strerror(errno); 
        return false;
    }

    if (AddrByteOrder::NETWORK == order) {
        if (!AddrHelper::IpAddrFromHostToNet(ipHost, ip)) {
            LOG_ERROR << "Get ip address error. convect to network byte order failed";
            return false;
        }
    }
    else {
        ip = ipHost;
    }

    return true;
}

bool InetAddrV4::getPort(AddrByteOrder order, uint16_t& port) const {
    if (!this->valid()) {
        LOG_ERROR << "Get port error. invalid address";
        return false;
    }

    if (AddrByteOrder::NETWORK == order) {
        if (!AddrHelper::PortFromHostToNet(m_addr.sin_port, port)) {
            LOG_ERROR << "Get port error. convect to network byte order failed";
            return false;
        }
    }
    else {
        port = m_addr.sin_port;
    }

    return true;
}

bool InetAddrV4::getIpPort(AddrByteOrder order, std::string& ipPort) const {
    if (!this->valid()) {
        LOG_ERROR << "Get ip address and port error. invalid address";
        return false;
    }

    std::string ip;
    if (!this->getIpAddr(order, ip)) {
        LOG_ERROR << "Get ip address and port error. get ip address failed";
        return false;
    }

    uint16_t port;
    if (!this->getPort(order, port)) {
        LOG_ERROR << "Get ip address and port error. get port failed";
        return false;
    }

    ipPort = ip + ":" + std::to_string(port);
    return true;
}

bool InetAddrV4::valid() const {
    if (AF_INET != m_addr.sin_family) {
        return false;
    }

    uint32_t ip = m_addr.sin_addr.s_addr;
    if (INADDR_NONE == ip || 0 == ip) {
        return false;
    }

    if (0 == m_addr.sin_port) {
        return false;
    }

    return true;
}

} // namespace NET