#pragma once
#include <string>
#include "Utils/Utils.h"
#include "Common/TypeDef.h"
using namespace Utils;

namespace Thread {

/**
 * @brief 事件循环线程池
 */
class EventLoopThreadPool : public Noncopyable {
public:
    explicit EventLoopThreadPool(const std::string& nameArg, int numThreads = 0, ThreadInitCb cb = nullptr);
    ~EventLoopThreadPool();
};

}; // namespace Thread