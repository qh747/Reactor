#include <chrono>
#include <iomanip>
#include <sstream>
#include <unistd.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h> 
#include "common/utils.h"

namespace COMMON {

std::string TimeHelper::GetCurrentTime() {
    auto now = std::chrono::system_clock::now();

    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::tm now_tm = *std::localtime(&now_time_t);

    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << now_ms.count();
    return oss.str();
}

std::string TimeHelper::GetCurrentData() {
    time_t now = time(nullptr);
    struct tm tstruct;

    char buf[80];
    tstruct = *localtime(&now);

    strftime(buf, sizeof(buf), "%Y%m%d", &tstruct);
    return std::string(buf);
}

const char* StringHelper::GetFileName(const char* path) {
    const char* file = path;
    while (*path) {
        if (*path == '/' || *path == '\\') file = path + 1;
        path++;
    }
    return file;
}

std::string DirHelper::GetDirectory(const std::string& path) {
    std::size_t found = path.find_last_of("/\\");
    if (found != std::string::npos) {
        return path.substr(0, found);
    }

    return "";
}

bool DirHelper::CheckDirectoryExists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info)) {
        return false;
    }

    return (info.st_mode & S_IFDIR);
}

bool DirHelper::CreateDirectory(const std::string& path) {
    if (mkdir(path.c_str(), 0755) == -1) {
        return false;
    }

    return true;
}

bool AddrHelper::IpAddrFromNetToHost(const std::string& ipAddrNet, std::string& ipAddrHost) {
    struct in_addr netAddr;
    
    // 1. 将字符串IP转换为网络字节序的in_addr
    if (inet_pton(AF_INET, ipAddrNet.c_str(), &netAddr) != 1) {
        // 转换失败（无效的IP格式）
        return false;
    }

    // 2. 将网络字节序的in_addr转换回字符串形式（主机字节序）
    char hostIpStr[INET_ADDRSTRLEN];
    if (!inet_ntop(AF_INET, &netAddr, hostIpStr, INET_ADDRSTRLEN)) {
        // 转换失败
        return false;
    }

    ipAddrHost = hostIpStr;
    return true;
}

bool AddrHelper::IpAddrFromHostToNet(const std::string& ipAddrHost, std::string& ipAddrNet) {
    struct in_addr addr;
    
    // 验证IP格式并转换为网络字节序
    if (inet_pton(AF_INET, ipAddrHost.c_str(), &addr) != 1) {
        return false;
    }
    
    // 转换回字符串确保格式正确
    char buf[INET_ADDRSTRLEN];
    if (!inet_ntop(AF_INET, &addr, buf, sizeof(buf))) {
        return false;
    }
    
    ipAddrNet = buf;
    return true;
}

bool AddrHelper::PortFromNetToHost(uint16_t portNet, uint16_t& portHost) {
    portHost = ntohs(portNet);
    return true;
}

bool AddrHelper::PortFromHostToNet(uint16_t portHost, uint16_t& portNet) {
    portNet = htons(portHost);
    return true;
}

} // namespace COMMON