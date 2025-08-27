#include "Memory/ShareMemory.h"

namespace Memory {

ShareMemory::ShareMemory(std::string id, const std::string& name, std::size_t size) {

}

ShareMemory::~ShareMemory() {

}

std::size_t ShareMemory::write(const Memory data, std::size_t size) {

    return size;
}

std::size_t ShareMemory::read(Memory data, std::size_t size) const {

    return size;
}

}; // namespace Memory
