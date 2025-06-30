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

void FuncTestFst() {
    std::cout << "TIMER TEST FST -----------------------------" << std::endl;

    EventLoop::Ptr loop = std::make_shared<EventLoop>("EV_TEST");
    loop->init();

    FuncShowTime();

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

    {
        TimerId id;
        loop->addTimerAfterSpecificTime(id, [loop]() {
            loop->quit();
        }, 6);
        std::cout << "Add third timer task. id: " << id << std::endl;
    }

    loop->loop();
}

void FuncTestSec() {
    std::cout << "TIMER TEST SEC -----------------------------" << std::endl;

    EventLoop::Ptr loop = std::make_shared<EventLoop>("EV_TEST");
    loop->init();

    FuncShowTime();

    {
        TimerId id;
        Timestamp taskTime = std::chrono::system_clock::now() + std::chrono::seconds(2);

        loop->addTimerAtSpecificTime(id, FuncShowTime, taskTime);
        std::cout << "Add first timer task. id: " << id << std::endl;
    }

    {
        TimerId id;
        loop->addTimerAfterSpecificTime(id, FuncShowTime, 1);
        std::cout << "Add second timer task. id: " << id << std::endl;
    }

    {
        TimerId id;
        loop->addTimerAfterSpecificTime(id, [loop]() {
            loop->quit();
        }, 4);
        std::cout << "Add third timer task. id: " << id << std::endl;
    }

    loop->loop();
}

void FuncTestThr() {
    std::cout << "TIMER TEST THR -----------------------------" << std::endl;

    EventLoop::Ptr loop = std::make_shared<EventLoop>("EV_TEST");
    loop->init();

    FuncShowTime();

    {
        TimerId id;
        Timestamp taskTime = std::chrono::system_clock::now() + std::chrono::seconds(1);

        loop->addTimerAtSpecificTime(id, FuncShowTime, taskTime, 1);
        std::cout << "Add first timer task. id: " << id << std::endl;
    }

    {
        TimerId id;
        loop->addTimerAfterSpecificTime(id, FuncShowTime, 2, 2);
        std::cout << "Add second timer task. id: " << id << std::endl;
    }

    {
        TimerId id;
        loop->addTimerAfterSpecificTime(id, [loop]() {
            loop->quit();
        }, 10);
        std::cout << "Add third timer task. id: " << id << std::endl;
    }

    loop->loop();
}

void FuncTestFor() {
    std::cout << "TIMER TEST FOR -----------------------------" << std::endl;

    EventLoop::Ptr loop = std::make_shared<EventLoop>("EV_TEST");
    loop->init();

    FuncShowTime();

    TimerId id;
    {
        loop->addTimerAfterSpecificTime(id, FuncShowTime, 2, 2);
        std::cout << "Add first timer task. id: " << id << std::endl;
    }

    {
        // 移除定时器
        TimerId delId;
        loop->addTimerAfterSpecificTime(delId, [id, loop]() {
            bool ret = loop->delTimer(id);
            std::cout << "Del timer task. id: " << id << " ret: " << ret << std::endl;
        }, 6);
    }

    {
        TimerId quitId;
        loop->addTimerAfterSpecificTime(quitId, [loop]() {
            loop->quit();
        }, 10);
        std::cout << "Add second timer task. id: " << id << std::endl;
    }

    loop->loop();
}

void FuncTestFif() {
    std::cout << "TIMER TEST FIF -----------------------------" << std::endl;

    EventLoop::Ptr loop = std::make_shared<EventLoop>("EV_TEST");
    loop->init();

    FuncShowTime();

    TimerId id;
    {
        loop->addTimerAfterSpecificTime(id, FuncShowTime, 2, 2);
        std::cout << "Add first timer task. id: " << id << std::endl;
    }

    std::thread delThread = std::thread([id, loop]() {
        std::this_thread::sleep_for(std::chrono::seconds(8));
        bool ret = loop->delTimer(id);
        std::cout << "Del timer task. id: " << id << " ret: " << ret << std::endl;
    });

    {
        TimerId quitId;
        loop->addTimerAfterSpecificTime(quitId, [loop]() {
            loop->quit();
        }, 10);
        std::cout << "Add second timer task. id: " << id << std::endl;
    }

    loop->loop();
    delThread.join();
}

void FuncTestSix() {
    std::cout << "TIMER TEST SIX -----------------------------" << std::endl;

    EventLoop::Ptr loop = std::make_shared<EventLoop>("EV_TEST");
    loop->init();

    FuncShowTime();

    TimerId id;
    {
        loop->addTimerAfterSpecificTime(id, FuncShowTime, 2);
        std::cout << "Add first timer task. id: " << id << std::endl;
    }

    std::thread delThread = std::thread([id, loop]() {
        std::this_thread::sleep_for(std::chrono::seconds(8));
        bool ret = loop->delTimer(id);
        std::cout << "Del timer task. id: " << id << " ret: " << ret << std::endl;
    });

    {
        TimerId quitId;
        loop->addTimerAfterSpecificTime(quitId, [loop]() {
            loop->quit();
        }, 10);
        std::cout << "Add second timer task. id: " << id << std::endl;
    }

    loop->loop();
    delThread.join();
}

int main() {
    FuncTestFst();
    FuncTestSec();
    FuncTestThr();
    FuncTestFor();
    FuncTestFif();
    FuncTestSix();

    return 0;
}