#include "Utils/Logger.h"
#include "Net/EventLoop.h"
#include "Net/Poller.h"
#include "Net/PPoller.h"
#include "Net/EpPoller.h"
#include "Factory/PollerFactory.h"

namespace Factory {

Poller::Ptr PollerFactory::CreatePoller(Poller_t type, EventLoop::WkPtr loop) {
    if (Poller_t::PollerPoll == type) {
        return std::make_shared<PPoller>(std::move(loop));
    }
    else if (Poller_t::PollerEpoll == type) {
        return std::make_shared<EpPoller>(std::move(loop));
    }
    else {
        LOG_ERROR << "Create poller error. invalid poller type. type: " << static_cast<int>(type);
        return nullptr;
    }
}

} // namespace Factory