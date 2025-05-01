#include "Utils/Logger.h"
#include "Net/PPoller.h"
#include "Net/EpPoller.h"
#include "Factory/PollerFactory.h"

namespace Factory {

Poller::Ptr PollerFactory::CreatePoller(Poller_t type, EventLoopPtr loop, const std::string& id) {
    if (Poller_t::PollerPoll == type) {
        return std::make_shared<PPoller>(loop, id);
    }
    else if (Poller_t::PollerEpoll == type) {
        return std::make_shared<EpPoller>(loop, id);
    }
    else {
        LOG_ERROR << "Create poller error. invalid poller type. type: " << static_cast<int>(type) << " id: " << id;
        return nullptr;
    }
}

} // namespace Factory