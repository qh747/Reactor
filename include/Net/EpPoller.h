#pragma once
#include <vector>
#include <sys/epoll.h>
#include "Net/Poller.h"

namespace Net {

/**
 * @brief I/O复用 epoll封装类
 */
class EpPoller : public Poller {
public:
    using Ptr = std::shared_ptr<EpPoller>;
    using WkPtr = std::weak_ptr<EpPoller>;

public:
    EpPoller(EventLoopWkPtr loop, const std::string& id);
    ~EpPoller() override;

public:
    /**
     * @brief  等待事件触发
     * @return 事件触发的时间戳
     * @param  timeoutMs 等待时间
     * @param  activeChannels 事件触发的channel
     * @param  errCode 错误码
     */
    Timestamp poll(int timeoutMs, ChannelWrapperList& activeChannels, int& errCode) override;

    /**
     * @brief  更新channel
     * @return 更新结果
     * @param  channel 需要更新的channel
     */
    bool updateChannel(ChannelPtr channel) override;

    /**
     * @brief  移除channel
     * @return 移除结果
     * @param  channel 需要移除的channel
     */
    bool removeChannel(ChannelPtr channel) override;

private:
    /**
     * @brief  操作epoll
     * @return 操作结果
     * @param  fd 操作的fd
     * @param  ev 操作的事件
     * @param  op 操作类型
     */
    bool operateControl(int fd, Event_t ev, PollerCtrl_t op) const;

private:
    // epoll fd
    int m_epollFd;

    // 监听事件列表
    std::vector<epoll_event> m_epollEventList;
};

}; // namespace Net