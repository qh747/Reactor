#pragma once
#include <atomic>
#include <memory>
#include <cstdint>
#include <functional>
#include "Common/TypeDef.h"
#include "Utils/Utils.h"
using namespace Common;
using namespace Net;

namespace Utils {

/**
 * @brief 定时器任务类
 */
class TimerTask : public Noncopyable, public std::enable_shared_from_this<TimerTask> {
public:
    using Ptr = std::shared_ptr<TimerTask>;
    using WkPtr = std::weak_ptr<TimerTask>;
    using Task = std::function<void()>;

public:
    TimerTask(Task cb, Timestamp expires, double intervalSec = 0);
    ~TimerTask() = default;

public:
    /**
     * @brief  执行定时器任务
     * @return 执行结果
     */
    bool executeTask();

    /**
     * @brief  定时器超时时间重置
     * @return 重置结果
     */
    bool reset();
    
public:
    /**
     * @brief  判断定时器任务是否重复触发
     * @return 判断结果
     */
    inline bool isRepeat() const {
        return m_repeat;
    }

    /**
     * @brief  获取定时器任务到期时间
     * @return 定时器到期时间
     */
    inline Timestamp getExpires() const {
        return m_expires;
    }

    /**
     * @brief  获取定时器任务id
     * @return 定时器任务id
     */
    inline uint64_t getId() const {
        return m_id;
    }

private:
    // 定时器任务id
    std::atomic<uint64_t> m_id;

    // 定时器任务回调函数
    Task m_cb;

    // 定时器到期时间
    Timestamp m_expires;

    // 定时器任务间隔(单位: 秒)
    double m_intervalSec;

    // 定时器是否重复触发
    bool m_repeat;
};

/**
 * @brief 定时器队列
 */
class TimerQueue : public Noncopyable, public std::enable_shared_from_this<TimerQueue> {
public:
    using Ptr = std::shared_ptr<TimerQueue>;
    using WkPtr = std::weak_ptr<TimerQueue>;

private:
    // 定时器队列所属的事件循环
    EventLoopPtr m_ownerEventLoop;

    // 定时器队列关联的channel
    ChannelPtr m_timerChannel;
};

}; // namespace Utils