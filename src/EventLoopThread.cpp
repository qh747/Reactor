#include <utility>
#include <condition_variable>
#include "Utils/Logger.h"
#include "Net/EventLoop.h"
#include "Thread/EventLoopThread.h"

namespace Thread {

EventLoopThread::EventLoopThread(std::string id, ThreadInitCb cb)
    : m_id(std::move(id)),
      m_thread(nullptr),
      m_threadInitCb(std::move(cb)),
      m_threadStartFlag(false),
      m_threadExitFlag(false) {

    LOG_DEBUG << "EventLoopThread construct. id: " << m_id;
}

EventLoopThread::~EventLoopThread() {
    if (!m_threadExitFlag) {
        this->quit();
    }

    LOG_DEBUG << "EventLoopThread deconstruct. id: " << m_id << " thread id: " << m_threadId;
}

void EventLoopThread::run() {
    // 校验线程是否重复运行
    if (m_threadStartFlag) {
        LOG_WARN << "Event loop thread run warning. thread is already running. id: " << m_id << " thread id: " << m_threadId;
        return;
    }

    // 创建事件循环线程
    std::condition_variable cv;
    auto weakSelf = this->weak_from_this();
    m_thread = std::make_shared<std::thread>([weakSelf, &cv]() {
        auto threadId = std::this_thread::get_id();
        auto strongSelf = weakSelf.lock();

        if (nullptr == strongSelf) {
            LOG_ERROR << "Event loop thread run error. event loop thread invalid. thread id: " << threadId;
            return;
        }

        // 创建事件循环
        {
            std::lock_guard<std::mutex> lock(strongSelf->m_mutex);

            strongSelf->m_eventLoop = std::make_shared<Net::EventLoop>(strongSelf->m_id + "-EV_LOOP_1");
            if (!strongSelf->m_eventLoop->init()) {
                LOG_ERROR << "Event loop thread run error. event loop init failed. id: " << strongSelf->m_id
                          << " thread id: " << threadId;
                return;
            }

            // 运行初始化函数
            if (nullptr != strongSelf->m_threadInitCb) {
                strongSelf->m_threadInitCb(strongSelf->m_eventLoop);
            }
        }

        // 运行事件循环
        if (!strongSelf->m_eventLoop->loop()) {
            LOG_ERROR << "Event loop thread run error. event loop loop failed. id: " << strongSelf->m_id
                      << " thread id: " << threadId;
        }

        // 通知主线程eventloop创建完成
        strongSelf->m_threadId = threadId;
        cv.notify_one();
    });

    {
        // 等待eventloop创建完成
        std::unique_lock<std::mutex> lock(m_mutex);
        cv.wait(lock);
    }

    m_threadStartFlag = true;
    LOG_DEBUG << "Event loop thread running. id: " << m_id << " thread id: " << m_threadId;
}

void EventLoopThread::quit() {
    // 校验线程是否已经退出
    if (m_threadExitFlag) {
        LOG_WARN << "Event loop thread quit warning. thread is already quit. id: " << m_id << " thread id: " << m_threadId;
        return;
    }

    // 退出事件循环
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (nullptr != m_eventLoop) {
            m_eventLoop->quit();
            m_eventLoop.reset();
        }
    }

    // 等待线程退出
    if (nullptr != m_thread) {
        if (m_thread->joinable()) {
            m_thread->join();
        }

        m_thread.reset();
    }

    m_threadExitFlag = true;
    LOG_INFO << "Event loop thread stopped. id: " << m_id << " thread id: " << m_threadId;
}

} // namespace Thread