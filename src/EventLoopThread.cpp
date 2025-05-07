#include "Utils/Logger.h"
#include "Net/EventLoop.h"
#include "Thread/EventLoopThread.h"

namespace Thread {

EventLoopThread::EventLoopThread(ThreadInitCb cb)
    : m_thread(nullptr),
      m_threadInitCb(cb),
      m_threadStartFlag(false),
      m_threadExitFlag(false) {
}
    
EventLoopThread::~EventLoopThread() {
    if (!m_threadExitFlag) {
        this->quit();
    }
}

bool EventLoopThread::run() {
    // 校验线程是否重复运行
    if (m_threadStartFlag) {
        LOG_WARN << "Event loop thread run warning. thread is already running. thread id: " << m_threadId;
        return true;
    }
    else {
        m_threadStartFlag = true;
    }

    // 创建事件循环线程
    auto weakSelf = this->weak_from_this();
    m_thread = std::make_shared<std::thread>([weakSelf]() {
        auto threadId = std::this_thread::get_id();
        auto strongSelf = weakSelf.lock();

        if (nullptr == strongSelf) {
            LOG_ERROR << "Event loop thread run error. event loop thread invalid. thread id: " << threadId;
            return;
        }

        // 创建事件循环
        {
            std::lock_guard<std::mutex> lock(strongSelf->m_mutex);

            strongSelf->m_eventLoop = std::make_shared<Net::EventLoop>(std::this_thread::get_id());
            if (!strongSelf->m_eventLoop->init()) {
                LOG_ERROR << "Event loop thread run error. event loop init failed. thread id: " << threadId;
                return;
            }

            // 运行初始化函数
            if (nullptr != strongSelf->m_threadInitCb) {
                strongSelf->m_threadInitCb(strongSelf->m_eventLoop);
            }
        }

        // 运行事件循环
        if (!strongSelf->m_eventLoop->loop()) {
            LOG_ERROR << "Event loop thread run error. event loop loop failed. thread id: " << threadId;
            return;
        }
    });

    m_threadId = m_thread->get_id();
    LOG_INFO << "Event loop thread runnning. thread id: " << m_threadId;
    return true;
}

bool EventLoopThread::quit() {
    // 校验线程是否已经退出
    if (m_threadExitFlag) {
        LOG_WARN << "Event loop thread quit warning. thread is already quit. thread id: " << m_threadId;
        return true;
    }
    else {
        m_threadExitFlag = true;
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

    LOG_INFO << "Event loop thread stopped. thread id: " << m_threadId;
    return true;
}

} // namespace Thread