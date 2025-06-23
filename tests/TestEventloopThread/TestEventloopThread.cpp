#include <iostream>
#include <Net/EventLoop.h>
#include <Thread/EventLoopThread.h>
using namespace Net;
using namespace Thread;

void FuncTest() {
    {
        std::cout << "NET EVENTLOOP THREAD TEST FIRST -----------------------------" << std::endl;

        // 创建未运行的eventloop thread，测试退出是否正常
        EventLoopThread::Ptr evThd = std::make_shared<EventLoopThread>("EV_THD_FST_TEST");
    }

    {
        std::cout << "NET EVENTLOOP THREAD TEST SECOND -----------------------------" << std::endl;

        // 创建运行但未关闭的eventloop thread，测试退出是否正常
        EventLoopThread::Ptr evThd = std::make_shared<EventLoopThread>("EV_THD_SEC_TEST");
        evThd->run();

        EventLoop::WkPtr evLoopWkPtr;
        if (evThd->getEventLoop(evLoopWkPtr)) {
            auto evLoop = evLoopWkPtr.lock();

            auto thdId = evThd->getThreadId();
            evLoop->executeTaskInLoop([thdId]() {
                std::cout << "Execute task in event loop success. thread id: " << thdId << std::endl;
            });

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    {
        std::cout << "NET EVENTLOOP THREAD TEST THIRD -----------------------------" << std::endl;

        // 创建运行并且关闭的eventloop thread，测试退出是否正常
        EventLoopThread::Ptr evThd = std::make_shared<EventLoopThread>("EV_THD_THR_TEST");
        evThd->run();

        EventLoop::WkPtr evLoopWkPtr;
        if (evThd->getEventLoop(evLoopWkPtr)) {
            auto evLoop = evLoopWkPtr.lock();

            auto thdId = evThd->getThreadId();
            evLoop->executeTaskInLoop([thdId]() {
                std::cout << "Execute task in event loop success. thread id: " << thdId << std::endl;
            });

            std::this_thread::sleep_for(std::chrono::seconds(1));

            evLoop->executeTaskInLoop([evLoopWkPtr]() {
                if (!evLoopWkPtr.expired()) {
                    auto evLoop = evLoopWkPtr.lock();
                    evLoop->quit();
                }
            });

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

int main() {
    FuncTest();

    return 0;
}