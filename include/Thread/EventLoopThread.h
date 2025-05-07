#pragma once
#include <mutex>
#include <atomic>
#include <memory>
#include <thread>
#include <functional>
#include "Common/TypeDef.h"
#include "Utils/Utils.h"
using namespace Utils;
using namespace Common;

namespace Thread {

/**
 * @brief 事件循环线程封装类
 */
class EventLoopThread : public Noncopyable, public std::enable_shared_from_this<EventLoopThread> {
public:
    using Ptr = std::shared_ptr<EventLoopThread>;
    using WkPtr = std::weak_ptr<EventLoopThread>;
    using ThreadPtr = std::shared_ptr<std::thread>;
    using ThreadInitCb = std::function<void(Net::EventLoopWkPtr)>;

public:
    EventLoopThread(ThreadInitCb cb = nullptr);
    ~EventLoopThread();

public:
    /**
     * @brief  运行事件循环线程
     * @return 运行结果
     */
    bool run();

    /**
     * @brief  退出事件循环线程
     * @return 退出结果
     */
    bool quit();

public:
    /**
     * @brief  获取事件循环
     * @return 获取结果
     * @param  eventLoop 事件循环
     */
    inline bool getEventLoop(Net::EventLoopWkPtr& eventLoop) {
        // 非有效状态下不允许获取事件循环
        if (!m_threadStartFlag || m_threadExitFlag || nullptr == m_eventLoop) {
            return false;
        }

        eventLoop = m_eventLoop;
        return true;
    }

private:
    // 事件循环线程
    ThreadPtr m_thread;

    // 事件循环线程互斥锁
    std::mutex m_mutex;

    // 事件循环线程id
    std::thread::id m_threadId;

    // 事件循环线程初始化回调函数
    ThreadInitCb m_threadInitCb;

    // 事件循环线程运行标志
    std::atomic_bool m_threadStartFlag;

    // 事件循环线程退出标志
    std::atomic_bool m_threadExitFlag;

    // 事件循环
    Net::EventLoopPtr m_eventLoop;
};

}; // namespace Thread