#pragma once
#include "Common/DataDef.h"
#include "Utils/Utils.h"
#include "Net/Poller.h"
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
     * @param  id Poller id
     */
    static Poller::Ptr CreatePoller(Poller_t type, EventLoopPtr loop, const std::string& id);
};

}; // namespace Factory