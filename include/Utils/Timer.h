#pragma once
#include <set>
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
     * @brief  自定义定时器任务比较函数
     * @return 比较结果
     * @param  lft 比较对象
     * @param  rhs 比较对象
     */
    struct Compare {
        bool operator()(const TimerTask::Ptr& lhs, const TimerTask::Ptr& rhs) const {
            return lhs->m_expires < rhs->m_expires;
        }
    };

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
    // 允许以下类初始化TimerQueue
    friend class Net::EventLoop;

public:
    using Ptr = std::shared_ptr<TimerQueue>;
    using WkPtr = std::weak_ptr<TimerQueue>;
    using TimerId = uint64_t;
    using TimerTasks = std::set<TimerTask::Ptr, TimerTask::Compare>;

public:
    explicit TimerQueue(EventLoopWkPtr loop);
    ~TimerQueue();

public:
    /**
     * @brief  添加定时器任务
     * @return 添加结果
     * @param  id 定时器任务id
     * @param  cb 定时器任务回调函数
     * @param  expires 定时器到期时间
     * @param  intervalSec 定时器任务间隔(单位: 秒)
     */
    bool addTimerTask(TimerId& id, TimerTask::Task cb, Timestamp expires, double intervalSec = 0);
    
    /**
     * @brief  删除定时器任务
     * @return 删除结果
     * @param  id 定时器任务id
     */
    bool delTimerTask(TimerId id);

private:
    /**
     * @brief  初始化timer queue
     * @return 初始化结果
     */
    bool init();

    /**
     * @brief  处理事件
     * @return 处理结果
     */
    bool handleTask();

    /**
     * @brief  获取超时定时器任务
     * @return 获取结果
     * @param  expired 超时时间
     * @param  expiredTasks 超时定时器任务map
     */
    bool getExpiredTasks(Timestamp expired, TimerTasks& expiredTasks);

    /**
     * @brief  重置定时器任务
     * @return 重置结果
     */
    bool resetExpiredTimerTask();

private:
    // 定时器队列所属的事件循环
    EventLoopWkPtr m_ownerLoop;

    // 定时器队列关联的channel
    ChannelPtr m_timerChannel;

    // 定时器任务map
    TimerTasks m_timerTasks;

    // 待移除定时器任务map
    TimerTasks m_cancelTimerTasks;

    // 是否在处理定时器任务
    std::atomic_bool m_isHandleTask;
};

}; // namespace Utils