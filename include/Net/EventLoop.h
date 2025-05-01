#pragma once
#include <thread>
#include <atomic>
#include <memory>
#include "Common/TypeDef.h"
#include "Utils/Utils.h"
using namespace Utils;

namespace Net {

/**
 * @note  EventLoop不允许拷贝，但允许通过指针形式传递
 * @brief 事件循环类
 */
class EventLoop : public Noncopyable, public std::enable_shared_from_this<EventLoop> {
public:
    using Ptr = std::shared_ptr<EventLoop>;
    using WkPtr = std::weak_ptr<EventLoop>;

public:
    EventLoop();
    ~EventLoop();

public:
    /**
     * @brief  启动事件循环
     * @return 启动结果
     */
    bool loop();

    /**
     * @brief  退出事件循环
     * @return 退出结果
     */
    bool quit();

    /**
     * @brief  唤醒事件循环
     * @return 唤醒结果
     */
    bool wakeup();

public:
    bool updateChannel(ChannelPtr channel);

    bool removeChannel(ChannelPtr channel);

private:
    // 事件循环所在线程id
    std::thread::id m_threadId; 

    // 事件循环是否在运行
    std::atomic_bool m_running;

    // I/O多路复用封装
    PollerPtr m_poller;

    // 唤醒事件循环的channel
    ChannelPtr m_wakeupChannel;
};

}; // namespace Net