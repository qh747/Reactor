#include <cstring>
#include <unistd.h>
#include "Utils/Logger.h"
#include "Net/Channel.h"
#include "Net/EpPoller.h"
using namespace Common;
using namespace Utils;

namespace Net {

EpPoller::EpPoller(EventLoopPtr loop)
    : Poller(loop),
      m_epollFd(epoll_create1(EPOLL_CLOEXEC)),
      m_epollEventList(POLL_INIT_WAIT_EVENTS_SIZE) {
    // 创建失败，程序退出
    if (m_epollFd < 0) {
        LOG_FATAL << "Construct epoll poller error. code: " << errno << ". msg: " << strerror(errno);
    }
}

EpPoller::~EpPoller() {
    close(m_epollFd);
}

Timestamp EpPoller::wait(int timeoutMs, ChannelWrapperList& activeChannels) {

    return std::chrono::system_clock::now();
}

bool EpPoller::updateChannel(Channel::Ptr channel) {
    if (nullptr == channel) {
        LOG_ERROR << "Update channel error. channel invalid. epoll fd: " << m_epollFd << ".";
        return false;
    }

    // 添加channel
    int fd = channel->getFd();
    if(m_channelMap.end() == m_channelMap.find(fd)) {
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
        this->operateControl(fd, evType, EpCtrl_t::EpollAdd);
    }
    else if (State_t::StateInLoop == state) {
        if (Event_t::EvTypeNone == evType) {
            // 更新channel状态
            state = State_t::StateNotInLoop;
            channel->setState(state);

            // epoll更新
            this->operateControl(fd, evType, EpCtrl_t::EpollRemove);
        }
        else {
            this->operateControl(fd, evType, EpCtrl_t::EpollModify);
        }
    }
    else {
        LOG_ERROR << "Update channel error. channel state invalid. epoll fd: " << m_epollFd << " fd: " << fd 
                  << " state: " << StringHelper::StateTypeToString(state) 
                  << " event type: " << StringHelper::EventTypeToString(evType) << ".";
        return false;
    }

    LOG_INFO << "Update channel success. epoll fd: " << m_epollFd << " fd: " << fd << " state: " << StringHelper::StateTypeToString(state) 
             << " event type: " << StringHelper::EventTypeToString(evType) << ".";
    return true;
}

bool EpPoller::removeChannel(Channel::Ptr channel) {
    if (nullptr == channel) {
        LOG_ERROR << "Remove channel error. channel invalid. epoll fd: " << m_epollFd << ".";
        return false;
    }

    // 设置channel状态
    State_t state = channel->getState();
    channel->setState(State_t::StatePending);

    // epoll移除
    int fd = channel->getFd();
    Event_t evType = channel->getEvType();
    bool result = this->operateControl(fd, evType, EpCtrl_t::EpollRemove);

    // 移除channel
    if(m_channelMap.end() != m_channelMap.find(fd)) {
        m_channelMap.erase(fd);
    }

    if (!result) {
        LOG_ERROR << "Remove channel error. epoll fd: " << m_epollFd << " fd: " << fd << " state: " << StringHelper::StateTypeToString(state) 
                  << " event type: " << StringHelper::EventTypeToString(evType) << ".";
        return false;
    }

    LOG_INFO << "Remove channel success. epoll fd: " << m_epollFd << " fd: " << fd << " state: " << StringHelper::StateTypeToString(state) 
             << " event type: " << StringHelper::EventTypeToString(evType) << ".";
    return true;
}

bool EpPoller::operateControl(int fd, Event_t ev, EpCtrl_t op) {
    epoll_event event;
    memset(&event, 0, sizeof(event));

    event.data.fd = fd;
    event.events = static_cast<int>(ev);

    if (epoll_ctl(m_epollFd, static_cast<int>(op), fd, &event) < 0) {
        if (EpCtrl_t::EpollRemove == op) {
            LOG_ERROR << "Epoll ctrl error. epoll fd: " << m_epollFd << " fd: " << fd
                      << " op: " << StringHelper::EpollCtrlTypeToString(op)
                      << " event type: " << StringHelper::EventTypeToString(ev) << ".";
        }
        else {
            LOG_FATAL << "Epoll ctrl error. epoll fd: " << m_epollFd << " fd: " << fd
                      << " op: " << StringHelper::EpollCtrlTypeToString(op)
                      << " event type: " << StringHelper::EventTypeToString(ev) << ".";
        }

        return false;
    }
    else {
        LOG_INFO << "Epoll ctrl success. epoll fd: " << m_epollFd << " fd: " << fd
                 << " op: " << StringHelper::EpollCtrlTypeToString(op)
                 << " event type: " << StringHelper::EventTypeToString(ev) << ".";
        return true;
    }
}

} // namespace Net