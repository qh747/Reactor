#include <mutex>
#include <unordered_set>
#include "Utils/Logger.h"
#include "Net/Channel.h"
#include "Net/EventLoop.h"
#include "Net/Poller.h"

namespace Net {

// poller id集合
static std::mutex PollerIdSetMutex;
static std::unordered_set<std::string> PollerIdSet;

Poller::Poller(EventLoop::WkPtr loop, const std::string& id) : m_ownerLoop(loop), m_id(id) {
    std::lock_guard<std::mutex> lock(PollerIdSetMutex);

    if (PollerIdSet.end() != PollerIdSet.find(id)) {
        LOG_FATAL << "Construct poller error. duplicate poller id. id: " << id;
    }

    PollerIdSet.insert(id);
}

bool Poller::hasChannel(Channel::Ptr channel) const {
    return nullptr == channel ? false : m_channelMap.end() != m_channelMap.find(channel->getFd());
}

} // namespace Net