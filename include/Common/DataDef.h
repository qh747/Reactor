#pragma once
#include <poll.h>

namespace Common {

/**
 * @brief 事件类型
 */
typedef enum class EventType : int {
    EvTypeNone = 0,
    EvTypeRead = POLLIN | POLLPRI | POLLRDHUP,
    EvTypeWrite = POLLOUT,
    EvTypeReadWrite = POLLIN | POLLPRI | POLLOUT,
    EvTypeClose = POLLHUP | POLLNVAL,
    EvTypeError = POLLERR,
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

}; // namespace Common