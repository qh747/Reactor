#pragma once
#include <list>
#include <mutex>
#include <thread>
#include <atomic>
#include <memory>
#include <functional>
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
    using Task = std::function<void()>;
    using TaskList = std::list<Task>;

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

    /**
     * @brief  更新channel
     * @return 更新结果
     * @param  channel 需要更新的channel
     */
    bool updateChannel(ChannelPtr channel);

    /**
     * @brief  移除channel
     * @return 移除结果
     * @param  channel 需要移除的channel
     */
    bool removeChannel(ChannelPtr channel);

    /**
     * @brief  执行任务
     * @return 执行结果
     * @param  task 需要执行的任务
     */
    bool executeTask(Task task);

    /**
     * @brief  执行任务
     * @return 执行结果
     * @param  task 需要执行的任务
     * @param  highPriority 是否为高优先级任务
     */
    bool executeTaskInLoop(Task task, bool highPriority = false);

public:
    /**
     * @brief  添加定时器任务
     * @return 添加结果
     * @param  id 定时器任务id
     * @param  cb 定时器任务回调函数
     * @param  expires 定时器到期时间
     * @param  intervalSec 定时器任务间隔(单位: 秒)
     */
    bool addTimerAtSpecificTime(TimerId& id, TimerTaskCb cb, Timestamp expires, double intervalSec = 0);

    /**
     * @brief  添加定时器任务
     * @return 添加结果
     * @param  id 定时器任务id
     * @param  cb 定时器任务回调函数
     * @param  delay 定时器任务延迟(单位: 秒)
     * @param  intervalSec 定时器任务间隔(单位: 秒)
     */
    bool addTimerAfterSpecificTime(TimerId& id, TimerTaskCb cb, double delay, double intervalSec = 0);

    /**
     * @brief  移除定时器任务
     * @return 移除结果
     * @param  id 定时器任务id
     */
    bool delTimer(TimerId id);

public:
    /**
     * @brief  事件循环是否在运行
     * @return 判断结果
     */
    inline bool isRunning() const {
        return m_running;
    }

    /**
     * @brief  事件循环是否在等待
     * @return 判断结果
     */
    inline bool isWaiting() const {
        return m_waiting;
    }

    /**
     * @brief  判断当前线程是否为事件循环所在线程
     * @return 判断结果
     */
    inline bool isInCurrentThread() const {
        return m_threadId == std::this_thread::get_id();
    }

    /**
     * @brief  获取事件循环所在线程id
     * @return 线程id
     */
    inline std::thread::id getThreadId() const {
        return m_threadId;
    }

private:
    /**
     * @brief  初始化eventLoop
     * @return 初始化结果
     */
    bool init();

    /**
     * @brief  处理事件
     * @return 处理结果
     */
    bool handleTask();

private:
    // 事件循环所在线程id
    std::thread::id m_threadId; 

    // 事件循环是否在运行
    std::atomic_bool m_running;

    // 事件循环是否在等待poll()中
    std::atomic_bool m_waiting;

    // I/O多路复用封装
    PollerPtr m_poller;

    // 唤醒事件循环的channel
    ChannelPtr m_wakeupChannel;

    // 有事件需要处理的channel列表
    ChannelWrapperList m_activeChannels;

    // 任务列表
    TaskList m_taskList;

    // 互斥锁, 保护任务列表
    std::mutex m_taskMutex;

    // 定时器队列
    TimerQueuePtr m_timerQueue;
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