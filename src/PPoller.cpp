#include <cstring>
#include <unistd.h>
#include "Utils/Logger.h"
#include "Net/Channel.h"
#include "Net/PPoller.h"
using namespace Common;
using namespace Utils;

namespace Net {

PPoller::PPoller(EventLoopWkPtr loop) 
    : Poller(std::move(loop)) {
    LOG_DEBUG << "Poll poller construct. id: " << m_id;
}

PPoller::~PPoller() {
    LOG_DEBUG << "Poll poller destruct. id: " << m_id;
}

Timestamp PPoller::poll(int timeoutMs, ChannelWrapperList& activeChannels, int& errCode) {
    // 填充事件列表
    m_pollEventList.resize(m_channelMap.size());
    for (const auto& pair : m_channelMap) {
        const auto& channel = pair.second;
        if (Event_t::EvTypeNone == channel->getEvType()) {
            continue;
        }

        pollfd event = {};
        event.fd = channel->getFd();
        event.events = static_cast<short>(channel->getEvType());

        m_pollEventList.emplace_back(event);
    }

    // 等待事件发生
    int activeEventSize = ::poll(m_pollEventList.data(), m_pollEventList.size() + 1, timeoutMs);
    auto now = std::chrono::system_clock::now();

    if (activeEventSize < 0) {
        if (errno == EINTR) {
            // 外部中断
            errCode = EINTR;
            LOG_WARN << "Poll poll warning. external interrupt. id: " << m_id << " code: " << errno << ". msg: " << strerror(errno); 
        }
        else {
            // poll()出错
            errCode = errno;
            LOG_FATAL << "Poll poll error. id: " << m_id << " code: " << errno << ". msg: " << strerror(errno);
        }
    }
    else if (0 == activeEventSize) {
        // poll()超时
        errCode = ETIMEDOUT;
        LOG_WARN << "Poll poll warning. timeout. id: " << m_id << " code: " << errno << ". msg: " << strerror(errno);
    }
    else {
        // 处理活跃的channel
        for (int idx = 0; idx < activeEventSize; ++idx) {
            const auto& event = m_pollEventList[idx];
            const auto& channelMapIter = m_channelMap.find(event.fd);
            if (m_channelMap.end() == channelMapIter) {
                LOG_ERROR << "Poll poll error. channel not found. id: " << m_id << " fd: " << event.fd << ".";
                continue;
            }

            // 添加活跃的channel
            auto evType = static_cast<Event_t>(event.events);
            activeChannels.emplace_back(std::make_shared<ChannelWrapper>(channelMapIter->second, evType));

            LOG_DEBUG << "Poll poll success. id: " << m_id << " fd: " << event.fd << " event type: " 
                      << StringHelper::EventTypeToString(evType) << ".";
        }
    }

    return now;
}

bool PPoller::updateChannel(Channel::Ptr channel) {
    if (nullptr == channel) {
        LOG_ERROR << "Update channel error. channel invalid. id: " << m_id << ".";
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
    }
    else if (State_t::StateInLoop == state) {
        if (Event_t::EvTypeNone == evType) {
            // 更新channel状态
            state = State_t::StateNotInLoop;
            channel->setState(state);
        }
    }
    else {
        LOG_ERROR << "Update channel error. channel state invalid. id: " << m_id << " fd: " << fd  << " state: " 
                  << StringHelper::StateTypeToString(state) << " event type: " << StringHelper::EventTypeToString(evType) << ".";
        return false;
    }

    LOG_INFO << "Update channel success. id: " << m_id << " fd: " << fd << " state: " << StringHelper::StateTypeToString(state) 
             << " event type: " << StringHelper::EventTypeToString(evType) << ".";
    return true;
}

bool PPoller::removeChannel(Channel::Ptr channel) {
    if (nullptr == channel) {
        LOG_ERROR << "Remove channel error. channel invalid.";
        return false;
    }

    // 设置channel状态
    State_t state = channel->getState();
    channel->setState(State_t::StatePending);

    // 移除channel
    int fd = channel->getFd();
    if(m_channelMap.end() != m_channelMap.find(fd)) {
        m_channelMap.erase(fd);
    }

    LOG_INFO << "Remove channel success. id: " << m_id << " fd: " << fd << " state: " << StringHelper::StateTypeToString(state) 
             << " event type: " << StringHelper::EventTypeToString(channel->getEvType()) << ".";
    return true;
}

} // namespace Net