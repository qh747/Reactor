#pragma once
#include <memory>
#include "Common/TypeDef.h"
#include "Utils/Utils.h"
#include "Utils/Address.h"
#include "Net/Socket.h"
using namespace Utils;
using namespace Common;

namespace Net {

/**
 * @brief 连接接受类
 */
class Acceptor : public Noncopyable, public std::enable_shared_from_this<Acceptor> {
public:
    using Ptr = std::shared_ptr<Acceptor>;
    using WkPtr = std::weak_ptr<Acceptor>;

public:
    Acceptor(EventLoopWkPtr loop, const Address::Ptr& addr, const Socket_t& type, bool reuseport = true);
    virtual ~Acceptor();

public:
    /**
     * @brief  开启监听
     */
    void listen();

    /**
     * @brief  正在已监听
     * @return 判断结果
     */
    inline bool isListening() const {
        return m_isListening;
    }

protected:
    /**
     * @brief  处理新连接
     * @return 处理结果
     * @param  recvTime 接收到事件的时间
     */
    virtual void handleRead(Timestamp recvTime) = 0;

protected:
    // 空闲文件描述符
    int m_idleFd;

    // 启动监听标识符
    bool m_isListening;

    // 本端地址
    Address::Ptr m_addr;

    // socket对象
    Socket::Ptr m_sock;

    // channel对象
    ChannelPtr m_channel;

    // 事件循环对象弱引用
    EventLoopWkPtr m_ownerLoop;
};

/**
 * @brief TCP连接接受类
 */
class TcpAcceptor : public Acceptor {
public:
    using Ptr = std::shared_ptr<TcpAcceptor>;
    using WkPtr = std::weak_ptr<TcpAcceptor>;
    using NewConnCb = std::function<void(Socket::Ptr& connSock, Timestamp recvTime)>;

public:
    TcpAcceptor(const EventLoopWkPtr& loop, const Address::Ptr& addr, bool reuseport = true);
    ~TcpAcceptor() override = default;

public:
    /**
     * @brief  设置新连接回调函数
     * @return 设置结果
     * @param  cb 新连接回调函数
     */
    inline void setNewConnCb(const NewConnCb& cb) {
        m_newConnCb = cb;
    }

protected:
    /**
     * @brief  处理新连接
     * @return 处理结果
     * @param  recvTime 接收到事件的时间
     */
    void handleRead(Timestamp recvTime) override;

private:
    // 新连接回调函数
    NewConnCb m_newConnCb;
};

}; // namespace Net