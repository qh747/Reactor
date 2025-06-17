#include "Net/EventLoop.h"
#include "Net/Channel.h"
#include "Net/Poller.h"

namespace Net {

static uint64_t POLLER_ID_KEY = 0;

Poller::Poller(EventLoop::WkPtr loop, const std::string& namePrefix)
    : m_id(namePrefix + std::to_string(++POLLER_ID_KEY)),
      m_ownerLoop(std::move(loop)) {
}

bool Poller::hasChannel(const Channel::Ptr& channel) const {
    return nullptr == channel ? false : m_channelMap.end() != m_channelMap.find(channel->getFd());
}

} // namespace Net