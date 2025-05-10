#include "Utils/Logger.h"
#include "Thread/EventLoopThread.h"
#include "Thread/EventLoopThreadPool.h"

namespace Thread {

static uint64_t EVENT_LOOP_THREAD_POOL_ID_KEY = 0;

EventLoopThreadPool::EventLoopThreadPool(unsigned int numWorkThreads, const ThreadInitCb& cb)
    : m_id("EVENT_LOOP_THREAD_POOL_" + std::to_string(++EVENT_LOOP_THREAD_POOL_ID_KEY)) {
    // 创建线程
    {
        // 创建主线程
        m_eventLoopMainThread = std::make_shared<EventLoopThread>(cb);

        // 创建工作线程
        for (unsigned int i = 0; i < numWorkThreads; ++i) {
            m_eventLoopWorkThreads.emplace_back(std::make_shared<EventLoopThread>(cb));
        }
    }

    // 运行线程
    {
        // 运行主线程
        m_eventLoopMainThread->run();

        // 运行工作线程
        for (auto& loop : m_eventLoopWorkThreads) {
            loop->run();
        }
    }

    LOG_DEBUG << "EventLoopThreadPool construct. numThreads: " << numWorkThreads << ". id: " << m_id;
}

EventLoopThreadPool::~EventLoopThreadPool() {
    LOG_DEBUG << "EventLoopThreadPool deconstruct. numThreads: " << m_eventLoopWorkThreads.size() << ". id: " << m_id;

    // 退出主线程
    m_eventLoopMainThread->quit();

    // 退出工作线程
    for (auto& loop : m_eventLoopWorkThreads) {
        loop->quit();
    }
}

bool EventLoopThreadPool::getMainEventLoop(Net::EventLoopWkPtr& eventLoop) const {
    if (nullptr == m_eventLoopMainThread) {
        LOG_ERROR << "Get main event loop error. main thread invalid. id" << m_id;
        return false;
    }

    return true;
}

bool EventLoopThreadPool::getNextWorkEventLoop(Net::EventLoopWkPtr& eventLoop) const {
    if (m_eventLoopWorkThreads.empty()) {
        LOG_ERROR << "Get next work event loop error. work thread invalid. id" << m_id;
        return false;
    }

    // 获取有效工作线程索引
    auto validNextIdx = (m_nextIdx >= m_eventLoopWorkThreads.size() ? 0 : m_nextIdx++);

    // 获取工作线程
    auto eventLoopWorkThread = m_eventLoopWorkThreads[validNextIdx];
    if (nullptr == eventLoopWorkThread) {
        LOG_ERROR << "Get next work event loop error. work thread invalid. id" << m_id;
        return false;
    }

    // 获取事件循环
    if (!eventLoopWorkThread->getEventLoop(eventLoop)) {
        LOG_ERROR << "Get next work event loop error. get event loop failed. id" << m_id;
        return false;
    }
    return true;
}

} // namespace Thread