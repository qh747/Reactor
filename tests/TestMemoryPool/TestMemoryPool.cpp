#include <cstring>
#include <iostream>
#include <Memory/MemoryPool.h>
using namespace Memory;

void FuncTestFst() {
    std::cout << "MEMORY POOL TEST FIRST -----------------------------" << std::endl;
    auto pool = std::make_shared<MemoryPool>("MEM_POOL_1", 1024);

    auto mem = pool->allocateMemory(512);
    if (nullptr == mem) {
        std::cerr << "Allocate memory error. id: " << pool->getId();
        return;
    }

    auto fmtMem = static_cast<char*>(mem);
    strncpy(fmtMem, "memory pool allocate 512 bytes", strlen("memory pool allocate 512 bytes"));

    std::cout << "Allocate memory success. id: " << pool->getId() << " value: " << fmtMem << std::endl;
}

int main() {
    FuncTestFst();

    return 0;
}
