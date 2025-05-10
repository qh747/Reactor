#include "Utils/Logger.h"
#include "Net/EventLoop.h"
#include "Net/Poller.h"
#include "Net/PPoller.h"
#include "Net/EpPoller.h"
#include "Factory/PollerFactory.h"

namespace Factory {

Poller::Ptr PollerFactory::CreatePoller(Poller_t type, EventLoopWkPtr loop, const std::string& id) {
    if (Poller_t::PollerPoll == type) {
        return std::make_shared<PPoller>(std::move(loop), id);
    }
    else if (Poller_t::PollerEpoll == type) {
        return std::make_shared<EpPoller>(std::move(loop), id);
    }
    else {
        LOG_ERROR << "Create poller error. invalid poller type. type: " << static_cast<int>(type) << " id: " << id;
        return nullptr;
    }
}

} // namespace Factory