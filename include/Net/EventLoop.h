#pragma once
#include <memory>
#include "Common/Typedef.h"
#include "Utils/Utils.h"
using namespace Utils;

namespace Net {

/**
 * @note  EventLoop不允许拷贝，但允许通过指针形式传递
 * @brief 事件循环类
 */
class EventLoop : public Noncopyable, public std::enable_shared_from_this<EventLoop> {
public:
    using Ptr = std::shared_ptr<EventLoop>;

public:
    bool updateChannel(ChannelPtr channel);

    bool removeChannel(ChannelPtr channel);
};

}; // namespace Net