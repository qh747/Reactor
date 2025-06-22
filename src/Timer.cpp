#include <chrono>
#include <cstring>
#include <utility>
#include <unistd.h>
#include <sys/timerfd.h>
#include "Utils/Logger.h"
#include "Net/Channel.h"
#include "Net/EventLoop.h"
#include "Utils/Timer.h"

namespace Utils {

/** -------------------------------- TimerTask ------------------------------------- */

static std::atomic<uint64_t> TimerIdCounter{0};

TimerTask::TimerTask(Task cb, Timestamp expires, double intervalSec)
    : m_id(TimerIdCounter++),
      m_cb(std::move(cb)),
      m_expires(expires),
      m_intervalSec(intervalSec),
      m_repeat(intervalSec > 0.0) {
}

bool TimerTask::executeTask() const {
    if (nullptr == m_cb) {
        LOG_ERROR << "Execute timer task error. timer task callback invalid. id: " << m_id;
        return false;
    }

    m_cb();
    return true;
}

bool TimerTask::reset() {
    if (!m_repeat) {
        LOG_ERROR << "Reset timer task error. timer task not repeat. id: " << m_id;
    }

    auto intervalMs = std::chrono::duration<double, std::milli>(m_intervalSec);
    auto intervalDuration = std::chrono::duration_cast<std::chrono::system_clock::duration>(intervalMs);
    m_expires += intervalDuration;
    return true;
}

/** -------------------------------- TimerQueue ------------------------------------- */

TimerQueue::TimerQueue(EventLoop::WkPtr loop, std::string id)
    : m_id(std::move(id)),
      m_isInit(false),
      m_ownerLoop(std::move(loop)),
      m_timerChannel(nullptr),
      m_isHandleTask(false) {
    LOG_DEBUG << "Timer queue construct. id: " << m_id;
}

TimerQueue::~TimerQueue() {
    if (nullptr != m_timerChannel) {
        m_timerChannel->close();
    }

    ::close(m_timerChannel->getFd());
    LOG_DEBUG << "Timer queue deconstruct. id: " << m_id;
}

bool TimerQueue::addTimerTask(TimerId& id, const TimerTask::Task& cb, Timestamp expires, double intervalSec) {
    if (nullptr == cb) {
        LOG_ERROR << "Add timer task error. timer task callback invalid. id: " << m_id;
        return false;
    }

    if (nullptr == m_timerChannel) {
        LOG_ERROR << "Add timer task error. timer channel invalid. id: " << m_id;
        return false;
    }

    // 创建定时器任务
    TimerTask::Ptr task = std::make_shared<TimerTask>(cb, expires, intervalSec);
    id = task->getId();

    std::string timerQueueId = m_id;
    auto weakSelf = this->weak_from_this();
    m_ownerLoop.lock()->executeTask([weakSelf, task, timerQueueId]() {
        if (weakSelf.expired()) {
            LOG_ERROR << "Timer queue add task error. timer queue expired. id: " << timerQueueId;
            return;
        }

        // 判断是否需要重置定时器的超时时间
        auto strongSelf = weakSelf.lock();
        auto resetFlag = strongSelf->m_timerTasks.empty() ?
            true :
            strongSelf->m_timerTasks.begin()->get()->getExpires() < task->getExpires() ? true : false;

        // 添加定时器任务
        strongSelf->m_timerTasks.insert(task);

        // 重置定时器的超时时间
        if (resetFlag && !strongSelf->resetExpiredTimerTask()) {
            LOG_ERROR << "Timer queue add task error. reset expired timer task failed. id: " << timerQueueId;
        }
    });

    return true;
}

bool TimerQueue::delTimerTask(TimerId id) {
    if (nullptr == m_timerChannel) {
        LOG_ERROR << "Delete timer task error. timer channel invalid. id: " << m_id;
        return false;
    }

    std::string timerQueueId = m_id;
    auto weakSelf = this->weak_from_this();
    m_ownerLoop.lock()->executeTask([weakSelf, id, timerQueueId]() {
        if (weakSelf.expired()) {
            LOG_ERROR << "Timer queue delete task error. timer queue expired. id: " << timerQueueId;
            return;
        }

        auto strongSelf = weakSelf.lock();
        for (auto iter = strongSelf->m_timerTasks.begin(); iter != strongSelf->m_timerTasks.end(); ++iter) {
            if ((*iter)->getId() != id) {
                continue;
            }

            if (strongSelf->m_isHandleTask) {
                strongSelf->m_cancelTimerTasks.insert(*iter);
            }

            strongSelf->m_timerTasks.erase(iter);
            break;
        }
    });

    return true;
}

bool TimerQueue::init() {
    // 防止重复初始化
    if (m_isInit) {
        LOG_WARN << "Timer queue init warning. initialized already. id: " << m_id;
        return true;
    }
    else {
        m_isInit = true;
    }

    // 创建timer channel
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        LOG_ERROR << "Timer queue init error. timer fd create failed. id: " << m_id << " errno: " << errno << " error info: " << strerror(errno);
        return false;
    }
    m_timerChannel = std::make_shared<Channel>(m_ownerLoop, timerfd);

    // 设置timer channel的读事件回调函数
    std::string id = m_id;
    auto weakSelf = this->weak_from_this();
    m_timerChannel->setEventCb(Event_t::EvTypeRead, [weakSelf, id](Timestamp recvTime) {
        (void)recvTime;

        if (weakSelf.expired()) {
            LOG_ERROR << "Timer queue handle task error. timer queue expired. id: " << id;
            return;
        }

        // 读取数据并非主要目的，主要目的用于唤醒处于poll()等待的eventLoop
        auto strongSelf = weakSelf.lock();
        if (!strongSelf->handleTask()) {
            LOG_ERROR << "Timer queue handle task error. id: " << id;
        }
    });

    if (!m_timerChannel->open(Event_t::EvTypeRead)) {
        LOG_ERROR << "Timer queue init error. timer channel open failed. id: " << m_id;
        return false;
    }

    LOG_DEBUG << "Timer queue init success. id: " << m_id;
    return true;
}

bool TimerQueue::handleTask() {
    // 读取数据(读取数据并非主要目的，主要目的用于处理超时的定时器任务)
    uint64_t data = 1;
    if (::read(m_timerChannel->getFd(), &data, sizeof(data)) < sizeof(data)) {
        LOG_ERROR << "Timer queue handle task error. read failed. id: " << m_id << " errno: " << errno << ", error: " << strerror(errno);
        return false;
    }

    // 移除待取消的定时器
    m_cancelTimerTasks.clear();

    // 执行超时的定时器任务
    m_isHandleTask = true;

    TimerTasks expiredTasks;
    auto now = std::chrono::system_clock::now();
    if (!this->getExpiredTasks(now, expiredTasks)) {
        for (auto& task : expiredTasks) {
            if (!task->executeTask()) {
                LOG_WARN << "Timer queue handle task warning. execute task failed. task id: " << task->getId() << " id: " << m_id;
                continue;
            }

            // 重置定时器任务
            if (task->isRepeat()) {
                task->reset();
                m_timerTasks.insert(task);
            }
        }
    }

    // 重置timer channel下次超时时间
    if (!m_timerTasks.empty() && !this->resetExpiredTimerTask()) {
        LOG_ERROR << "Timer queue handle task error. reset expired timer task failed. id: " << m_id;
        return false;
    }

    m_isHandleTask = false;
    return true;
}

bool TimerQueue::getExpiredTasks(Timestamp expired, TimerTasks& expiredTasks) {
    expiredTasks.clear();

    // 查找超时的定时器任务
    auto iter = m_timerTasks.begin();
    while (iter != m_timerTasks.end() && (*iter)->getExpires() <= expired) {
        expiredTasks.insert(*iter);
        ++iter;
    }

    // 删除超时的定时器任务
    if (expiredTasks.empty()) {
        return false;
    }
    else {
        m_timerTasks.erase(m_timerTasks.begin(), iter);
        return true;
    }
}

bool TimerQueue::resetExpiredTimerTask() const {
    auto nextExpired = m_timerTasks.begin()->get()->getExpires();
    auto nextExpireDuration = std::chrono::duration_cast<std::chrono::system_clock::duration>(nextExpired - std::chrono::system_clock::now());
    auto nextExpiredMs = std::chrono::duration_cast<std::chrono::milliseconds>(nextExpireDuration);

    itimerspec spec = {};
    spec.it_value.tv_sec = nextExpiredMs.count() / 1000;
    spec.it_value.tv_nsec = (nextExpiredMs.count() % 1000) * 1000000;
    if (::timerfd_settime(m_timerChannel->getFd(), 0, &spec, nullptr) < 0) {
        LOG_ERROR << "Timer queue handle task error. timer fd settime failed. id: " << m_id << " errno: " << errno
            << " error info: " << strerror(errno);
        return false;
    }

    return true;
}

} // namespace Utils