#include "Common/Logger.h"
#include "Net/EventLoop.h"
#include "Net/Channel.h"

namespace Net {

Channel::Channel(EventLoopPtr loop, int fd)
    : m_ownerLoopPtr(loop),
      m_fd(fd) {
}

Channel::~Channel() {
}

void Channel::handleEvent(Event_t type, Timestamp recvTime) {
    auto cbIter = m_evCbMap.find(type);
    if (m_evCbMap.end() == cbIter) {
        cbIter->second(recvTime);
        LOG_INFO << "Channel handle event. fd: " << m_fd << " event type: " << StringHelper::EventTypeToString(type);
    }
    else {
        LOG_INFO << "Channel handle event error. event callback funtion not register. fd: "
                 << m_fd << " event type: " << StringHelper::EventTypeToString(type);
    }
}

void Channel::setListenEvent(Event_t type) {
    // 监听事件类型更新
    m_listenEvType = type;

    // 在事件循环中更新channel监听的事件类型
    if (Event_t::EvTypeNone != m_listenEvType) {
        m_ownerLoopPtr->updateChannel(this->shared_from_this());
    }
    else {
        m_ownerLoopPtr->removeChannel(this->shared_from_this());
    }
}

} // namespace Net