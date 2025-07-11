#pragma once
#include <sys/poll.h>
#include <sys/socket.h>

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
    EvTypeAll = EvTypeRead | EvTypeWrite | EvTypeClose | EvTypeError

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
 * @brief 连接状态类型
 */
typedef enum class ConnStateType : int {
    ConnStateClosed = 0,
    ConnStateConnected = 1,
    ConnStateDisconnected = 2,
    ConnStateError = 3

} ConnState_t;

/**
 * @brief poller类型
 */
typedef enum class PollerType : int {
    PollerPoll = 0,
    PollerEpoll = 1

} Poller_t;

/**
 * @note  类型值分别与EPOLL_CTRL_ADD、EPOLL_CTL_MOD、EPOLL_CTL_DEL对应
 * @brief Poller控制类型
 */
typedef enum class PollerControlType : int {
    PollerAdd = 1,
    PollerModify = 2,
    PollerRemove = 3

} PollerCtrl_t;

/**
 * @brief 网络地址类型
 */
typedef enum class AddrType : int {
    IPv4 = AF_INET,
    IPv6 = AF_INET6

} Addr_t;

/**
 * @brief socket类型
 */
typedef enum class SocketType : int {
    TCP = SOCK_STREAM,
    UDP = SOCK_DGRAM

} Socket_t;

/**
 * @brief socket关闭类型
 */
typedef enum class SocketShutdownType : int {
    ShutdownRead = SHUT_RD,
    ShutdownWrite = SHUT_WR,
    ShutdownReadWrite = SHUT_RDWR

} SocketShutdown_t;

}; // namespace Common