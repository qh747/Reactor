#pragma once
#include <sys/epoll.h>
#include "Net/Poller.h"

namespace Net {

/**
 * @brief I/O复用 epoll封装类
 */
class EpPoller : public Poller {
public:
    EpPoller(EventLoopPtr loop);
    ~EpPoller() override;

public:
    /**
     * @brief  等待事件触发
     * @return 事件触发的时间戳
     * @param timeoutMs 等待时间
     * @param activeChannels 事件触发的channel
     */
    Timestamp wait(int timeoutMs, ChannelList& activeChannels) override;

    /**
     * @brief  更新channel
     * @return 更新结果
     * @param channel 需要更新的channel
     */
    bool updateChannel(ChannelPtr channel) override;

    /**
     * @brief  移除channel
     * @return 移除结果
     * @param channel 需要移除的channel
     */
    bool removeChannel(ChannelPtr channel) override;

private:
    /**
     * @brief  操作epoll
     * @return 操作结果
     * @param fd 操作的fd
     * @param ev 操作的事件
     * @param op 操作类型
     */
    bool operateControl(int fd, Event_t ev, EpCtrl_t op);

private:
    // epoll fd
    int m_epollFd;

    // 监听事件数量
    std::size_t m_waitEventsSize;
};

}; // namespace Net