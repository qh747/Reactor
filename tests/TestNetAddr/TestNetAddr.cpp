#include <string>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <Net/NetAddr.h>
using namespace Net;

void FuncTestFst() {
    std::cout << "NET ADDRESS TEST FIRST -----------------------------" << std::endl;

    InetAddrV4 addr("127.0.0.1", 90);

    std::string ip;
    addr.getIpAddr(ip);

    uint16_t port;
    addr.getPort(port);
    std::cout << "host order. ip: " << ip << ", port: " << port << std::endl;
}

void FuncTestSnd() {
    std::cout << "NET ADDRESS TEST SECOND -----------------------------" << std::endl;

    try {
        InetAddrV4 addr("256.0.0.1", 90);
    } catch (const std::exception& e) {
        std::cout << "catch exception: " << e.what() << std::endl;
    }
}

void FuncTestTrd() {
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;

    if (inet_pton(AF_INET, "192.168.3.10", &addr.sin_addr) <= 0) {
        std::cerr << "Invalid ip address" << std::endl;
        return;
    }

    addr.sin_port = htons(9091);

    InetAddrV4 addrV4(addr);

    std::string ip;
    addrV4.getIpAddr(ip);

    uint16_t port;
    addrV4.getPort(port);

    std::cout << "host order. ip: " << ip << ", port: " << port << std::endl;
}

int main() {
    FuncTestFst();
    FuncTestSnd();
    FuncTestTrd();

    return 0;
}