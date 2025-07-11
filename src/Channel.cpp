#include <unordered_set>
#include "Utils/Logger.h"
#include "Net/EventLoop.h"
#include "Net/Channel.h"

namespace Net {

const static std::unordered_set<Event_t> ValidEvents = {
    Event_t::EvTypeNone,
    Event_t::EvTypeRead,
    Event_t::EvTypeWrite,
    Event_t::EvTypeClose,
    Event_t::EvTypeError,
    Event_t::EvTypeReadWrite,
    Event_t::EvTypeReadClose,
    Event_t::EvTypeReadError,
    Event_t::EvTypeWriteClose,
    Event_t::EvTypeWriteError,
    Event_t::EvTypeCloseError,
    Event_t::EvTypeReadWriteClose,
    Event_t::EvTypeReadWriteError,
    Event_t::EvTypeReadCloseError,
    Event_t::EvTypeWriteCloseError,
    Event_t::EvTypeAll};

const static std::unordered_set<Event_t> ValidCbEvents = {
    Event_t::EvTypeRead,
    Event_t::EvTypeWrite,
    Event_t::EvTypeClose,
    Event_t::EvTypeError,
    Event_t::EvTypeReadWrite};

constexpr static int EvReadTypeCmp = static_cast<int>(Event_t::EvTypeRead);
constexpr static int EvWriteTypeCmp = static_cast<int>(Event_t::EvTypeWrite);
constexpr static int EvCloseTypeCmp = static_cast<int>(Event_t::EvTypeClose);
constexpr static int EvErrorTypeCmp = static_cast<int>(Event_t::EvTypeError);

Channel::Channel(EventLoop::WkPtr loop, int fd)
    : m_fd(fd), m_ownerLoop(std::move(loop)) {
    LOG_DEBUG << "Channel construct. fd: " << m_fd;
}

Channel::~Channel() {
    if (State_t::StatePending != m_state) {
        this->close();
    }

    LOG_DEBUG << "Channel deconstruct. fd: " << m_fd;
}

bool Channel::open(Event_t type) {
    // 事件类型校验
    if (ValidEvents.end() == ValidEvents.find(type)) {
        LOG_ERROR << "Channel open error. invalid event type. fd: " << m_fd << " event type: "
            << StringHelper::EventTypeToString(type);
        return false;
    }

    // channel已启动
    if (State_t::StatePending != m_state) {
        LOG_WARN << "Channel open warning. open already. fd: " << m_fd;
        return true;
    }

    // 监听事件类型更新
    m_listenEvType = type;

    // 在事件循环中更新channel
    if (!m_ownerLoop.lock()->updateChannel(this->shared_from_this())) {
        LOG_ERROR << "Channel open error. update channel failed. fd: " << m_fd;
        return false;
    }

    LOG_DEBUG << "Channel open success. fd: " << m_fd << " event type: " << StringHelper::EventTypeToString(type);
    return true;
}

bool Channel::update(Event_t type) {
    // 事件类型校验
    if (ValidEvents.end() == ValidEvents.find(type)) {
        LOG_ERROR << "Channel update error. invalid event type. fd: " << m_fd << " event type: "
            << StringHelper::EventTypeToString(type);
        return false;
    }

    // channel是否已启动
    if (State_t::StatePending == m_state) {
        LOG_ERROR << "Channel update error. not opened. fd: " << m_fd;
        return false;
    }

    // 监听事件类型更新
    m_listenEvType = type;

    // 在事件循环中更新channel
    if (!m_ownerLoop.lock()->updateChannel(this->shared_from_this())) {
        LOG_ERROR << "Channel update error. update channel failed. fd: " << m_fd;
        return false;
    }

    LOG_DEBUG << "Channel update success. fd: " << m_fd << " event type: " << StringHelper::EventTypeToString(type);
    return true;
}

bool Channel::close() {
    // channel未启动
    if (State_t::StatePending == m_state) {
        LOG_WARN << "Channel close warning. closed already. fd: " << m_fd;
        return true;
    }

    // 在事件循环中移除channel
    if (!m_ownerLoop.expired()) {
        LOG_DEBUG << "Channel close success. fd: " << m_fd;
        return m_ownerLoop.lock()->removeChannel(this->shared_from_this());
    }
    else {
        m_listenEvType = Event_t::EvTypeNone;
        return true;
    }
}

bool Channel::handleEvent(Event_t type, Timestamp recvTime) {
    // 事件类型校验
    if (ValidEvents.end() == ValidEvents.find(type)) {
        LOG_ERROR << "Channel handle event error. invalid event type. fd: " << m_fd << " event type: "
            << StringHelper::EventTypeToString(type);
        return false;
    }

    // channel未处于事件处理状态
    if (State_t::StatePending == m_state || Event_t::EvTypeNone == m_listenEvType) {
        LOG_ERROR << "Channel handle event error. channel not in handle event state. fd: " << m_fd << " event type: "
            << StringHelper::EventTypeToString(type);
        return false;
    }

    /** 事件类型匹配则直接处理 */
    auto evCbIter = m_evCbMap.find(type);
    if (m_evCbMap.end() != evCbIter) {
        return this->handleEventWithoutCheck(type, recvTime);
    }

    /** 单独事件类型匹配 */
    int listenEvType = static_cast<int>(m_listenEvType);
    int activeEvType = static_cast<int>(type);

    // 读事件处理
    if ((listenEvType & EvReadTypeCmp) && (activeEvType & EvReadTypeCmp)) {
        return this->handleEventWithoutCheck(Event_t::EvTypeRead, recvTime);
    }

    // 写事件处理
    if ((listenEvType & EvWriteTypeCmp) && (activeEvType & EvWriteTypeCmp)) {
        return this->handleEventWithoutCheck(Event_t::EvTypeWrite, recvTime);
    }

    // 关闭事件处理
    if ((listenEvType & EvCloseTypeCmp) && (activeEvType & EvCloseTypeCmp)) {
        return this->handleEventWithoutCheck(Event_t::EvTypeClose, recvTime);
    }

    // 错误事件处理
    if ((listenEvType & EvErrorTypeCmp) && (activeEvType & EvErrorTypeCmp)) {
        return this->handleEventWithoutCheck(Event_t::EvTypeError, recvTime);
    }

    LOG_ERROR << "Channel handle event error. event not handled. fd: " << m_fd
        << " listen event type: " << StringHelper::EventTypeToString(m_listenEvType)
        << " active event type: " << StringHelper::EventTypeToString(type);
    return false;
}

bool Channel::setEventCb(Event_t type, EventCb cb) {
    // 事件类型无效
    if (ValidCbEvents.end() == ValidCbEvents.find(type)) {
        LOG_ERROR << "Channel set event callback function error. support event type: "
            << "EvTypeRead | EvTypeWrite | EvTypeClose | EvTypeError | EvTypeReadWrite. "
            << "fd: " << m_fd << " event type: " << StringHelper::EventTypeToString(type);
        return false;
    }

    // 事件处理回调函数无效
    if (nullptr == cb) {
        LOG_ERROR << "Channel set event callback function error. event callback function invalid. fd: "
            << m_fd << " event type: " << StringHelper::EventTypeToString(type);
        return false;
    }

    m_evCbMap[type] = std::move(cb);
    return true;
}

void Channel::setWriteEnabled(bool enabled) {
    int evType = enabled ?
        static_cast<int>(m_listenEvType) | static_cast<int>(Event_t::EvTypeWrite) :
        static_cast<int>(m_listenEvType) & (~static_cast<int>(Event_t::EvTypeWrite));

    this->update(static_cast<Event_t>(evType));
}

void Channel::setReadEnabled(bool enabled) {
    int evType = enabled ?
        static_cast<int>(m_listenEvType) | static_cast<int>(Event_t::EvTypeRead) :
        static_cast<int>(m_listenEvType) & (~static_cast<int>(Event_t::EvTypeRead));

    this->update(static_cast<Event_t>(evType));
}

bool Channel::handleEventWithoutCheck(Event_t type, Timestamp recvTime) {
    auto evCbIter = m_evCbMap.find(type);
    if (m_evCbMap.end() == evCbIter) {
        LOG_WARN << "Channel handle event warning. event callback function not regist. fd: "
            << m_fd << " event type: " << StringHelper::EventTypeToString(type);
        return false;
    }

    // 事件处理
    LOG_INFO << "Channel handle event. fd: " << m_fd << " event type: " << StringHelper::EventTypeToString(type);
    evCbIter->second(recvTime);
    return true;
}

} // namespace Net