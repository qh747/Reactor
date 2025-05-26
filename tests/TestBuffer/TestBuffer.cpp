#include <ctime>
#include <chrono>
#include <random>
#include <thread>
#include <memory>
#include <cstring>
#include <iostream>
#include <Utils/Buffer.h>
using namespace Utils;

void FuncTestFst() {
    std::cout << "NET BUFFER TEST FIRST -----------------------------" << std::endl;

    Buffer::Ptr buffer = std::make_shared<Buffer>();
    buffer->write("hello world", strlen("hello world"));

    std::vector<char> data;
    std::size_t len;
    buffer->read(data, len);

    std::cout << "read data: " << std::string(data.data(), len) << std::endl;
}

void FuncTestSnd() { 
    std::cout << "NET BUFFER TEST SECOND -----------------------------" << std::endl;

    Buffer::Ptr buffer = std::make_shared<Buffer>();
    buffer->write("123456789", strlen("123456789"));

    std::cout << "readable bytes: " << buffer->readableBytes() << std::endl;
    std::cout << "writable bytes: " << buffer->writableBytes() << std::endl;

    std::vector<char> data;
    buffer->readFixSize(data, 5);
    std::cout << "read data: " << std::string(data.data(), 5) << std::endl;

    std::cout << "readable bytes: " << buffer->readableBytes() << std::endl;
    std::cout << "writable bytes: " << buffer->writableBytes() << std::endl;

    std::size_t readableSize = buffer->readableBytes();
    buffer->readFixSize(data, readableSize);
    std::cout << "read data: " << std::string(data.data(), readableSize) << std::endl;

    std::cout << "readable bytes: " << buffer->readableBytes() << std::endl;
    std::cout << "writable bytes: " << buffer->writableBytes() << std::endl;
}

void FuncTestTrd() {
    std::cout << "NET BUFFER TEST THIRD -----------------------------" << std::endl;

    ThreadBuffer::Ptr ThdBuf = std::make_shared<ThreadBuffer>();

    std::thread readThread([ThdBuf]() {
        int count = 0;
        while (++count <= 20) {
            if (0 != ThdBuf->readableBytes()) {
                std::vector<char> data;
                ThdBuf->readFixSize(data, 10);

                std::cout << "Thread: " << std::this_thread::get_id() << " reading data: "
                    << std::string(data.data(), data.size()) << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });

    std::thread writeThread([ThdBuf]() {
        const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

        int count = 0;
        while (++count <= 10) {
            if (0 != ThdBuf->writableBytes()) {
                // 生成随机数
                std::default_random_engine engine(static_cast<unsigned>(std::time(nullptr)));
                std::uniform_int_distribution<size_t> distribution(0, charset.size() - 1);

                std::string randomString;
                for (size_t i = 0; i < 10; ++i) {
                    randomString += charset[distribution(engine)];
                }

                // 写入数据
                std::cout << "Thread: " << std::this_thread::get_id() << " writing data: " << randomString << std::endl;
                ThdBuf->write(randomString.c_str(), randomString.length());
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    });

    writeThread.join();
    readThread.join();
}

int main() {
    FuncTestFst();
    FuncTestSnd();
    FuncTestTrd();

    return 0;
}