#pragma once
#include <cstddef>
#include <poll.h>

namespace Common {

// epoll初始监听事件储量
const std::size_t EPOLL_INIT_WAIT_EVENTS_SIZE = 16;

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
 * @brief epoll控制类型
 */
typedef enum class EpollControlType : int {
    EpollAdd = 1,
    EpollModify = 2,
    EpollRemove = 3,
} EpCtrl_t;

}; // namespace Common