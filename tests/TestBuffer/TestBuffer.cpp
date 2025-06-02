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
    buffer->write(reinterpret_cast<const uint8_t*>("hello world"), strlen("hello world"));

    std::vector<uint8_t> data;
    std::size_t len;
    buffer->read(data, len);

    std::cout << "read data: " << std::string(reinterpret_cast<const char*>(data.data()), len) << std::endl;
}

void FuncTestSnd() { 
    std::cout << "NET BUFFER TEST SECOND -----------------------------" << std::endl;

    Buffer::Ptr buffer = std::make_shared<Buffer>();
    buffer->write(reinterpret_cast<const uint8_t*>("123456789"), strlen("123456789"));

    std::cout << "readable bytes: " << buffer->readableBytes() << std::endl;
    std::cout << "writable bytes: " << buffer->writableBytes() << std::endl;

    std::vector<uint8_t> data;
    buffer->readFixSize(data, 5);
    std::cout << "read data: " << std::string(reinterpret_cast<const char*>(data.data()), 5) << std::endl;

    std::cout << "readable bytes: " << buffer->readableBytes() << std::endl;
    std::cout << "writable bytes: " << buffer->writableBytes() << std::endl;

    std::size_t readableSize = buffer->readableBytes();
    buffer->readFixSize(data, readableSize);
    std::cout << "read data: " << std::string(reinterpret_cast<const char*>(data.data()), readableSize) << std::endl;

    std::cout << "readable bytes: " << buffer->readableBytes() << std::endl;
    std::cout << "writable bytes: " << buffer->writableBytes() << std::endl;
}

int main() {
    FuncTestFst();
    FuncTestSnd();

    return 0;
}