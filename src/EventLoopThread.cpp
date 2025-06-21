#include <utility>
#include <condition_variable>
#include "Common/ConfigDef.h"
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
    std::atomic_bool waitReady(false);

    auto weakSelf = this->weak_from_this();
    m_thread = std::make_shared<std::thread>([weakSelf, &cv, &waitReady]() {
        auto threadId = std::this_thread::get_id();
        auto strongSelf = weakSelf.lock();

        if (nullptr == strongSelf) {
            LOG_ERROR << "Event loop thread run error. event loop thread invalid. thread id: " << threadId;
            return;
        }

        // 创建事件循环
        {
            std::stringstream ss;
            ss << PREFIX_SIGN << EV_LOOP_PREFIX << "1";

            // 确保等待线程先获得互斥锁
            while (!waitReady) {
                std::this_thread::yield();
            }

            std::lock_guard<std::mutex> lock(strongSelf->m_mutex);

            strongSelf->m_eventLoop = std::make_shared<Net::EventLoop>(strongSelf->m_id + ss.str());
            if (!strongSelf->m_eventLoop->init()) {
                LOG_ERROR << "Event loop thread run error. event loop init failed. id: " << strongSelf->m_id
                    << " thread id: " << threadId;
                return;
            }

            // 运行初始化函数
            if (nullptr != strongSelf->m_threadInitCb) {
                strongSelf->m_threadInitCb(strongSelf->m_eventLoop);
            }

            strongSelf->m_threadId = threadId;
        }

        // 通知主线程事件循环创建完成
        cv.notify_one();

        // 运行事件循环
        if (!strongSelf->m_eventLoop->loop()) {
            LOG_ERROR << "Event loop thread run error. event loop loop failed. id: " << strongSelf->m_id
                << " thread id: " << threadId;
        }
    });

    {
        // 等待eventloop创建完成
        std::unique_lock<std::mutex> lock(m_mutex);
        waitReady = true;
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