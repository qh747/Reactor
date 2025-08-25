#include <cstring>
#include <sstream>
#include <iostream>
#include <Utils/Logger.h>
#include <Utils/Utils.h>
#include <Memory/MemoryPool.h>
using namespace Utils;
using namespace Memory;

void FuncTestFirst() {
    std::cout << "MEMORY POOL TEST FIRST -----------------------------" << std::endl;
    auto pool = std::make_shared<MemoryPool>("MEM_POOL_1", 1024);

    auto mem = pool->allocateMemory(512);
    if (nullptr == mem) {
        std::cerr << "Allocate memory error. id: " << pool->getId();
        return;
    }

    auto fmtMem = static_cast<char*>(mem);
    strncpy(fmtMem, "memory pool allocate 512 bytes", strlen("memory pool allocate 512 bytes"));

    std::cout << "Allocate memory success. id: " << pool->getId() << " value: " << fmtMem << " addr: " <<  mem << std::endl;
}

void FuncTestSecond() {
    std::cout << "MEMORY POOL TEST SECOND -----------------------------" << std::endl;
    auto pool = std::make_shared<MemoryPool>("MEM_POOL_2", 1024);

    // 第一次申请512字节内存
    auto mem = pool->allocateMemory(512);
    if (nullptr == mem) {
        std::cerr << "Allocate first memory error. id: " << pool->getId();
        return;
    }
    auto fmtMem = static_cast<char*>(mem);
    strncpy(fmtMem, "memory pool first allocate 512 bytes", strlen("memory pool first allocate 512 bytes"));
    std::cout << "Allocate first memory success. id: " << pool->getId() << " value: " << fmtMem << " addr: " <<  mem << std::endl;

    // 第二次申请1024字节内存
    auto secMem = pool->allocateMemory(1024);
    if (nullptr == secMem) {
        std::cerr << "Allocate second memory error. id: " << pool->getId();
        return;
    }

    fmtMem = static_cast<char*>(secMem);
    strncpy(fmtMem, "memory pool second allocate 1024 bytes", strlen("memory pool second allocate 1024 bytes"));
    std::cout << "Allocate second memory success. id: " << pool->getId() << " value: " << fmtMem << " addr: " << secMem << std::endl;
}

void FuncTestThird() {
    LOG_DEBUG << "MEMORY POOL TEST THIRD -----------------------------" << std::endl;
    auto pool = std::make_shared<MemoryPool>("MEM_POOL_3", 1024);

    int count = 10;
    for (int idx = 0; idx < count; ++idx) {
        LOG_DEBUG << "Start allocate memory -----------------------------" << std::endl;
        auto mem = pool->allocateMemory(512);
        if (nullptr == mem) {
            LOG_ERROR << "Allocate memory error. id: " << pool->getId() << " count:  " << idx;
            return;
        }

        auto fmtMem = static_cast<char*>(mem);
        strncpy(fmtMem, "memory pool allocate 512 bytes", strlen("memory pool allocate 512 bytes"));
        LOG_DEBUG << "Allocate memory success. id: " << pool->getId() << " value: " << fmtMem << " addr: " <<  mem << " count: " << idx << std::endl;
    }
}

void FuncTestFourth() {
    LOG_DEBUG << "MEMORY POOL TEST FORTH -----------------------------" << std::endl;
    auto pool = std::make_shared<MemoryPool>("MEM_POOL_4", 1024);

    int count = 10;
    for (int idx = 0; idx < count; ++idx) {
        LOG_DEBUG << "Start allocate memory -----------------------------" << std::endl;
        std::size_t memSize = RandomHelper::GetRandomAddr(100, 2000);
        std::size_t alignSize = 8;

        auto mem = pool->allocateMemory(memSize, alignSize);
        if (nullptr == mem) {
            LOG_ERROR << "Allocate memory error. id: " << pool->getId() << " count:  " << idx << " size: " << memSize
                << " align: " << alignSize << std::endl;
            return;
        }

        std::stringstream ss;
        ss << "memory pool allocate " << memSize << " bytes" << "  align size: " << alignSize;

        auto fmtMem = static_cast<char*>(mem);
        strncpy(fmtMem, ss.str().c_str(), ss.str().size());
        LOG_DEBUG << "Allocate memory success. id: " << pool->getId() << " addr: " << mem << " size: " << memSize << " align: " << alignSize
            << " count: " << idx << " value: " << fmtMem << std::endl;
    }
}

void FuncTestFifth() {
    LOG_DEBUG << "MEMORY POOL TEST FIFTH -----------------------------" << std::endl;
    auto pool = std::make_shared<MemoryPool>("MEM_POOL_5", 1024);
    auto id = pool->getId();

    auto fstMem = pool->allocateMemory(1024, [id](void* mem) {
        LOG_DEBUG << "Free first memory. id: " << id << " addr: " << mem << " data: " <<  static_cast<char*>(mem) << std::endl;
    });

    if (nullptr == fstMem) {
        LOG_ERROR << "Allocate first memory error. id: " << id << std::endl;
        return;
    }

    auto fmtMem = static_cast<char*>(fstMem);
    strncpy(fmtMem, "memory pool allocate first memory. size: 1024.", strlen("memory pool allocate first memory. size: 1024."));

    auto secMem = pool->allocateMemory(1024, [id](void* mem) {
        LOG_DEBUG << "Free second memory. id: " << id << " addr: " << mem << " data: " <<  static_cast<char*>(mem) << std::endl;
    });

    if (nullptr == secMem) {
        LOG_ERROR << "Allocate second memory error. id: " << id << std::endl;
        return;
    }

    fmtMem = static_cast<char*>(secMem);
    strncpy(fmtMem, "memory pool allocate second memory. size: 1024.", strlen("memory pool allocate second memory. size: 1024."));
}

void FuncTestSixth() {
    LOG_DEBUG << "MEMORY POOL TEST SIXTH -----------------------------" << std::endl;
    auto pool = std::make_shared<MemoryPool>("MEM_POOL_6", 1024);

    int count = 10;
    for (int idx = 0; idx < count; ++idx) {
        LOG_DEBUG << "Start allocate memory -----------------------------" << std::endl;
        auto mem = pool->nallocateMemory(512);
        if (nullptr == mem) {
            LOG_ERROR << "Allocate memory error. id: " << pool->getId() << " count:  " << idx;
            return;
        }

        auto fmtMem = static_cast<char*>(mem);
        strncpy(fmtMem, "memory pool allocate 512 bytes", strlen("memory pool allocate 512 bytes"));
        LOG_DEBUG << "Allocate memory success. id: " << pool->getId() << " value: " << fmtMem << " addr: " <<  mem << " count: " << idx << std::endl;
    }
}

void FuncTestSeventh() {
    LOG_DEBUG << "MEMORY POOL TEST SEVENTH -----------------------------" << std::endl;
    auto pool = std::make_shared<MemoryPool>("MEM_POOL_7", 1024);

    int count = 10;
    for (int idx = 0; idx < count; ++idx) {
        LOG_DEBUG << "Start allocate memory -----------------------------" << std::endl;
        auto mem = pool->callocateMemory(256);
        if (nullptr == mem) {
            LOG_ERROR << "Allocate memory error. id: " << pool->getId() << " count:  " << idx;
            return;
        }

        auto fmtMem = static_cast<char*>(mem);
        strncpy(fmtMem, "memory pool allocate 256 bytes", strlen("memory pool allocate 256 bytes"));
        LOG_DEBUG << "Allocate memory success. id: " << pool->getId() << " value: " << fmtMem << " addr: " <<  mem << " count: " << idx << std::endl;
    }
}

void FuncTestEighth() {
    LOG_DEBUG << "MEMORY POOL TEST EIGHTH -----------------------------" << std::endl;
    auto pool = std::make_shared<MemoryPool>("MEM_POOL_8", 1024);

    auto memSize = pool->getMaxSize() + 1;
    auto mem = pool->allocateMemory(memSize);
    if (nullptr == mem) {
        LOG_ERROR << "Allocate memory error. id: " << pool->getId() << " size: " << memSize;
        return;
    }

    std::stringstream ss;
    ss << "Allocate large memory. size: " << memSize;

    auto fmtMem = static_cast<char*>(mem);
    strncpy(fmtMem, ss.str().c_str(), ss.str().size());
    LOG_DEBUG << "Allocate memory success. id: " << pool->getId() << " value: " << fmtMem << " addr: " << mem << std::endl;

    if (pool->freeMemory(mem)) {
        LOG_DEBUG << "Free memory success. id: " << pool->getId() << " size: " << memSize;
    }
    else {
        LOG_ERROR << "Free memory error. id: " << pool->getId() << " size: " << memSize;
    }
}

void FuncTestNinth() {
    LOG_DEBUG << "MEMORY POOL TEST NINTH -----------------------------" << std::endl;
    auto pool = std::make_shared<MemoryPool>("MEM_POOL_9", 1024);

    LOG_DEBUG << "Start first allocate memory cycle -----------------------------" << std::endl;
    int count = 10;
    for (int idx = 0; idx < count; ++idx) {
        auto size = ( idx % 2 == 0) ? 512 : 1024;
        auto mem = pool->allocateMemory(size);
        if (nullptr == mem) {
            LOG_ERROR << "Allocate memory in first cycle error. id: " << pool->getId() << " count:  " << idx;
            return;
        }

        auto fmtMem = static_cast<char*>(mem);
        strncpy(fmtMem, "memory pool allocate in first cycle", strlen("memory pool allocate in first cycle"));
        LOG_DEBUG << "Allocate memory in first cycle success. id: " << pool->getId() << " value: " << fmtMem << " addr: " <<  mem
            << " count: " << idx << std::endl;
    }

    pool->reset();

    LOG_DEBUG << "Start second allocate memory cycle -----------------------------" << std::endl;
    for (int idx = 0; idx < count; ++idx) {
        auto size = ( idx % 2 == 0) ? 512 : 1024;
        auto mem = pool->allocateMemory(size);
        if (nullptr == mem) {
            LOG_ERROR << "Allocate memory in second cycle error. id: " << pool->getId() << " count:  " << idx;
            return;
        }

        auto fmtMem = static_cast<char*>(mem);
        strncpy(fmtMem, "memory pool allocate in second cycle", strlen("memory pool allocate in second cycle"));
        LOG_DEBUG << "Allocate memory in second cycle success. id: " << pool->getId() << " value: " << fmtMem << " addr: " <<  mem
            << " count: " << idx << std::endl;
    }
}

int main() {
    // FuncTestFirst();
    // FuncTestSecond();
    // FuncTestThird();
    // FuncTestFourth();
    // FuncTestFifth();
    // FuncTestSixth();
    // FuncTestSeventh();
    // FuncTestEighth();
    FuncTestNinth();

    return 0;
}
