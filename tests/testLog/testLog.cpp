#include <thread>
#include <iostream>
#include <common/logger.h>
using namespace COMMON;

void FuncTestFst() {
    std::cout << "LOG TEST FIRST -----------------------------" << std::endl;

    LOG_DEBUG << "Debug Test";
    LOG_INFO << "Info Test";
    LOG_WARN << "Warn Test";
    LOG_ERROR << "Error Test";
}

void FuncTestSnd() {
    std::cout << "LOG TEST SECOND -----------------------------" << std::endl;

    Logger::SetLowestLevel(LogLevel::INFO);

    // 不会输出日志
    LOG_DEBUG << "Debug Test";

    // 会输出日志
    LOG_INFO << "Info Test";
    LOG_WARN << "Warn Test";
    LOG_ERROR << "Error Test";
}

void FuncTestTrd() {
    std::cout << "LOG TEST THIRD -----------------------------" << std::endl;

    std::thread thd1([]() {
        for (int i = 0; i < 100; ++i) {
            LOG_DEBUG << "Debug Thread Test " << i;
        }
    });

    std::thread thd2([]() {
        for (int i = 0; i < 100; ++i) {
            LOG_INFO << "Info Thread Test " << i;
        }
    });

    std::thread thd3([]() {
        for (int i = 0; i < 100; ++i) {
            LOG_WARN << "Warn Thread Test " << i;
        }
    });

    std::thread thd4([]() {
        for (int i = 0; i < 100; ++i) {
            LOG_ERROR << "Error Thread Test " << i;
        }
    });

    thd1.join();
    thd2.join();
    thd3.join();
    thd4.join();
}

int main() {
    FuncTestFst();
    FuncTestSnd();
    FuncTestTrd();

    return 0;
}