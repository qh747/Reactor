#pragma once
#include <vector>
#include <memory>
#include <sys/poll.h>

namespace Common {

/**
 * @brief 事件类型
 */
typedef enum class EventType : int {
    /**  无事件类型 */
    EvTypeNone = 0,

    /** 单事件类型 */
    EvTypeRead = POLLIN | POLLPRI | POLLRDHUP,
    EvTypeWrite = POLLOUT,
    EvTypeClose = POLLHUP | POLLNVAL,
    EvTypeError = POLLERR,

    /** 复合事件类型 */
    EvTypeReadWrite = EvTypeRead | EvTypeWrite,
    EvTypeReadClose = EvTypeRead | EvTypeClose,
    EvTypeReadError = EvTypeRead | EvTypeError,
    EvTypeWriteClose = EvTypeWrite | EvTypeClose,
    EvTypeWriteError = EvTypeWrite | EvTypeError,
    EvTypeCloseError = EvTypeClose | EvTypeClose,
    EvTypeReadWriteClose = EvTypeReadWrite | EvTypeClose,
    EvTypeReadWriteError = EvTypeReadWrite | EvTypeError,
    EvTypeReadCloseError = EvTypeReadClose | EvTypeError,
    EvTypeWriteCloseError = EvTypeWriteClose | EvTypeError,
    EvTypeAll = EvTypeRead | EvTypeWrite | EvTypeClose | EvTypeError,
} Event_t;

/**
 * @brief 状态类型
 */
typedef enum class StateType : int {
    StatePending = -1,
    StateInLoop = 0,
    StateNotInLoop = 1
} State_t;

/**
 * @brief poller类型
 */
typedef enum class PollerType : int {
    PollerPoll = 0,
    PollerEpoll = 1,
} Poller_t;

/**
 * @note  类型值分别与EPOLL_CTRL_ADD、EPOLL_CTL_MOD、EPOLL_CTL_DEL对应
 * @brief Poller控制类型
 */
typedef enum class PollerControlType : int {
    PollerAdd = 1,
    PollerModify = 2,
    PollerRemove = 3,
} PollerCtrl_t;

}; // namespace Common

namespace Net {

/**
 * @brief channel包装类
 */
class Channel;
typedef struct ChannelWrapperDataType {
    Common::Event_t evType;
    std::shared_ptr<Channel> channel;

    ChannelWrapperDataType() = default;
    ChannelWrapperDataType(Common::Event_t evType, std::shared_ptr<Channel> channel)
        : evType(evType), channel(channel) {
    }

} ChannelWrapper_dt;

// Channel包装类
using ChannelWrapperPtr = std::shared_ptr<ChannelWrapper_dt>;
using ChannelWrapperList = std::vector<ChannelWrapperPtr>;

}; // namespace Net