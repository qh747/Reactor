#include <cstring>
#include <unistd.h>
#include "Common/ConfigDef.h"
#include "Utils/Logger.h"
#include "Net/Channel.h"
#include "Net/EventLoop.h"
#include "Net/EpPoller.h"
using namespace Common;
using namespace Utils;

namespace Net {

EpPoller::EpPoller(EventLoop::WkPtr loop)
    : Poller(std::move(loop), "EPOLL_"),
      m_epollFd(epoll_create1(EPOLL_CLOEXEC)),
      m_epollEventList(POLL_INIT_WAIT_EVENTS_SIZE) {
    // 创建失败，程序退出
    if (m_epollFd < 0) {
        LOG_FATAL << "Construct epoll poller error. id: " << m_id << " code: " << errno << ". msg: " << strerror(errno);
    }
    LOG_DEBUG << "Epoll poller construct. id: " << m_id;
}

EpPoller::~EpPoller() {
    close(m_epollFd);
    LOG_DEBUG << "Epoll poller deconstruct. id: " << m_id;
}

Timestamp EpPoller::poll(int timeoutMs, ChannelWrapperList& activeChannels, int& errCode) {
    int activeEventSize = ::epoll_wait(m_epollFd, m_epollEventList.data(), static_cast<int>(m_epollEventList.size()), timeoutMs);
    auto now = std::chrono::system_clock::now();
    if (activeEventSize < 0) {
        if (errno == EINTR) {
            // 外部中断
            errCode = EINTR;
            LOG_WARN << "Epoll poll warning. external interrupt. id: " << m_id << " code: " << errno << ". msg: " << strerror(errno);
        }
        else {
            // epoll_wait()出错
            errCode = errno;
            LOG_FATAL << "Epoll poll error. id: " << m_id << " code: " << errno << ". msg: " << strerror(errno);
        }
    }
    else if (0 == activeEventSize) {
        // epoll_wait()超时
        errCode = ETIMEDOUT;
        LOG_WARN << "Epoll poll warning. timeout. id: " << m_id << " code: " << errno << ". msg: " << strerror(errno);
    }
    else {
        // 处理活跃的channel
        for (int idx = 0; idx < activeEventSize; ++idx) {
            const auto& event = m_epollEventList[idx];
            const auto& channelMapIter = m_channelMap.find(event.data.fd);
            if (m_channelMap.end() == channelMapIter) {
                LOG_ERROR << "Epoll poll error. channel not found. id: " << m_id << " fd: " << event.data.fd << ".";
                continue;
            }

            // 添加活跃的channel
            auto evType = EventHelper::ConvertToEventType(event.events);
            activeChannels.emplace_back(std::make_shared<ChannelWrapper>(channelMapIter->second, evType));

            LOG_DEBUG << "Epoll poll success. id: " << m_id << " fd: " << event.data.fd << " event type: "
                << StringHelper::EventTypeToString(evType) << ".";
        }

        // 判断是否需要对epoll event列表扩容
        if (activeEventSize >= m_epollEventList.size()) {
            m_epollEventList.resize(m_epollEventList.size() * 2);
        }
    }

    return now;
}

bool EpPoller::updateChannel(Channel::Ptr channel) {
    if (nullptr == channel) {
        LOG_ERROR << "Update channel error. channel invalid. id: " << m_id << ".";
        return false;
    }

    // 添加channel
    int fd = channel->getFd();
    if (m_channelMap.end() == m_channelMap.find(fd)) {
        m_channelMap[fd] = channel;
    }

    // 更新类型判断
    State_t state = channel->getState();
    Event_t evType = channel->getEvType();

    if (State_t::StatePending == state || State_t::StateNotInLoop == state) {
        // 更新channel状态
        state = State_t::StateInLoop;
        channel->setState(state);

        // epoll更新
        if (!this->operateControl(fd, evType, PollerCtrl_t::PollerAdd)) {
            LOG_ERROR << "Update channel error. id: " << m_id << " fd: " << fd << " state: " << StringHelper::StateTypeToString(state);
        }
    }
    else if (State_t::StateInLoop == state) {
        if (Event_t::EvTypeNone == evType) {
            // 更新channel状态
            state = State_t::StateNotInLoop;
            channel->setState(state);

            // epoll更新
            if (!this->operateControl(fd, evType, PollerCtrl_t::PollerRemove)) {
                LOG_ERROR << "Update channel error. id: " << m_id << " fd: " << fd << " state: " << StringHelper::StateTypeToString(state);
            }
        }
        else {
            if (!this->operateControl(fd, evType, PollerCtrl_t::PollerModify)) {
                LOG_ERROR << "Update channel error. id: " << m_id << " fd: " << fd << " state: " << StringHelper::StateTypeToString(state);
            }
        }
    }
    else {
        LOG_ERROR << "Update channel error. channel state invalid. id: " << m_id << " fd: " << fd
            << " state: " << StringHelper::StateTypeToString(state) << " event type: " << StringHelper::EventTypeToString(evType) << ".";
        return false;
    }

    LOG_INFO << "Update channel success. id: " << m_id << " fd: " << fd << " state: " << StringHelper::StateTypeToString(state)
        << " event type: " << StringHelper::EventTypeToString(evType) << ".";
    return true;
}

bool EpPoller::removeChannel(Channel::Ptr channel) {
    if (nullptr == channel) {
        LOG_ERROR << "Remove channel error. channel invalid. id: " << m_id << ".";
        return false;
    }

    // 设置channel状态
    State_t state = channel->getState();
    channel->setState(State_t::StatePending);

    // epoll移除
    int fd = channel->getFd();
    Event_t evType = channel->getEvType();
    channel->setEvType(Event_t::EvTypeNone);

    bool result = this->operateControl(fd, evType, PollerCtrl_t::PollerRemove);

    // 移除channel
    if (m_channelMap.end() != m_channelMap.find(fd)) {
        m_channelMap.erase(fd);
    }

    if (!result) {
        LOG_ERROR << "Remove channel error. id: " << m_id << " fd: " << fd << " state: " << StringHelper::StateTypeToString(state)
            << " event type: " << StringHelper::EventTypeToString(evType) << ".";
        return false;
    }

    LOG_INFO << "Remove channel success. id: " << m_id << " fd: " << fd << " state: " << StringHelper::StateTypeToString(state)
        << " event type: " << StringHelper::EventTypeToString(evType) << ".";
    return true;
}

bool EpPoller::operateControl(int fd, Event_t ev, PollerCtrl_t op) const {
    epoll_event event = {};
    event.data.fd = fd;
    event.events = static_cast<int>(ev);

    if (::epoll_ctl(m_epollFd, static_cast<int>(op), fd, &event) < 0) {
        if (PollerCtrl_t::PollerRemove == op) {
            LOG_ERROR << "Epoll ctrl error. id: " << m_id << " fd: " << fd << " op: " << StringHelper::PollerCtrlTypeToString(op)
                << " event type: " << StringHelper::EventTypeToString(ev) << ".";
        }
        else {
            LOG_FATAL << "Epoll ctrl error. id: " << m_id << " fd: " << fd << " op: " << StringHelper::PollerCtrlTypeToString(op)
                << " event type: " << StringHelper::EventTypeToString(ev) << ".";
        }

        return false;
    }
    else {
        LOG_INFO << "Epoll ctrl success. id: " << m_id << " fd: " << fd << " op: " << StringHelper::PollerCtrlTypeToString(op)
            << " event type: " << StringHelper::EventTypeToString(ev) << ".";
        return true;
    }
}

} // namespace Net