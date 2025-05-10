#pragma once
#include <vector>
#include <sys/poll.h>
#include "Net/Poller.h"

namespace Net {

/**
 * @brief I/O复用 poll封装类
 */
class PPoller : public Poller {
public:
    using Ptr = std::shared_ptr<PPoller>;
    using WkPtr = std::weak_ptr<PPoller>;

public:
    explicit PPoller(EventLoopWkPtr loop);
    ~PPoller() override;

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
    // 监听事件列表
    std::vector<pollfd> m_pollEventList;
};

}; // namespace Net