#pragma once
#include <atomic>
#include <memory>
#include "Common/TypeDef.h"
#include "Utils/Utils.h"
#include "Utils/Buffer.h"
#include "Net/Socket.h"
using namespace Utils;
using namespace Common;

namespace Net {

/**
 * @brief 连接类
 */
class Connection : public Noncopyable, public std::enable_shared_from_this<Connection> {
public:
    using Ptr = std::shared_ptr<Connection>;
    using WkPtr = std::weak_ptr<Connection>;
    using ConnCb = std::function<void(Connection::Ptr conn, bool isConn)>;
    using CloseCb = std::function<void(Connection::Ptr conn)>;
    using ReadableCb = std::function<void(Connection::Ptr conn, Buffer::Ptr buf, Timestamp recvTime)>;
    using WriteableCb = std::function<void(Connection::Ptr conn)>;

public:
    Connection(const EventLoopWkPtr& loop, const Socket::Ptr& sock);
    virtual ~Connection() = default;

public:
    /**
     * @brief  连接打开
     * @return 打开结果
     */
    virtual bool open();

    /**
     * @brief  连接关闭
     * @return 关闭结果
     * @param  delay 延迟关闭时间(单位: 秒)
     */
    virtual bool close(double delay);

    /**
     * @brief  连接重新建立
     * @return 连接结果
     */
    virtual bool reconnect();

    /**
     * @brief  连接断开
     * @return 断开结果
     */
    virtual bool disconnect();

    /**
     * @brief  发送数据
     * @return 发送结果
     * @param  data 发送数据
     * @param  size 发送数据长度
     */
    virtual bool send(const void* data, std::size_t size);

public:
    /**
     * @brief  获取连接信息
     * @return 连接信息
     */
    std::string getConnectionInfo() const;

    /**
     * @brief  开启读事件
     * @return 开启结果
     */
    bool enableRead();

    /**
     * @brief  关闭读事件
     * @return 关闭结果
     */
    bool disableRead();

    /**
     * @brief 设置连接回调函数
     * @param cb 回调函数
     */
    inline void setConnectCallback(const ConnCb& cb) {
        m_connCb = cb;
    }

    /**
     * @brief 设置连接关闭回调函数
     * @param cb 回调函数
     */
    inline void setCloseCallback(const CloseCb& cb) {
        m_closeCb = cb;
    }

    /**
     * @brief 设置消息回调函数
     * @param cb 回调函数
     */
    inline void setMessageCallback(const ReadableCb& cb) {
        m_readCb = cb;
    }

    /**
     * @brief 设置可写回调函数
     * @param cb 回调函数
     */
    inline void setWriteCompleteCallback(const WriteableCb& cb) {
        m_writeCb = cb;
    }

    /**
     * @brief  获取输入缓冲区
     * @return 输入缓冲区
     */
    inline Buffer::Ptr getInputBuffer() const {
        return m_inBuf;
    }

    /**
     * @brief  获取输出缓冲区
     * @return 输出缓冲区
     */
    inline Buffer::Ptr getOutputBuffer() const {
        return m_outBuf;
    }

    /**
     * @brief  获取连接id
     * @return 连接id
     */
    inline std::string getConnectionId() const {
        return m_connId;
    }

    /**
     * @brief  获取本地地址
     * @return 获取结果
     * @param  addr 本地地址
     */
    inline bool getLocalAddr(Address::Ptr& addr) const {
        if (nullptr == m_sock) {
            return false;
        }

        addr = m_sock->getLocalAddr();
        return true;
    }

    /**
     * @brief  打印本地地址
     * @return 本地地址
     */
    inline std::string printLocalAddr() const {
        return nullptr == m_sock ? "" : m_sock->printLocalAddr();
    }

    /**
     * @brief  获取对端地址
     * @return 获取结果
     * @param  addr 对端地址
     */
    inline bool getRemoteAddr(Address::Ptr& addr) const {
        if (nullptr == m_sock) {
            return false;
        }

        addr = m_sock->getRemoteAddr();
        return true;
    }

    /**
     * @brief  打印本地地址
     * @return 本地地址
     */
    inline std::string printRemoteAddr() const {
        return nullptr == m_sock ? "" : m_sock->printRemoteAddr();
    }

    /**
     * @brief  获取socket fd
     * @return socket fd
     */
    inline int getFd() const {
        return nullptr == m_sock ? -1 : m_sock->getFd();
    }

    /**
     * @brief  获取事件循环对象弱引用
     * @return 事件循环对象弱引用
     */
    inline EventLoopWkPtr getOwnerLoop() const {
        return m_ownerLoop;
    }

protected:
    /**
     * @brief  处理读事件
     * @param  recvTime 接收到事件的时间
     */
    virtual void handleRead(Timestamp recvTime) = 0;

    /**
     * @brief  处理写事件
     * @param  recvTime 接收到事件的时间
     */
    virtual void handleWrite(Timestamp recvTime) = 0;

    /**
     * @brief  处理连接关闭事件
     * @param  recvTime 接收到事件的时间
     */
    virtual void handleClose(Timestamp recvTime) = 0;

    /**
     * @brief  处理连接错误事件
     * @param  recvTime 接收到事件的时间
     */
    virtual void handleError(Timestamp recvTime) = 0;

protected:
    // 连接id
    std::string m_connId;

    // socket对象
    Socket::Ptr m_sock;

    // 连接状态回调函数
    ConnCb m_connCb;

    // 连接关闭回调函数
    CloseCb m_closeCb;

    // 可读回调函数
    ReadableCb m_readCb;

    // 可写回调函数
    WriteableCb m_writeCb;

    // 输入缓冲区对象
    Buffer::Ptr m_inBuf;

    // 输出缓冲区对象
    Buffer::Ptr m_outBuf;

    // channel对象
    ChannelPtr m_channel;

    // 事件循环对象弱引用
    EventLoopWkPtr m_ownerLoop;

    // 连接状态
    std::atomic<ConnState_t> m_connState;
};

/**
 * @brief tcp连接类
 */
class TcpConnection : public Connection {
public:
    using Ptr = std::shared_ptr<TcpConnection>;
    using WkPtr = std::weak_ptr<TcpConnection>;
    using HighWaterMarkCb = std::function<void(Connection::Ptr conn, std::size_t highWaterMark)>;

public:
    TcpConnection(const EventLoopWkPtr& loop, const Socket::Ptr& sock);
    ~TcpConnection() override = default;

public:
    /**
     * @brief  发送数据
     * @return 发送结果
     * @param  data 发送数据
     * @param  size 发送数据长度
     */
    bool send(const void* data, std::size_t size) override;

    /**
     * @brief  关闭连接
     * @return 关闭结果
     */
    bool shutdown();

public:
    /**
     * @brief 设置高水位回调函数
     * @param cb 回调函数
     * @param highWaterMark 高水位
     */
    inline void setHighWaterMarkCallback(const HighWaterMarkCb& cb, std::size_t highWaterMark) {
        m_highWaterMarkCb = cb;
    }

    /**
     * @brief 设置是否启用nodelay
     * @param enabled 是否启用
     */
    inline void setNodelay(bool enabled) const {
        m_sock->setNoDelayEnabled(enabled);
    }

    /**
     * @brief 设置是否启用keepalive
     * @param enabled 是否启用
     */
    inline void setKeepalive(bool enabled) const {
        m_sock->setKeepaliveEnabled(enabled);
    }

protected:
    /**
     * @brief  处理读事件
     * @param  recvTime 接收到事件的时间
     */
    void handleRead(Timestamp recvTime) override;

    /**
     * @brief  处理写事件
     * @param  recvTime 接收到事件的时间
     */
    void handleWrite(Timestamp recvTime) override;

    /**
     * @brief  处理连接关闭事件
     * @param  recvTime 接收到事件的时间
     */
    void handleClose(Timestamp recvTime) override;

    /**
     * @brief  处理连接错误事件
     * @param  recvTime 接收到事件的时间
     */
    void handleError(Timestamp recvTime) override;

private:
    // tcp高水位线
    std::size_t m_highWaterMark;

    // 高水位回调函数
    HighWaterMarkCb m_highWaterMarkCb;
};

}; // namespace Net