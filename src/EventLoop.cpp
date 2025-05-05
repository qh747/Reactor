#include <random>
#include <cstdint>
#include <cstring>
#include <unordered_set>
#include <unordered_map>
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

/** --------------------------  EventLoopIdGenerator ----------------------------------- */

/**
 * @brief 事件循环Id生成器
 */
class EventLoopIdGenerator : public NoncopyableConstructable {
public:
    /**
     * @brief  生成id
     * @return id
     * @param  prefix id前缀
     */
    static std::string GenerateId(const std::string& prefix) {
        std::lock_guard<std::mutex> lock(Mutex);

        int generateCount = 0;
        while (true) {
            if (generateCount++ >= 3) {
                LOG_FATAL << "Generate unique id error. generate conflict id more than 3 times.";
                break;
            }

            std::string id = prefix + "_" + GetRandom(32) + "_" + GetIdKey();
            if (IdSet.end() == IdSet.find(id)) {
                IdSet.insert(id);
                return id;
            }
        }

        return "";
    }

private:
    static std::string GetRandom(std::size_t length) {
        const std::string charset = 
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
    
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<> distribution(0, charset.size() - 1);
        
        std::string result;
        result.reserve(length);
        
        for (size_t i = 0; i < length; ++i) {
            result += charset[distribution(generator)];
        }

        return result;
    }

    static std::string GetIdKey() {
        if (IdKey.load() >= UINT64_MAX) {
            IdKey.store(0);
        }

        return std::to_string(IdKey++);
    }

private:
    static std::mutex Mutex;
    static std::atomic<uint64_t> IdKey;
    static std::unordered_set<std::string> IdSet;
};

std::mutex EventLoopIdGenerator::Mutex;
std::atomic<uint64_t> EventLoopIdGenerator::IdKey {0};
std::unordered_set<std::string> EventLoopIdGenerator::IdSet;

/** --------------------------  EventLoop ----------------------------------- */

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
        m_poller = PollerFactory::CreatePoller(POLLER_DEFAULT_TYPE, this->weak_from_this(), 
        EventLoopIdGenerator::GenerateId("EVENT_LOOP_POLLER_"));

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
        Timestamp returnTime;
        m_activeChannels.clear();

        // 事件循环流程
        {
            // 等待事件触发
            m_waiting = true;
            returnTime = m_poller->poll(POLLER_DEFAULT_WAIT_TIME, m_activeChannels, errCode);
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
        wakeup();
    }

    return true;
}

bool EventLoop::wakeup() {
    uint64_t data = 1;
    if (::write(m_wakeupChannel->getFd(), &data, sizeof(data)) < sizeof(data)) {
        LOG_ERROR << "Eventloop wakeup error. write failed. thread id: " << m_threadId
                  << " errno: " << errno << ", error: " << strerror(errno);
    }
    return true;
}

bool EventLoop::updateChannel(ChannelPtr channel) {
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

bool EventLoop::removeChannel(ChannelPtr channel) {
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

bool EventLoop::executeTask(Task task) {
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

bool EventLoop::executeTaskInLoop(Task task, bool highPriority) {
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
        this->wakeup();
    }

    return true;
}

bool EventLoop::addTimerAtSpecificTime(TimerQueue::TimerId& id, TimerTask::Task cb, Timestamp expires, double intervalSec) {
    return m_timerQueue->addTimerTask(id, cb, expires, intervalSec);
}

bool EventLoop::addTimerAfterSpecificTime(TimerQueue::TimerId& id, TimerTask::Task cb, double delay, double intervalSec) {
    auto firstRunTime = std::chrono::system_clock::now() + std::chrono::milliseconds(static_cast<int64_t>(delay * 1000));
    return m_timerQueue->addTimerTask(id, cb, firstRunTime, intervalSec);
}

bool EventLoop::delTimer(TimerId id) {
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

/** --------------------------  EventLoopManager ----------------------------------- */

class EventLoopManager::ManageObject {
public:
    ~ManageObject() {
        std::lock_guard<std::mutex> lock(m_mutex);

        for (auto& it : m_eventLoopMap) {
            if (it.second->isRunning()) {
                it.second->quit();
            }
        }
        m_eventLoopMap.clear();
    }

public:
    std::mutex m_mutex;
    std::unordered_map<std::thread::id, EventLoop::Ptr> m_eventLoopMap;
};

static EventLoopManager::ManageObject EventLoopManageObject;

EventLoop::Ptr EventLoopManager::GetCurrentEventLoop() {
    std::lock_guard<std::mutex> lock(EventLoopManageObject.m_mutex);

    auto threadId = std::this_thread::get_id();
    auto it = EventLoopManageObject.m_eventLoopMap.find(threadId);
    if (EventLoopManageObject.m_eventLoopMap.end() != it) {
        // eventloop已存在，直接返回
        return it->second;
    }
    else {
        // eventloop不存在，创建一个新的eventloop
        EventLoop::Ptr eventLoop = std::make_shared<EventLoop>(threadId);
        if (!eventLoop->init()) {
            LOG_FATAL << "Get eventLoop error. initialize eventLoop error. thread id: " << threadId;
        }

        EventLoopManageObject.m_eventLoopMap[threadId] = eventLoop;
        return eventLoop;
    }
}

} // namespace Net