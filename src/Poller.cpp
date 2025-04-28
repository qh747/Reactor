#include "Net/Channel.h"
#include "Net/Poller.h"

namespace Net {

Poller::Poller(EventLoopPtr loop) : m_ownerLoop(loop) {

}

bool Poller::hasChannel(ChannelPtr channel) const {
    return nullptr == channel ? false : m_channelMap.end() != m_channelMap.find(channel->getFd());
}

} // namespace Net