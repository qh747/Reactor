#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <arpa/inet.h>
#include "Utils/Logger.h"
#include "Utils/Address.h"
using namespace Utils;

namespace Utils {

/** ------------------------------------ IPv4Address ------------------------------------------- */

IPv4Address::IPv4Address(const std::string& ip, uint16_t port)
    : m_addr() {
    do {
        if (ip.empty() || 0 == port) {
            LOG_ERROR << "IPv4Address construct error. Invalid input param";
            break;
        }

        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin_family = AF_INET;

        if (inet_pton(AF_INET, ip.c_str(), &m_addr.sin_addr) != 1) {
            LOG_ERROR << "IPv4Address construct error. Invalid ip address";
            break;
        }

        m_addr.sin_port = ::htons(port);

        if (!this->IPv4Address::valid()) {
            LOG_ERROR << "IPv4Address construct error. Invalid socket address";
        }
    } while (false);
}

IPv4Address::IPv4Address(const sockaddr_in& addr)
    : m_addr(addr) {
    if (!this->IPv4Address::valid()) {
        throw std::runtime_error("Invalid socket address");
    }
}

bool IPv4Address::getSockAddr(sockaddr& addr) const {
    if (!this->valid()) {
        LOG_ERROR << "Get socket address error. invalid address";
        return false;
    }

    memcpy(&addr, &m_addr, sizeof(sockaddr_in));
    return true;
}

bool IPv4Address::getIpAddr(std::string& ip) const {
    if (!this->valid()) {
        LOG_ERROR << "Get ip address error. invalid address";
        return false;
    }

    char buffer[INET_ADDRSTRLEN];
    if (nullptr == ::inet_ntop(AF_INET, &(m_addr.sin_addr), buffer, INET_ADDRSTRLEN)) {
        LOG_ERROR << "Get ip address error. call inet_ntop() failed";
        return false;
    }

    ip = std::string(buffer);
    return true;
}

bool IPv4Address::getPort(uint16_t& port) const {
    if (!this->valid()) {
        LOG_ERROR << "Get port error. invalid address";
        return false;
    }

    port = ::ntohs(m_addr.sin_port);
    return true;
}

bool IPv4Address::getIpPort(std::string& ip, uint16_t& port) const {
    if (!this->valid()) {
        LOG_ERROR << "Get ip address and port error. invalid address";
        return false;
    }

    char buffer[INET_ADDRSTRLEN];
    if (nullptr == ::inet_ntop(AF_INET, &(m_addr.sin_addr), buffer, INET_ADDRSTRLEN)) {
        LOG_ERROR << "Get ip address error. call inet_ntop() failed";
        return false;
    }

    ip = std::string(buffer);
    port = ::ntohs(m_addr.sin_port);
    return true;
}

bool IPv4Address::valid() const {
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

std::string IPv4Address::printIpPort() const {
    std::string ip;
    uint16_t port;

    if (this->IPv4Address::getIpPort(ip, port)) {
        return ip + " : " + std::to_string(port);
    }
    else {
        return "invalid ipv4 address";
    }
}

/** ------------------------------------ IPv6Address ------------------------------------------- */

IPv6Address::IPv6Address(const std::string& ip, uint16_t port)
    : m_addr() {
    if (ip.empty() || 0 == port) {
        throw std::runtime_error("Invalid input param");
    }

    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;

    if (::inet_pton(AF_INET6, ip.c_str(), &m_addr.sin6_addr) != 1) {
        throw std::runtime_error("Invalid ip address");
    }

    m_addr.sin6_port = ::htons(port);

    if (!this->IPv6Address::valid()) {
        throw std::runtime_error("Invalid socket address");
    }
}

IPv6Address::IPv6Address(const sockaddr_in6& addr)
    : m_addr(addr) {
    if (!this->IPv6Address::valid()) {
        throw std::runtime_error("Invalid socket address");
    }
}

bool IPv6Address::getSockAddr(sockaddr& addr) const {
    if (!this->valid()) {
        LOG_ERROR << "Get socket address error. invalid address";
        return false;
    }

    memcpy(&addr, &m_addr, sizeof(sockaddr_in6));
    return true;
}

bool IPv6Address::getIpAddr(std::string& ip) const {
    if (!this->valid()) {
        LOG_ERROR << "Get ip address error. invalid address";
        return false;
    }

    char buffer[INET6_ADDRSTRLEN];
    if (nullptr == ::inet_ntop(AF_INET6, &(m_addr.sin6_addr), buffer, INET6_ADDRSTRLEN)) {
        LOG_ERROR << "Get ip address error. call inet_ntop() failed";
        return false;
    }

    ip = std::string(buffer);
    return true;
}

bool IPv6Address::getPort(uint16_t& port) const {
    if (!this->valid()) {
        LOG_ERROR << "Get port error. invalid address";
        return false;
    }

    port = ::ntohs(m_addr.sin6_port);
    return true;
}

bool IPv6Address::getIpPort(std::string& ip, uint16_t& port) const {
    if (!this->valid()) {
        LOG_ERROR << "Get ip address and port error. invalid address";
        return false;
    }

    char buffer[INET6_ADDRSTRLEN];
    if (nullptr == ::inet_ntop(AF_INET6, &(m_addr.sin6_addr), buffer, INET6_ADDRSTRLEN)) {
        LOG_ERROR << "Get ip address and port error. call inet_ntop() failed";
        return false;
    }

    ip = std::string(buffer);
    port = ::ntohs(m_addr.sin6_port);
    return true;
}

bool IPv6Address::valid() const {
    if (AF_INET6 != m_addr.sin6_family) {
        return false;
    }

    const auto* addr = reinterpret_cast<const uint32_t*>(&m_addr.sin6_addr);

    bool isZero = true;
    for (int i = 0; i < 4; ++i) {
        if (addr[i] != 0) {
            isZero = false;
            break;
        }
    }

    // 全零地址无效
    if (isZero) {
        return false;
    }

    // 端口为 0
    if (0 == m_addr.sin6_port) {
        return false;
    }

    return true;
}

std::string IPv6Address::printIpPort() const {
    std::string ip;
    uint16_t port;

    if (this->IPv6Address::getIpPort(ip, port)) {
        return ip + " : " + std::to_string(port);
    }
    else {
        return "invalid ipv6 address";
    }
}

} // namespace Utils