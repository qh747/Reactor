#pragma once
#include <memory>
#include <chrono>
#include <vector>
#include <functional>
#include <unordered_map>

namespace Common {

// 时间戳类型
using Timestamp = std::chrono::system_clock::time_point;

}; // namespace Common

namespace Utils {

// TimerQueue类型前置声明
class TimerQueue;
using TimerQueuePtr = std::shared_ptr<TimerQueue>;
using TimerQueueWkPtr = std::weak_ptr<TimerQueue>;

using TimerId = uint64_t;
using TimerTaskCb = std::function<void()>;

}; // namespace Utils

namespace Net {

// Channel类型前置声明
class Channel;
using ChannelPtr = std::shared_ptr<Channel>;
// channel管理map，key = fd, value = channel pointer
using ChannelMap = std::unordered_map<int, ChannelPtr>;

// Channel包装类前置声明
class ChannelWrapper;
using ChannelWrapperPtr = std::shared_ptr<ChannelWrapper>;
using ChannelWrapperList = std::vector<ChannelWrapperPtr>;

// I/O多路复用封装类前置声明
class Poller;
using PollerPtr = std::shared_ptr<Poller>;

// 事件循环类型前置声明
class EventLoop;
using EventLoopPtr = std::shared_ptr<EventLoop>;
using EventLoopWkPtr = std::weak_ptr<EventLoop>;

}; // namespace Net
