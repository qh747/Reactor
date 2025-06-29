#include <chrono>
#include <thread>
#include <iostream>
#include <Utils/Utils.h>
#include <Net/EventLoop.h>
using namespace Net;
using namespace Utils;

void FuncShowTime() {
    std::cout << "Current time: " << TimeHelper::GetCurrentTime() << std::endl;
}


void FuncExitLoop(const EventLoop::WkPtr& wkLoop, int waitSec) {
    std::this_thread::sleep_for(std::chrono::seconds(waitSec));
    if (!wkLoop.expired()) {
        wkLoop.lock()->quit();
    }
}

void FuncTestFst() {
    std::cout << "TIMER TEST FST -----------------------------" << std::endl;

    EventLoop::Ptr loop = std::make_shared<EventLoop>("EV_TEST");
    loop->init();

    FuncShowTime();

    std::thread exitThread = std::thread(FuncExitLoop, loop, 6);

    {
        TimerId id;
        Timestamp taskTime = std::chrono::system_clock::now() + std::chrono::seconds(2);

        loop->addTimerAtSpecificTime(id, FuncShowTime, taskTime);
        std::cout << "Add first timer task. id: " << id << std::endl;
    }

    {
        TimerId id;
        loop->addTimerAfterSpecificTime(id, FuncShowTime, 4);
        std::cout << "Add second timer task. id: " << id << std::endl;
    }

    loop->loop();
    exitThread.join();
}

int main() {
    FuncTestFst();

    return 0;
}