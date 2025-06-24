#include <iostream>
#include <Net/EventLoop.h>
#include <Thread/EventLoopThread.h>
#include <Thread/EventLoopThreadPool.h>
using namespace Net;
using namespace Thread;

void FuncTest() {
    auto funcThdInit = [](const EventLoop::WkPtr& evLoopWkPtr) {
        if (!evLoopWkPtr.expired()) {
            auto evLoop = evLoopWkPtr.lock();
            std::cout << "Thread init success. id: " << evLoop->getId() << " thread id: " << evLoop->getThreadId() << std::endl;
        }
    };

    {
        std::cout << "NET EVENTLOOP THREAD POOL TEST FIRST -----------------------------" << std::endl;

        EventLoopThreadPool::Ptr evThdPool = std::make_shared<EventLoopThreadPool>(0, funcThdInit);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    {
        /*std::cout << "NET EVENTLOOP THREAD POOL TEST SECOND -----------------------------" << std::endl;

        EventLoopThreadPool::Ptr evThdPool = std::make_shared<EventLoopThreadPool>(1, funcThdInit);

        EventLoop::WkPtr evLoopMainWkPtr;
        if (evThdPool->getMainEventLoop(evLoopMainWkPtr)) {
            auto evMainLoop = evLoopMainWkPtr.lock();
            evMainLoop->executeTaskInLoop([evLoopMainWkPtr]() {
                if (!evLoopMainWkPtr.expired()) {
                    auto evLoop = evLoopMainWkPtr.lock();
                    std::cout << "Execute task in main event loop success. id: " << evLoop->getId()
                        << " thread id: " << evLoop->getThreadId() << std::endl;
                }
            });
        }

        EventLoop::WkPtr evLoopWorkWkPtr;
        if (evThdPool->getNextWorkEventLoop(evLoopWorkWkPtr)) {
            auto evWorkLoop = evLoopWorkWkPtr.lock();
            evWorkLoop->executeTaskInLoop([evLoopWorkWkPtr]() {
                if (!evLoopWorkWkPtr.expired()) {
                    auto evLoop = evLoopWorkWkPtr.lock();
                    std::cout << "Execute task in work event loop success. id: " << evLoop->getId()
                        << " thread id: " << evLoop->getThreadId() << std::endl;
                }
            });
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));*/
    }
}

int main() {
    FuncTest();
    return 0;
}