#include <thread>
#include <memory>
#include <iostream>
#include <Net/EventLoop.h>
using namespace Net;

void FuncTest() {
    std::cout << "NET EVENTLOOP TEST -----------------------------" << std::endl;

    EventLoop::Ptr loop = std::make_shared<EventLoop>("EV_TEST");
    loop->init();

    // 指定10秒后退出事件循环
    EventLoop::WkPtr wkLoop = loop->weak_from_this();
    std::thread loopExitThread = std::thread([wkLoop]() {
        std::this_thread::sleep_for(std::chrono::seconds(3));

        if (!wkLoop.expired()) {
            auto loop = wkLoop.lock();
            std::cout << "Exit event loop. id: " << loop->getId() << std::endl;
            loop->quit();
        }
    });

    // 指定1秒后执行任务
    std::thread executeTaskThread = std::thread([wkLoop]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        if (!wkLoop.expired()) {
            auto loop = wkLoop.lock();
            std::string id = loop->getId();
            loop->executeTaskInLoop([id]() {
                std::cout << "Execute task in event loop success. id: " << id << std::endl;
            });
        }
    });

    loop->loop();
    executeTaskThread.join();
    loopExitThread.join();
}

int main() {
    FuncTest();
    return 0;
}