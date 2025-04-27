#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <arpa/inet.h>
#include "common/logger.h"
#include "net/netAddr.h"
using namespace COMMON;

namespace NET {

InetAddrV4::InetAddrV4(const std::string& ip, uint16_t port) {
    if (ip.empty() || 0 == port) {
        throw std::runtime_error("Invalid input param");
    }

    memset(&m_addr, 0, sizeof(m_addr)); 
    m_addr.sin_family = AF_INET; 

    if (inet_pton(AF_INET, ip.c_str(), &m_addr.sin_addr) != 1) {
        throw std::runtime_error("Invalid ip address");
    }

    m_addr.sin_port = htons(port);

    if (!this->valid()) {
        throw std::runtime_error("Invalid socket address");
    }
}

InetAddrV4::InetAddrV4(const sockaddr_in& addr) : m_addr(addr) {
    if (!this->valid()) {
        throw std::runtime_error("Invalid socket address");
    }
}

bool InetAddrV4::getSockAddr(sockaddr_in& addr) const {
    if (!this->valid()) {
        LOG_ERROR << "Get socket address error. invalid address";
        return false;
    }

    addr = m_addr;
    return true;
}

bool InetAddrV4::getIpAddr(std::string& ip) const {
    if (!this->valid()) {
        LOG_ERROR << "Get ip address error. invalid address";
        return false;
    }

    char buffer[INET_ADDRSTRLEN]; 
    if (nullptr == inet_ntop(AF_INET, &(m_addr.sin_addr), buffer, INET_ADDRSTRLEN)) {
        LOG_ERROR << "Get ip address error. call inet_ntop() failed";
        return false;
    }
    
    ip = std::string(buffer);
    return true;
}

bool InetAddrV4::getPort(uint16_t& port) const {
    if (!this->valid()) {
        LOG_ERROR << "Get port error. invalid address";
        return false;
    }

    port = ntohs(m_addr.sin_port);
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