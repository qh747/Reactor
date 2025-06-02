#pragma once
#include <memory>
#include <functional>
#include "Common/TypeDef.h"
#include "Common/DataDef.h"
#include "Utils/Utils.h"
using namespace Common;
using namespace Utils;

namespace Net {

/**
 * @note Channel不允许拷贝，但允许通过指针形式传递
 * @brief 通道类
 */
class Channel : public Noncopyable, public std::enable_shared_from_this<Channel> {
public:
    // 允许以下友元类修改channel状态
    friend class PPoller;
    friend class EpPoller;

public:
    using Ptr = std::shared_ptr<Channel>;
    using WkPtr = std::weak_ptr<Channel>;

    // 事件回调函数类型
    using EventCb = std::function<void(Timestamp)>;
    // 事件回调函数map，key = 事件类型，value = 事件回调函数
    using EventCbMap = std::unordered_map<Event_t, EventCb>;

public:
    Channel(EventLoopWkPtr loop, int fd);
    ~Channel();

public:
    /**
     * @brief  开启channel
     * @return 开启结果
     * @param  type 监听的事件类型
     */
    bool open(Event_t type);

    /**
     * @brief  更新监听事件
     * @return 更新结果
     */
    bool update(Event_t type);

    /**
     * @brief  关闭channel
     * @return 关闭结果
     */
    bool close();

    /**
     * @brief  事件处理
     * @return 处理结果
     * @param  type 发生事件类型
     * @param  recvTime 发生事件时间
     */
    bool handleEvent(Event_t type, Timestamp recvTime);

    /**
     * @brief  设置事件回调函数
     * @return 设置结果
     * @param  type 事件类型
     * @param  cb 回调函数
     */
    bool setEventCb(Event_t type, EventCb cb);

public:
    /**
     * @brief  获取文件描述符
     * @return 文件描述符
     */
    inline int getFd() const {
        return m_fd;
    }

    /**
     * @brief  获取状态
     * @return 状态
     */
    inline State_t getState() const {
        return m_state;
    }

    /**
     * @brief  获取监听事件类型
     * @return 事件类型
     */
    inline Event_t getEvType() const {
        return m_listenEvType;
    }

    /**
     * @brief  判断是否可写
     * @return 判断结果
     */
    inline bool writeEnabled() const {
        return static_cast<int>(m_listenEvType) & static_cast<int>(Event_t::EvTypeWrite);
    }

    /**
     * @brief 开启写事件
     */
    inline void enableWrite() {
        int evType = static_cast<int>(m_listenEvType) | static_cast<int>(Event_t::EvTypeWrite);
        m_listenEvType = static_cast<Event_t>(evType);
    }

    /**
     * @brief 关闭写事件
     */
    inline void disableWrite() {
        int evType = static_cast<int>(m_listenEvType) & (~static_cast<int>(Event_t::EvTypeWrite));
        m_listenEvType = static_cast<Event_t>(evType);
    }

    /**
     * @brief  判断是否可读
     * @return 判断结果
     */
    inline bool readEnabled() const {
        return static_cast<int>(m_listenEvType) & static_cast<int>(Event_t::EvTypeRead);
    }

    /**
     * @brief 开启读事件
     */
    inline void enableRead() {
        int evType = static_cast<int>(m_listenEvType) | static_cast<int>(Event_t::EvTypeRead);
        m_listenEvType = static_cast<Event_t>(evType);
    }

    /**
     * @brief 关闭读事件
     */
    inline void disableRead() {
        int evType = static_cast<int>(m_listenEvType) & (~static_cast<int>(Event_t::EvTypeRead));
        m_listenEvType = static_cast<Event_t>(evType);
    }

    /**
     * @brief  获取事件循环对象
     * @return 事件循环对象
     */
    inline EventLoopWkPtr getOwnerLoop() const {
        return m_ownerLoop;
    }

private:
    /**
     * @brief 设置状态
     * @param state 状态
     */
    inline void setState(State_t state) {
        m_state = state;
    }

    /**
     * @brief  无需校验的事件处理
     * @return 处理结果
     * @param  type 发生事件类型
     * @param  recvTime 发生事件时间
     */
    bool handleEventWithoutCheck(Event_t type, Timestamp recvTime);

private:
    // channel关联事件句柄id
    const int m_fd;

    // 状态
    State_t m_state{State_t::StatePending};

    // 监听事件类型
    Event_t m_listenEvType{Event_t::EvTypeNone};

    // 事件循环对象弱引用
    EventLoopWkPtr m_ownerLoop;

    // 事件回调函数map
    EventCbMap m_evCbMap;
};

/**
 * @brief Channel包装类
 */
class ChannelWrapper {
public:
    using Ptr = std::shared_ptr<ChannelWrapper>;
    using WkPtr = std::weak_ptr<ChannelWrapper>;

public:
    ChannelWrapper(Channel::Ptr channel, Event_t activeEvType)
        : m_activeEvType(activeEvType),
          m_channel(std::move(channel)) {
    }
    ~ChannelWrapper() = default;

public:
    // 激活channel的事件类型
    Event_t m_activeEvType;

    // channel对象
    Channel::Ptr m_channel;
};

}; // namespace Net