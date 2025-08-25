#include <cmath>
#include <cstdlib>
#include <cstring>
#include <utility>
#include <unistd.h>
#include "Utils/Logger.h"
#include "Memory/MemoryPool.h"

namespace Memory {

MemoryPool::MemoryPool(std::string id, std::size_t size) : m_id(std::move(id)) {
    // 申请内存池空间
    Memory mem = nullptr;
    if (0 != posix_memalign(&mem, MEMORY_POOL_ALIGNMENT, size)) {
        LOG_ERROR << "Create memory pool error. posix_memalign() failed. id: " << m_id;
        return;
    }

    // 初始化内存池
    m_pool = static_cast<MemoryPoolPtr>(mem);

    m_pool->data.start  = reinterpret_cast<uint8_t*>(m_pool) + sizeof(MemoryPool_dt);
    m_pool->data.end    = reinterpret_cast<uint8_t*>(m_pool) + size;
    m_pool->data.next   = nullptr;
    m_pool->data.failed = 0;

    m_pool->maxSize = std::min(size - sizeof(MemoryPool_dt), static_cast<std::size_t>(getpagesize() - 1));
    m_pool->current = m_pool;
    m_pool->large   = nullptr;
    m_pool->clean   = nullptr;

    LOG_DEBUG << "Construct memory pool. id: " << m_id
        << " size: "  << size << " pool: " << mem
        << " start: " << static_cast<void*>(m_pool->data.start)
        << " end: "   << static_cast<void*>(m_pool->data.end)
        << " max: "   << m_pool->maxSize
        << " curr: "  << m_pool->current;
}

MemoryPool::~MemoryPool() {
    if (nullptr != m_pool) {
        // 调用自定义清理内存函数
        for (auto clean = m_pool->clean; nullptr != clean; clean = clean->next) {
            if (nullptr != clean->cb) {
                LOG_DEBUG << "Release self manage memory. id: " << m_id << " addr: " << clean->data;
                clean->cb(clean->data);
            }
        }

        // 释放大块内存空间
        int blockBlockCount = 0;
        for (auto large = m_pool->large; nullptr != large; large = large->next) {
            if (nullptr != large->data) {
                LOG_DEBUG << "Release large memory. id: " << m_id << " addr: " << large->data << " count: " <<  ++blockBlockCount;
                free(large->data);
            }
        }

        // 释放小块内存空间
        blockBlockCount = 0;
        for (auto small = m_pool, next = m_pool->data.next; /* void */ ; small = next, next = next->data.next) {
            LOG_DEBUG << "Release small memory. id: " << m_id << " addr: " << static_cast<void*>(small) << " count: " <<  ++blockBlockCount;
            free(small);

            if (nullptr == next) {
                break;
            }
        }
    }

    LOG_DEBUG << "Destruct memory pool. id: " << m_id;
}

void MemoryPool::reset() const {
    if (nullptr != m_pool) {
        // 释放大块内存空间
        int blockBlockCount = 0;
        if (nullptr != m_pool->large) {
            for (auto large = m_pool->large; nullptr != large; large = large->next) {
                if (nullptr != large->data) {
                    LOG_DEBUG << "Release large memory. id: " << m_id << " addr: " << large->data << " count: " <<  ++blockBlockCount;
                    free(large->data);
                }
            }

            m_pool->large = nullptr;
        }

        // 重置小块内存空间的起始位置和分配失败次数
        blockBlockCount = 0;
        for (auto small = m_pool; nullptr != small; small = small->data.next) {
            auto startBefore = small->data.start;
            small->data.start = reinterpret_cast<uint8_t*>(small + sizeof(MemoryPool_dt));
            small->data.failed = 0;

            LOG_DEBUG << "Reset small memory. id: " << m_id << " old addr: " << static_cast<void*>(startBefore)
                << " old addr: " << static_cast<void*>(small->data.start) << " count: " <<  ++blockBlockCount;
        }

        auto currentBefore = m_pool->current;
        m_pool->current = m_pool;

        LOG_DEBUG << "Reset current. id: " << m_id << " old addr: " << static_cast<void*>(currentBefore)
            << " old addr: " << static_cast<void*>(m_pool->current) << " count: " <<  ++blockBlockCount;
    }

    LOG_DEBUG << "Reset memory pool. id: " << m_id;
}

MemoryPool::Memory MemoryPool::allocateMemory(std::size_t size) const {
    if (nullptr == m_pool) {
        LOG_ERROR << "Allocate memory error. poll invalid. id: " << m_id << " size: " << size;
        return nullptr;
    }

    if (size <= m_pool->maxSize) {
        LOG_DEBUG << "Allocate small memory. id: " << m_id << " size: " << size << " max size: " <<  m_pool->maxSize;
        return this->allocateSmallMemory(size, true);
    }
    else {
        LOG_DEBUG << "Allocate large memory. id: " << m_id << " size: " << size << " max size: " <<  m_pool->maxSize;
        return this->allocateLargeMemory(size);
    }
}

MemoryPool::Memory MemoryPool::allocateMemory(std::size_t size, std::size_t align) const {
    if (nullptr == m_pool) {
        LOG_ERROR << "Allocate memory error. poll invalid. id: " << m_id << " size: " << size << " align: " << align;
        return nullptr;
    }

    // 创建内存
    Memory mem = nullptr;
    if (0 != posix_memalign(&mem, align, size)) {
        LOG_ERROR << "Allocate memory error. posix_memalign() failed. id: " << m_id;
        return nullptr;
    }

    // 创建大块内存管理节点
    auto large = static_cast<MemoryPoolLargeBlockPtr>(this->allocateSmallMemory(sizeof(MemoryPoolLargeBlock_dt), true));
    if (nullptr == large) {
        LOG_ERROR << "Allocate memory error. create large memory block failed. id: " << m_id << " size: " << size << " align: " << align;
        free(mem);
        return nullptr;
    }

    // 记录存储数据的内存地址
    large->data = mem;

    // 将当前大块内存节点链接到大块内存链表首部
    large->next = m_pool->large;
    m_pool->large = large;

    LOG_DEBUG << "Allocate memory success. id: " << m_id << " size: " << size << " align: " << align << " addr: " << mem;
    return mem;
}

MemoryPool::Memory MemoryPool::allocateMemory(std::size_t size, const CleanCb& cb) const {
    if (nullptr == m_pool) {
        LOG_ERROR << "Allocate memory with self clean function error. poll invalid. id: " << m_id << " size: " << size;
        return nullptr;
    }

    // 申请内存
    auto mem = this->allocateMemory(size);
    if (nullptr == mem) {
        LOG_ERROR << "Allocate memory with self clean function error. call MemoryPool::allocateMemory(std::size_t size) failed. id: " << m_id
            << " size: " << size;
        return nullptr;
    }

    // 申请自定义释放函数内存管理节点
    auto clean = static_cast<MemoryPoolCleanPtr>(this->allocateSmallMemory(sizeof(MemoryPoolClean_dt), true));
    if (nullptr == clean) {
        LOG_ERROR << "Allocate memory with self clean function error. create memory clean node failed. id: " << m_id << " size: " << size;
        free(mem);
        return nullptr;
    }

    // 记录自定义释放函数
    clean->data = mem;
    clean->cb = cb;

    // 将当前自定义释放函数节点链接到自定义释放函数链表首部
    clean->next = m_pool->clean;
    m_pool->clean = clean;

    LOG_DEBUG << "Allocate memory with self clean function success. id: " << m_id << " size: " << size << " addr: " << mem;
    return mem;
}

MemoryPool::Memory MemoryPool::nallocateMemory(std::size_t size) const {
    if (nullptr == m_pool) {
        LOG_ERROR << "NAllocate memory error. poll invalid. id: " << m_id << " size: " << size;
        return nullptr;
    }

    if (size <= m_pool->maxSize) {
        LOG_DEBUG << "NAllocate small memory. id: " << m_id << " size: " << size << " max size: " <<  m_pool->maxSize;
        return this->allocateSmallMemory(size, false);
    }
    else {
        LOG_DEBUG << "NAllocate large memory. id: " << m_id << " size: " << size << " max size: " <<  m_pool->maxSize;
        return this->allocateLargeMemory(size);
    }
}

MemoryPool::Memory MemoryPool::callocateMemory(std::size_t size) const {
    if (nullptr == m_pool) {
        LOG_ERROR << "CAllocate memory error. poll invalid. id: " << m_id << " size: " << size;
        return nullptr;
    }

    auto mem = allocateMemory(size);
    if (nullptr != mem) {
        memset(mem, 0, size);
    }

    LOG_DEBUG << "CAllocate memory success. id: " << m_id << " size: " << size << " addr: " << mem;
    return mem;
}

bool MemoryPool::freeMemory(Memory data) const {
    if (nullptr == m_pool) {
        LOG_ERROR << "Free memory error. poll invalid. id: " << m_id << " addr: " << data;
        return false;
    }

    for (auto large = m_pool->large; nullptr != large; large = large->next) {
        if (data == large->data) {
            LOG_DEBUG << "Free memory success. id: " << m_id << " addr: " << data;

            free(large->data);
            large->data = nullptr;

            return true;
        }
    }

    LOG_ERROR << "Free memory error. large memory node not found. id: " << m_id << " addr: " << data;
    return false;
}

MemoryPool::Memory MemoryPool::allocateSmallMemory(std::size_t size, bool align) const {
    if (nullptr == m_pool) {
        LOG_ERROR << "Allocate small memory error. poll invalid. id: " << m_id << " size: " << size << " align: " << align;
        return nullptr;
    }

    // 从当前内存池申请内存
    auto current = m_pool->current;
    do {
        auto start = current->data.start;

        /**
         * 对齐内存地址，将m的地址按照MEMORY_ALIGNMENT对齐方式向上取整
         * 例如：m = 0x1007, NGX_ALIGNMENT = 8, 则m = 0x1008
         */
        if (align) {
            start = MemoryAddrHelper::GetAlignAddr(start, MEMORY_ALIGNMENT);
        }

        // 当前内存块剩余空间足够本次内存申请
        auto remainSize = static_cast<std::size_t>(current->data.end - start);
        if (remainSize >= size) {
            current->data.start = start + size;

            LOG_DEBUG << "Allocate small memory success. id: " << m_id << " size: " << size << " align: " << align << " addr: "
                << static_cast<void*>(start) << " addr after: " << static_cast<void*>(current->data.start) << " remain size: " << remainSize;
            return start;
        }

        // 当前内存块剩余空间不够本次内存申请，切换到下一个内存块
        current = current->data.next;

    } while (nullptr != current);

    // 申请新的内存块
    return this->allocateNewMemory(size);
}

MemoryPool::Memory MemoryPool::allocateLargeMemory(std::size_t size) const {
    if (nullptr == m_pool) {
        LOG_ERROR << "Allocate small memory error. poll invalid. id: " << m_id << " size: " << size;
        return nullptr;
    }

    auto mem = malloc(size);
    if (nullptr == mem) {
        LOG_ERROR << "Allocate large memory error. malloc() failed. id: " << m_id << " size: " << size;
        return nullptr;
    }

    // 从已有的大块内存链表里寻找空闲的节点
    int count = 0;
    auto large = m_pool->large;
    for (/* void */; nullptr != large; large = large->next) {
        // 查找未使用的记录大块内存的节点
        if (nullptr == large->data) {
            LOG_DEBUG << "Allocate large memory success. id: " << m_id << " size: " << size << " addr: " << mem;
            large->data = mem;
            return mem;
        }

        // 只查找前三个节点
        if (++count >= 3) {
            break;
        }
    }

    // 创建大块内存管理节点
    large = static_cast<MemoryPoolLargeBlockPtr>(this->allocateSmallMemory(sizeof(MemoryPoolLargeBlock_dt), true));
    if (nullptr == large) {
        LOG_ERROR << "Allocate large memory error. create large block failed. id: " << m_id << " size: " << size;
        free(mem);
        return nullptr;
    }

    // 记录存储数据的内存地址
    large->data = mem;

    // 将当前大块内存节点链接到大块内存链表首部
    large->next = m_pool->large;
    m_pool->large = large;

    LOG_DEBUG << "Allocate large memory success. create new large block. id: " << m_id << " size: " << size << " addr: " << mem;
    return mem;
}

MemoryPool::Memory MemoryPool::allocateNewMemory(std::size_t size) const {
    if (nullptr == m_pool) {
        LOG_ERROR << "Allocate new memory error. poll invalid. id: " << m_id << " size: " << size;
        return nullptr;
    }

    // 创建新的内存块
    Memory mem = nullptr;
    auto memSize = m_pool->data.end - reinterpret_cast<uint8_t*>(m_pool);

    if (0 != posix_memalign(&mem, MEMORY_POOL_ALIGNMENT, memSize)) {
        LOG_ERROR << "Allocate new memory error. posix_memalign() failed. id: " << m_id << " size: " << size;
        return nullptr;
    }

    /**
     * 跳过内存池节点数据结构
     * 只有内存池的首节点会保留d、current、chain、large、cleanup、log字段
     * 后续新分配的内存池节点只保留d字段
     */
    auto start = MemoryAddrHelper::GetAlignAddr(static_cast<uint8_t*>(mem) + sizeof(MemoryPoolSmallBlock_dt), MEMORY_ALIGNMENT);

    // 跳过本次分配的大小
    auto memBlock         = static_cast<MemoryPoolPtr>(mem);
    memBlock->data.start  = start + size;
    memBlock->data.end    = static_cast<uint8_t*>(mem) + memSize;
    memBlock->data.next   = nullptr;
    memBlock->data.failed = 0;

    // 过滤不可用的内存池节点，判断方式：如果该内存池节点分配失败次数超过4，后续不用该内存池节点
    auto current = m_pool->current;
    for (/* void */; nullptr != current->data.next; current = current->data.next) {
        if (++current->data.failed >= 4) {
            m_pool->current = current->data.next;
        }
    }

    current->data.next = memBlock;

    LOG_DEBUG << "Allocate new memory success. id: " << m_id << " size: " << size << " addr: " << start;
    return start;
}

} // namespace Memory