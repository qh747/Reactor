#include <iostream>
#include <App/TcpServer.h>
using namespace App;

class ExampleTcpServer : public std::enable_shared_from_this<ExampleTcpServer> {
public:
    using Ptr = std::shared_ptr<ExampleTcpServer>;

public:
    ExampleTcpServer(const std::string& ip, uint16_t port) {
        auto addr = std::make_shared<IPv4Address>(ip, port);
        m_server = std::make_shared<TcpServer>(addr, nullptr, 0);

        m_server->setConnectCallback([](const Connection::Ptr& conn, bool isConn) {
            if (isConn) {
                std::cout << "new connection: " << conn->getConnectionInfo() << std::endl;
            }
            else {
                std::cout << "connection closed: " << conn->getConnectionInfo() << std::endl;
            }
        });

        m_server->setMessageCallback([](const Connection::Ptr& conn, const Buffer::Ptr& buf, Timestamp recvTime) {
            std::vector<uint8_t> buffer;
            std::size_t len = 0;

            buf->read(buffer, len);
            std::cout << "recv: " << std::string(buffer.begin(), buffer.end()) << std::endl;

            conn->send(buffer.data(), len);
            std::cout << "send: " << std::string(buffer.begin(), buffer.end()) << std::endl;
        });
    }

    ~ExampleTcpServer() {
        m_server->shutdown();
    }

public:
    void run() const {
        m_server->run();
    }

private:
    TcpServer::Ptr m_server;
};

int main() {
    ExampleTcpServer::Ptr server = std::make_shared<ExampleTcpServer>("127.0.0.1", 8888);
    server->run();

    char ch;
    std::cin >> ch;

    return 0;
}