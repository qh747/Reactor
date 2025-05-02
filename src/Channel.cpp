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

const static int EvReadTypeCmp = static_cast<int>(Event_t::EvTypeRead);
const static int EvWriteTypeCmp = static_cast<int>(Event_t::EvTypeWrite);
const static int EvCloseTypeCmp = static_cast<int>(Event_t::EvTypeClose);
const static int EvErrorTypeCmp = static_cast<int>(Event_t::EvTypeError);

Channel::Channel(EventLoop::WkPtr loop, int fd)
    : m_ownerLoop(loop),
      m_fd(fd) {
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
    return m_ownerLoop.lock()->updateChannel(this->shared_from_this());
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
    return m_ownerLoop.lock()->updateChannel(this->shared_from_this());
}

bool Channel::close() {
    // channel未启动
    if (State_t::StatePending == m_state) {
        LOG_WARN << "Channel close warning. closed already. fd: " << m_fd;
        return true;
    }

    // 监听事件类型更新
    m_listenEvType = Event_t::EvTypeNone;

    // 在事件循环中移除channel
    return m_ownerLoop.lock()->removeChannel(this->shared_from_this());
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
        LOG_ERROR << "Channel set event callback function error. event callback funtion invalid. fd: "
                  << m_fd << " event type: " << StringHelper::EventTypeToString(type);
        return false;
    }

    m_evCbMap[type] = std::move(cb);
    return true;
}

bool Channel::handleEventWithoutCheck(Event_t type, Timestamp recvTime) {
    auto evCbIter = m_evCbMap.find(Event_t::EvTypeRead);
    if (m_evCbMap.end() == evCbIter) {
        LOG_WARN << "Channel handle event warning. event callback funtion not regist. fd: "
                 << m_fd << " event type: " << StringHelper::EventTypeToString(type);
        return false;
    }

    // 读事件处理
    evCbIter->second(recvTime);
    LOG_INFO << "Channel handle event. fd: " << m_fd << " event type: " << StringHelper::EventTypeToString(type);
    return true;
}

} // namespace Net