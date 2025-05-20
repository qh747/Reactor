#pragma once
#include "Common/TypeDef.h"
#include "Utils/Utils.h"

using namespace Common;
using namespace Utils;
using namespace Net;

namespace Factory {

/**
 * @brief Poller工厂类
 */
class PollerFactory : public NoncopyableConstructable {
public:
    /**
     * @brief  创建Poller
     * @return Poller::Ptr Poller指针
     * @param  type Poller类型
     * @param  loop EventLoop指针
     */
    static PollerPtr CreatePoller(Poller_t type, EventLoopWkPtr loop);
};

}; // namespace Factory