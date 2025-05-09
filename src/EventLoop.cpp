#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <sys/eventfd.h>
#include "Common/ConfigDef.h"
#include "Utils/Logger.h"
#include "Utils/Timer.h"
#include "Factory/PollerFactory.h"
#include "Net/Channel.h"
#include "Net/Poller.h"
#include "Net/EventLoop.h"
using namespace Common;
using namespace Factory;

namespace Net {

EventLoop::EventLoop(std::thread::id threadId) 
    : m_threadId(threadId), 
      m_running(false),
      m_waiting(false),
      m_poller(nullptr),
      m_wakeupChannel(nullptr) {
    LOG_DEBUG << "Eventloop construct. thread id: " << m_threadId;
}

EventLoop::~EventLoop() {
    LOG_DEBUG << "Eventloop deconstruct. thread id: " << m_threadId;

    if (nullptr != m_wakeupChannel) {
        m_wakeupChannel->close();
    }

    ::close(m_wakeupChannel->getFd());
}

bool EventLoop::init() {
    // 防止重复初始化
    if (m_running) {
        LOG_WARN << "Eventloop init warning. initialized already. thread id: " << m_threadId;
        return true;
    }

    // 创建流程
    {
        // 创建用于唤醒eventLoop的channel
        int wakeupFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (wakeupFd < 0) {
            LOG_ERROR << "Eventloop init error. create wakeup fd failed. thread id: " << m_threadId
                      << " errno: " << errno << ", error: " << strerror(errno);
            return false;
        }
        m_wakeupChannel = std::make_shared<Channel>(this->weak_from_this(), wakeupFd);

        // 创建定时器队列
        m_timerQueue = std::make_shared<TimerQueue>(this->weak_from_this());

        // 创建I/O多路复用封装对象
        m_poller = PollerFactory::CreatePoller(POLLER_DEFAULT_TYPE, this->weak_from_this());

        if (nullptr == m_poller) {
            LOG_ERROR << "Eventloop init error. create poller failed. thread id: " << m_threadId;
            return false;
        }
    }
    
    // 初始化流程
    {
        // 初始化用于唤醒eventLoop的channel
        auto weakSelf = this->weak_from_this();
        m_wakeupChannel->setEventCb(Event_t::EvTypeRead, [weakSelf](Timestamp recvTime) {
            (void)recvTime;

            if (weakSelf.expired()) {
                LOG_ERROR << "Eventloop wakeup error. eventloop expired. thread id: " << std::this_thread::get_id();
                return;
            }

            // 读取数据并非主要目的，主要目的用于唤醒处于poll()等待的eventLoop
            auto strongSelf = weakSelf.lock();
            uint64_t data = 0;
            if (::read(strongSelf->m_wakeupChannel->getFd(), &data, sizeof(data)) < sizeof(data)) {
                LOG_ERROR << "Eventloop wakeup error. read failed. thread id: " << strongSelf->m_threadId
                          << " errno: " << errno << ", error: " << strerror(errno);
            }
        });

        m_wakeupChannel->open(Event_t::EvTypeRead);

        // 初始化定时器队列
        if (!m_timerQueue->init()) {
            LOG_ERROR << "Eventloop init error. timer queue init failed. thread id: " << m_threadId;
            return false;
        }
    }
    
    return true;
}

bool EventLoop::loop() {
    // 防止重复启动事件循环
    if (m_running) {
        LOG_WARN << "Eventloop loop warning. looped already. thread id: " << m_threadId;
        return true;
    }

    LOG_INFO << "Eventloop start. thread id: " << m_threadId;

    // 启动事件循环
    m_running = true;
    while (m_running) {
        // 事件相关参数重置
        int errCode = 0;
        m_activeChannels.clear();

        // 等待事件触发
        m_waiting = true;
        Timestamp returnTime = m_poller->poll(POLLER_DEFAULT_WAIT_TIME, m_activeChannels, errCode);
        if (0 != errCode) {
            if (EINTR != errCode && ETIMEDOUT != errCode) {
                LOG_ERROR << "Eventloop loop error. poll failed. thread id: " << m_threadId
                          << " errno: " << errno << ", error: " << strerror(errno);
                return false;
            }
            else {
                continue;
            }
        }
    
        // 处理事件
        for (const auto& channelWrapper : m_activeChannels) {
            channelWrapper->m_channel->handleEvent(channelWrapper->m_activeEvType, returnTime);
        }

        // 处理其他EventLoop分配给当前EventLoop的任务
        this->handleTask();
        m_waiting = false;
    }

    LOG_INFO << "Eventloop stop. thread id: " << m_threadId;
    return true;
}

bool EventLoop::quit() {
    // 防止重复退出事件循环
    if (!m_running) {
        LOG_WARN << "Eventloop quit warning. quited already. thread id: " << m_threadId;
        return true;
    }

    // 修改事件循环运行状态标志位
    m_running = false;

    // 如果当前处于poll()等待或退出其他线程的事件循环时，则唤醒
    if (m_waiting || !this->isInCurrentThread()) {
        if (!wakeup()) {
            LOG_ERROR << "Eventloop quit error. wakeup failed. thread id: " << m_threadId;
            return false;
        }
    }

    return true;
}

bool EventLoop::wakeup() const {
    uint64_t data = 1;
    if (::write(m_wakeupChannel->getFd(), &data, sizeof(data)) < sizeof(data)) {
        LOG_ERROR << "Eventloop wakeup error. write failed. thread id: " << m_threadId
                  << " errno: " << errno << ", error: " << strerror(errno);
        return false;
    }
    return true;
}

bool EventLoop::updateChannel(const ChannelPtr& channel) const {
    // 判断channel是否有效
    if (nullptr == channel) {
        LOG_ERROR << "Eventloop update channel error. channel invalid. thread id: " << m_threadId;
        return false;
    }

    // 判断channel是否与当前eventLoop关联
    auto channelOwnerLoop = channel->getOwnerLoop();
    if (nullptr == channel || channelOwnerLoop.expired() || channelOwnerLoop.lock()->getThreadId() != m_threadId) {
        LOG_ERROR << "Eventloop update channel error. channel owner event loop invalid. thread id: " << m_threadId;
        return false;
    }

    // 更新channel
    if (!m_poller->updateChannel(channel)) {
        LOG_ERROR << "Eventloop update channel error. update poller channel failed. thread id: " << m_threadId;
        return false;
    }
    return true;
}

bool EventLoop::removeChannel(const ChannelPtr& channel) const {
    // 判断channel是否有效
    if (nullptr == channel) {
        LOG_ERROR << "Eventloop remove channel error. channel invalid. thread id: " << m_threadId;
        return false;
    }

    // 判断channel是否与当前eventLoop关联
    auto channelOwnerLoop = channel->getOwnerLoop();
    if (nullptr == channel || channelOwnerLoop.expired() || channelOwnerLoop.lock()->getThreadId() != m_threadId) {
        LOG_ERROR << "Eventloop remove channel error. channel owner event loop invalid. thread id: " << m_threadId;
        return false;
    }

    // 移除channel
    if (!m_poller->removeChannel(channel)) {
        LOG_ERROR << "Eventloop remove channel error. remove poller channel failed. thread id: " << m_threadId;
        return false;
    }
    return true;
}

bool EventLoop::executeTask(const Task& task) {
    // 任务有效性校验
    if (nullptr == task) {
        LOG_ERROR << "Eventloop execute task error. task invalid. thread id: " << m_threadId;
        return false;
    }

    // 任务执行
    if (this->isInCurrentThread()) {
        // 本线程的任务则直接执行
        task();
    }
    else {
        // 其他线程的任务则以高优先级缓存到当前EventLoop的任务队列中
        this->executeTaskInLoop(task, true);
    }

    return true;
}

bool EventLoop::executeTaskInLoop(const Task& task, bool highPriority) {
    // 任务有效性校验
    if (nullptr == task) {
        LOG_ERROR << "Eventloop execute task in loop error. task invalid. thread id: " << m_threadId;
        return false;
    }

    // 缓存任务
    {
        std::lock_guard<std::mutex> lock(m_taskMutex);

        if (highPriority) {
            m_taskList.push_front(task);
        }
        else {
            m_taskList.push_back(task);
        }
    }

    // 唤醒当前EventLoop所在线程，以便处理任务
    if (!this->isInCurrentThread() || m_waiting) {
        // 如果当前线程不是EventLoop所在线程或者当前EventLoop正在等待，则唤醒
        if (!this->wakeup()) {
            LOG_ERROR << "Eventloop execute task in loop error. wakeup failed. thread id: " << m_threadId;
        }
    }

    return true;
}

bool EventLoop::addTimerAtSpecificTime(TimerQueue::TimerId& id, TimerTask::Task cb, Timestamp expires, double intervalSec) const {
    return m_timerQueue->addTimerTask(id, std::move(cb), expires, intervalSec);
}

bool EventLoop::addTimerAfterSpecificTime(TimerQueue::TimerId& id, TimerTask::Task cb, double delay, double intervalSec) const {
    auto firstRunTime = std::chrono::system_clock::now() + std::chrono::milliseconds(static_cast<int64_t>(delay * 1000));
    return m_timerQueue->addTimerTask(id, std::move(cb), firstRunTime, intervalSec);
}

bool EventLoop::delTimer(TimerId id) const {
    return m_timerQueue->delTimerTask(id);
}

bool EventLoop::handleTask() {
    TaskList currentTaskList;

    {
        std::lock_guard<std::mutex> lock(m_taskMutex);

        // 将当前EventLoop分配的任务转移到currentTaskList
        currentTaskList.swap(m_taskList);
    }

    // 处理当前EventLoop分配的任务
    for (const auto& task : currentTaskList) {
        task();
    }

    return true;
}

} // namespace Net