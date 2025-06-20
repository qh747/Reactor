#pragma once
#include <string>
#include <cstddef>
#include "Common/DataDef.h"

namespace Common {

// I/O多路复用初始监听事件储量
constexpr std::size_t POLL_INIT_WAIT_EVENTS_SIZE = 16;

// I/O多路复用默认类型
constexpr Poller_t POLLER_DEFAULT_TYPE = Poller_t::PollerEpoll;

// I/O多路复用默认等待时长(-1则阻塞等待)，单位：秒
constexpr int POLLER_DEFAULT_WAIT_TIME = -1;

// 缓冲区初始大小，单位：字节
constexpr int BUFFER_INIT_SIZE = 1024;

// 缓冲区预分配空间大小，单位：字节
constexpr int BUFFER_PREPEND_SIZE = 8;

// 命名前缀
const std::string EV_LOOP_THD_POOL_PREFIX = "EV_LOOP_THD_POOL_";
const std::string EV_LOOP_MAIN_THD_PREFIX = "MAIN_THD_";
const std::string EV_LOOP_WORK_THD_PREFIX = "WORK_THD_";
const std::string EV_LOOP_PREFIX = "EV_LOOP_";
const std::string TIME_QUEUE_PREFIX = "TIME_QUEUE_";

// 多级命名连接符
const std::string PREFIX_SIGN = "@";

}; // namespace Common