#include <iostream>
#include <Utils/Logger.h>
#include <Net/TcpServer.h>
using namespace Net;
using namespace Utils;

class EchoServer : public std::enable_shared_from_this<EchoServer> {
public:
    using Ptr = std::shared_ptr<EchoServer>;

public:
    EchoServer(const std::string& ip, uint16_t port) {
        auto addr = std::make_shared<IPv4Address>(ip, port);
        m_server = std::make_shared<TcpServer>(addr, nullptr, 0);

        m_server->setConnectCallback([](const Connection::Ptr& conn, bool isConn) {
            if (isConn) {
                LOG_INFO << "new connection: " << conn->getConnectionInfo();
            }
            else {
                LOG_INFO << "connection closed: " << conn->getConnectionInfo();
            }
        });

        m_server->setMessageCallback([](const Connection::Ptr& conn, const Buffer::Ptr& buf, Timestamp recvTime) {
            std::vector<uint8_t> buffer;
            std::size_t len = 0;

            buf->read(buffer, len);
            LOG_INFO << "recv: " << std::string(buffer.begin(), buffer.end());

            conn->send(buffer.data(), len);
            LOG_INFO << "send: " << std::string(buffer.begin(), buffer.end());
        });
    }

    ~EchoServer() {
        m_server->shutdown();
    }

public:
    void run() const {
        LOG_INFO << "echo server run: " << m_server->getServerInfo();
        m_server->run();
    }

private:
    TcpServer::Ptr m_server;
};

int main() {
    EchoServer::Ptr server = std::make_shared<EchoServer>("127.0.0.1", 8881);
    server->run();

    char ch;
    std::cin >> ch;

    return 0;
}