#pragma once
#include <string>
#include <memory>
#include "Common/TypeDef.h"
#include "Utils/Utils.h"
using namespace Common;
using namespace Utils;

namespace Net {

/**
 * @note  使用PollerFactory::CreatePoller()函数创建Poller对象
 * @brief I/O多路复用基类
 */
class Poller : public Noncopyable, public std::enable_shared_from_this<Poller> {
public:
    using Ptr = std::shared_ptr<Poller>;
    using WkPtr = std::weak_ptr<Poller>;

public:
    explicit Poller(EventLoopWkPtr loop);
    virtual ~Poller() = default;

public:
    /**
     * @brief  等待事件触发
     * @return 事件触发的时间戳
     * @param  timeoutMs 等待时间
     * @param  activeChannels 事件触发的channel
     * @param  errCode 错误码
     */
    virtual Timestamp poll(int timeoutMs, ChannelWrapperList& activeChannels, int& errCode) = 0;

    /**
     * @brief  更新channel
     * @return 更新结果
     * @param  channel 需要更新的channel
     */
    virtual bool updateChannel(ChannelPtr channel) = 0;

    /**
     * @brief  移除channel
     * @return 移除结果
     * @param  channel 需要移除的channel
     */
    virtual bool removeChannel(ChannelPtr channel) = 0;

public:
    /**
     * @brief  判断channel是否存在
     * @return 判断结果
     * @param  channel 需要判断的channel
     */
    bool hasChannel(const ChannelPtr& channel) const;

    /**
     * @brief  获取poller id
     * @return poller id
     */
    inline std::string getId() const {
        return m_id;
    }

    /** 
     * @brief  获取poller所属的事件循环
     * @return poller所属的事件循环
     */
    inline EventLoopWkPtr getOwnerLoop() const {
        return m_ownerLoop;
    }

protected:
    // poller id
    const std::string m_id;

    // poller所属的事件循环弱引用
    EventLoopWkPtr m_ownerLoop;

    // poller管理的channel map
    ChannelMap m_channelMap;
};

}; // namespace Net