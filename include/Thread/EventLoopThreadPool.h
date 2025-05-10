#pragma once
#include <string>
#include <memory>
#include <vector>
#include "Utils/Utils.h"
#include "Common/TypeDef.h"
using namespace Utils;

namespace Thread {

/**
 * @brief 事件循环线程池
 */
class EventLoopThreadPool : public Noncopyable, public std::enable_shared_from_this<EventLoopThreadPool> {
public:
    using Ptr = std::shared_ptr<EventLoopThread>;
    using WkPtr = std::weak_ptr<EventLoopThread>;

public:
    explicit EventLoopThreadPool(unsigned int numWorkThreads = 0, const ThreadInitCb& cb = nullptr);
    ~EventLoopThreadPool();

public:
    /**
     * @brief  获取主线程事件循环
     * @return 获取结果
     * @param  eventLoop 事件循环
     */
    bool getMainEventLoop(Net::EventLoopWkPtr& eventLoop) const;

    /**
     * @brief  获取下一个工作线程事件循环
     * @return 获取结果
     * @param  eventLoop 事件循环
     */
    bool getNextWorkEventLoop(Net::EventLoopWkPtr& eventLoop) const;

private:
    // 事件循环线程池id
    const std::string m_id;

    // 事件循环工作循环索引
    mutable unsigned int m_nextIdx{0};

    // 事件循环主线程
    EventLoopThreadPtr m_eventLoopMainThread;

    // 事件循环工作线程列表
    std::vector<EventLoopThreadPtr> m_eventLoopWorkThreads;
};

}; // namespace Thread