#include <cmath>
#include <cstdlib>
#include <cstring>
#include <utility>
#include <unistd.h>
#include "Utils/Logger.h"
#include "Memory/MemoryPool.h"

namespace Memory {

MemoryPool::MemoryPool(std::string  id, std::size_t size) : m_id(std::move(id)) {
    // 申请内存池空间
    Memory mem = nullptr;
    if (0 != posix_memalign(&mem, MEMORY_POOL_ALIGNMENT, size)) {
        LOG_ERROR << "Create memory pool error. posix_memalign() failed. id" << m_id;
        return;
    }

    // 初始化内存池
    m_pool = static_cast<MemoryPoolPtr>(mem);

    m_pool->data.start  = reinterpret_cast<uint8_t*>(m_pool + sizeof(MemoryPool_dt));
    m_pool->data.end    = reinterpret_cast<uint8_t*>(m_pool + size);
    m_pool->data.next   = nullptr;
    m_pool->data.failed = 0;

    m_pool->maxSize = std::min(size - sizeof(MemoryPool_dt), static_cast<std::size_t>(getpagesize() - 1));
    m_pool->current = m_pool;
    m_pool->large   = nullptr;
    m_pool->clean   = nullptr;

    LOG_DEBUG << "Construct memory pool. id" << m_id;
}

MemoryPool::~MemoryPool() {
    if (nullptr != m_pool) {
        // 调用自定义清理内存函数
        for (auto clean = m_pool->clean; nullptr != clean; clean = clean->next) {
            if (nullptr != clean->cb) {
                clean->cb(clean->data);
            }
        }

        // 释放大块内存空间
        for (auto large = m_pool->large; nullptr != large; large = large->next) {
            if (nullptr != large->data) {
                free(large->data);
            }
        }

        // 释放小块内存空间
        for (auto small = m_pool, next = m_pool->data.next; /* void */ ; small = next, next = next->data.next) {
            free(small);

            if (nullptr == next) {
                break;
            }
        }
    }

    LOG_DEBUG << "Destruct memory pool. id" << m_id;
}

void MemoryPool::reset() const {
    if (nullptr != m_pool) {
        // 释放大块内存空间
        if (nullptr != m_pool->large) {
            for (auto large = m_pool->large; nullptr != large; large = large->next) {
                if (nullptr != large->data) {
                    free(large->data);
                }
            }

            m_pool->large = nullptr;
        }

        // 重置小块内存空间的起始位置和分配失败次数
        for (auto small = m_pool; nullptr != small; small = small->data.next) {
            small->data.start = reinterpret_cast<uint8_t*>(small + sizeof(MemoryPool_dt));
            small->data.failed = 0;
        }

        m_pool->current = m_pool;
    }

    LOG_DEBUG << "Reset memory pool. id" << m_id;
}

MemoryPool::Memory MemoryPool::allocateMemory(std::size_t size) const {
    if (nullptr == m_pool) {
        return nullptr;
    }

    return (size <= m_pool->maxSize) ? this->allocateSmallMemory(size, true) : this->allocateLargeMemory(size);
}

MemoryPool::Memory MemoryPool::allocateMemory(std::size_t size, std::size_t align) const {
    if (nullptr == m_pool) {
        return nullptr;
    }

    // 创建内存
    Memory mem = nullptr;
    if (0 != posix_memalign(&mem, align, size)) {
        LOG_ERROR << "Allocate new memory error. posix_memalign() failed. id" << m_id;
        return nullptr;
    }

    // 创建大块内存管理节点
    auto large = static_cast<MemoryPoolLargeBlockPtr>(this->allocateSmallMemory(sizeof(MemoryPoolLargeBlock_dt), true));
    if (nullptr ==  large) {
        free(mem);
        return nullptr;
    }

    // 记录存储数据的内存地址
    large->data = mem;

    // 将当前大块内存节点链接到大块内存链表首部
    large->next = m_pool->large;
    m_pool->large = large;

    return mem;
}

MemoryPool::Memory MemoryPool::allocateMemory(std::size_t size, const CleanCb& cb) const {
    if (nullptr == m_pool) {
        return nullptr;
    }

    // 申请内存
    auto mem = this->allocateMemory(size);
    if (nullptr == mem) {
        return nullptr;
    }

    // 申请自定义释放函数内存管理节点
    auto clean = static_cast<MemoryPoolCleanPtr>(this->allocateSmallMemory(sizeof(MemoryPoolClean_dt), true));
    if (nullptr == clean) {
        free(mem);
        return nullptr;
    }

    // 记录自定义释放函数
    clean->data = mem;
    clean->cb = cb;

    // 将当前自定义释放函数节点链接到自定义释放函数链表首部
    clean->next = m_pool->clean;
    m_pool->clean = clean;

    return mem;
}

MemoryPool::Memory MemoryPool::nallocateMemory(std::size_t size) const {
    if (nullptr == m_pool) {
        return nullptr;
    }

    return (size <= m_pool->maxSize) ? this->allocateSmallMemory(size, false) : this->allocateLargeMemory(size);
}

MemoryPool::Memory MemoryPool::callocateMemory(std::size_t size) const {
    if (nullptr == m_pool) {
        return nullptr;
    }

    auto mem = allocateMemory(size);
    if (nullptr != mem) {
        memset(mem, 0, size);
    }

    return mem;
}

bool MemoryPool::freeMemory(Memory data) const {
    if (nullptr == m_pool) {
        return false;
    }

    for (auto large = m_pool->large; nullptr != large; large = large->next) {
        if (data == large->data) {
            free(large->data);
            large->data = nullptr;

            return true;
        }
    }

    return false;
}

MemoryPool::Memory MemoryPool::allocateSmallMemory(std::size_t size, bool align) const {
    if (nullptr == m_pool) {
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
        std::size_t remainSize = current->data.end - start;
        if (remainSize >= size) {
            current->data.start = start + size;
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
        return nullptr;
    }

    auto mem = malloc(size);
    if (nullptr == mem) {
        LOG_ERROR << "Allocate large memory error. malloc() failed. id" << m_id;
        return nullptr;
    }

    // 从已有的大块内存链表里寻找空闲的节点
    int count = 0;
    auto large = m_pool->large;
    for (/* void */; nullptr != large; large = large->next) {
        // 查找未使用的记录大块内存的节点
        if (nullptr == large->data) {
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
    if (nullptr ==  large) {
        free(mem);
        return nullptr;
    }

    // 记录存储数据的内存地址
    large->data = mem;

    // 将当前大块内存节点链接到大块内存链表首部
    large->next = m_pool->large;
    m_pool->large = large;

    return mem;
}

MemoryPool::Memory MemoryPool::allocateNewMemory(std::size_t size) const {
    if (nullptr == m_pool) {
        return nullptr;
    }

    // 创建新的内存块
    Memory mem = nullptr;
    std::size_t memSize = m_pool->data.end - reinterpret_cast<uint8_t*>(m_pool);

    if (0 != posix_memalign(&mem, MEMORY_POOL_ALIGNMENT, memSize)) {
        LOG_ERROR << "Allocate new memory error. posix_memalign() failed. id" << m_id;
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
    return start;
}

} // namespace Memory