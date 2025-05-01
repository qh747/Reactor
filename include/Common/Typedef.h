#pragma once
#include <vector>
#include <memory>
#include <chrono>
#include <functional>
#include <unordered_map>
#include "Common/DataDef.h"

namespace Common {

// 时间戳类型
using Timestamp = std::chrono::system_clock::time_point;

}; // namespace Common

namespace Net {

// 事件回调函数类型
using EventCb = std::function<void(Common::Timestamp)>;
// 事件回调函数map，key = 事件类型，value = 事件回调函数
using EventCbMap = std::unordered_map<Common::Event_t, EventCb>;

// Channel类型前置声明
class Channel;
using ChannelPtr = std::shared_ptr<Channel>;

// Channel包装类
using ChannelWrapperPtr = std::shared_ptr<ChannelWrapper_dt>;
using ChannelWrapperList = std::vector<ChannelWrapperPtr>;

// channel管理map，key = fd, value = channel pointer
using ChannelMap = std::unordered_map<int, ChannelPtr>;

// 事件循环类型前置声明
class EventLoop;
using EventLoopPtr = std::shared_ptr<EventLoop>;

// I/O多路复用封装类前置声明
class Poller;
using PollerPtr = std::shared_ptr<Poller>;

}; // namespace Net
