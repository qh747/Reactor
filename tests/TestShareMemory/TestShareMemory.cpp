#include <string>
#include <random>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <Utils/Logger.h>
#include <Utils/Utils.h>
#include <Memory/ShareMemory.h>
using namespace Utils;
using namespace Memory;

int GetUserInput() {
    int input;
    while (true) {
        std::cout << "Please input number (1 - read，2 - write，3 - exit): ";
        std::cin >> input;

        if (input == 1 || input == 2 || input == 3) {
            break;
        }

        std::cout << "Input error: " << input << std::endl;
    }

    return input;
}

std::string GetRandomString() {
    const std::string charset =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<size_t> distribution(0, charset.size() - 1);

    std::string result(32, '\0');
    std::generate_n(result.begin(), 32, [&]() {
        return charset[distribution(generator)];
    });

    return result;
}

void FuncTestFirst() {
    LOG_DEBUG << "SHARE MEMORY TEST FIRST -----------------------------" << std::endl;
    ShareMemory::Ptr shm = nullptr;

    try {
        shm = std::make_shared<ShareMemory>("SHM_1", "test.txt");
    }
    catch (std::exception& e) {
        LOG_ERROR << "Create share memory failed: " << e.what() << std::endl;
        return;
    }

    int input = GetUserInput();

    if (2 == input) {
        while (true) {
            LOG_DEBUG << "Press any key to continue(q to quit)...";
            char block;
            std::cin >> block;

            if ('q' == block) {
                break;
            }

            std::string data = GetRandomString();
            auto size = shm->write(const_cast<char*>(data.c_str()), data.size());

            LOG_DEBUG << "Write data success. data: " << data << ", size: " << size;
        }
    }
    else if (1 == input) {
        while (true) {
            LOG_DEBUG << "Press any key to continue(q to quit)...";
            char block;
            std::cin >> block;

            if ('q' == block) {
                break;
            }

            char buf[256] = {0};
            auto size = shm->read(buf, sizeof(buf));

            LOG_DEBUG << "Read data success. data: " << buf << ", size: " << size;
        }
    }
    else {
        LOG_ERROR << "Invalid input:  " << input << std::endl;
    }
}

int main() {
    FuncTestFirst();

    return 0;
}