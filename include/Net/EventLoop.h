#pragma once
#include <thread>
#include <atomic>
#include <memory>
#include "Common/TypeDef.h"
#include "Utils/Utils.h"
using namespace Utils;

namespace Net {

// 前置声明EventLoop管理类
class EventLoopManager;

/**
 * @note  EventLoop不允许拷贝，但允许通过指针形式传递
 * @brief 事件循环类
 */
class EventLoop : public Noncopyable, public std::enable_shared_from_this<EventLoop> {
public:
    // 允许以下类初始化EventLoop
    friend class EventLoopManager;

public:
    using Ptr = std::shared_ptr<EventLoop>;
    using WkPtr = std::weak_ptr<EventLoop>;

public:
    EventLoop(std::thread::id threadId);
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

public:
    /**
     * @brief  事件循环是否在运行
     * @return 判断结果
     */
    inline bool isRunning() const {
        return m_running;
    }

private:
    /**
     * @brief  初始化eventLoop
     * @return 初始化结果
     */
    bool init();

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

/**
 * @brief  事件循环管理类
 */
class EventLoopManager : public NoncopyableConstructable {
public:
    class ManageObject;

public:
    /**
     * @brief  获取当前线程的事件循环
     * @return 当前线程的事件循环
     */
    static EventLoop::Ptr GetCurrentEventLoop();
}; 

}; // namespace Net