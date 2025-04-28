#include "Common/Logger.h"
#include "Factory/PollerFactory.h"

namespace Factory {

Poller::Ptr PollerFactory::CreatePoller(Poller_t type, Poller::EventLoopPtr loop) {
    if (Poller_t::PollerPoll == type) {
        return nullptr;
    }
    else if (Poller_t::PollerEpoll == type) {
        return nullptr;
    }
    else {
        LOG_ERROR << "Create poller error. invalid poller type. type: " << static_cast<int>(type);
        return nullptr;
    }
}

} // namespace Factory