#pragma once
#include <memory>
#include <chrono>
#include <functional>
#include <unordered_map>
#include "Common/DataDef.h"
#include "Common/Utils.h"
using namespace Common;

namespace Net {

// 前置声明
class EventLoop;

/**
 * @note Channel不允许拷贝，但允许通过指针形式传递
 * @brief 通道类
 */
class Channel : public Noncopyable, public std::enable_shared_from_this<Channel> {
public:
    using Ptr = std::shared_ptr<Channel>;
    using EventLoopPtr = std::shared_ptr<EventLoop>;
    using Timestamp = std::chrono::system_clock::time_point;
    using EventCb = std::function<void(Timestamp)>;
    using EventCbMap = std::unordered_map<Event_t, EventCb>;

public:
    Channel(EventLoopPtr loop, int fd);
    ~Channel();

public:
    /**
     * @brief 事件处理
     * @param type 发生事件类型
     * @param recvTime 发生事件时间
     */
    void handleEvent(Event_t type, Timestamp recvTime);

    /**
     * @brief 设置监听事件类型
     */
    void setListenEvent(Event_t type);

public:
    /**
     * @brief 设置事件回调函数
     * @param type 事件类型
     * @param cb 回调函数
     */
    inline void setEventCb(Event_t type, EventCb cb) {
        m_evCbMap[type] = std::move(cb);
    }

    /**
     * @brief 设置状态
     * @param state 状态
     */
    inline void setState(State_t state) {
        m_state = state;
    }

    /**
     * @brief 获取状态
     * @return 状态
     */
    inline State_t getState() const {
        return m_state;
    }

    /**
     * @brief 获取文件描述符
     * @return 文件描述符
     */
    inline int getFd() const {
        return m_fd;
    }

    /**
     * @brief 获取事件循环对象
     * @return 事件循环对象
     */
    inline EventLoopPtr getOwnerLoop() const {
        return m_ownerLoopPtr;
    }

private:
    // channel关联事件句柄id
    const int m_fd;

    // 状态
    State_t m_state{State_t::StatePending};

    // 监听事件类型
    Event_t m_listenEvType{Event_t::EvTypeNone};

    // 事件循环对象
    EventLoopPtr m_ownerLoopPtr;

    // 事件回调函数map
    EventCbMap m_evCbMap;
};

}; // namespace Net