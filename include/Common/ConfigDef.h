#pragma once
#include <cstddef>
#include "Common/DataDef.h"

namespace Common {

// I/O多路复用初始监听事件储量
const std::size_t POLL_INIT_WAIT_EVENTS_SIZE = 16;

// I/O多路复用默认类型
const Poller_t POLLER_DEFAULT_TYPE = Poller_t::PollerEpoll;

}; // namespace Common