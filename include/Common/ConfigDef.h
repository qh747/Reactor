#pragma once
#include <cstddef>
#include "Common/DataDef.h"

namespace Common {

// I/O多路复用初始监听事件储量
constexpr std::size_t POLL_INIT_WAIT_EVENTS_SIZE = 16;

// I/O多路复用默认类型
constexpr Poller_t POLLER_DEFAULT_TYPE = Poller_t::PollerEpoll;

// I/O多路复用默认等待时长(-1则阻塞等待)，单位：秒
constexpr int POLLER_DEFAULT_WAIT_TIME = -1;

}; // namespace Common