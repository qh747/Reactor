#pragma once
#include <string>
#include <memory>
#include "Utils/Utils.h"
#include "Common/memDef.h"
using namespace Utils;
using namespace Common;

namespace Memory {

/**
 * @brief 内存池类
 */
class MemoryPool : public Noncopyable, public std::enable_shared_from_this<MemoryPool> {
public:
    using Ptr = std::shared_ptr<MemoryPool>;
    using WkPtr = std::weak_ptr<MemoryPool>;
    using Memory = void*;

public:
    explicit MemoryPool(std::string id, std::size_t size);
    ~MemoryPool();

public:
    /**
     * @brief           重置内存池
     */
    void                reset() const;

    /**
     * @brief           申请内存
     * @return          内存指针
     * @param           size 申请内存大小
     */
    Memory              allocateMemory(std::size_t size) const;

    /**
     * @brief           申请内存
     * @return          内存指针
     * @param           size 申请内存大小
     * @param           align 内存地址首字节对齐大小
     */
    Memory              allocateMemory(std::size_t size, std::size_t align) const;

    /**
     * @brief           申请内存
     * @return          内存指针
     * @param           size 申请内存大小
     * @param           cb 自定义内存释放回调函数
     */
    Memory              allocateMemory(std::size_t size, const CleanCb& cb) const;

    /**
     * @brief           申请内存，并初始化内存为0
     * @return          内存指针
     * @param           size 申请内存大小
     */
    Memory              nallocateMemory(std::size_t size) const;

    /**
     * @brief           申请内存，内存首地址随机
     * @return          内存指针
     * @param           size 申请内存大小
     */
    Memory              callocateMemory(std::size_t size) const;

    /**
     * @note            仅大块内存会释放，小块内存在内存池析构时释放
     * @brief           释放内存
     * @return          释放结果
     * @param           data 内存首地址
     */
    bool                freeMemory(Memory data) const;

private:
    /**
     * @brief           申请小块内存
     * @return          内存指针
     * @param           size 申请内存大小
     * @param           align 是否对内存地址首字节对齐
     */
    Memory              allocateSmallMemory(std::size_t size, bool align) const;

    /**
     * @brief           申请大块内存
     * @return          内存指针
     * @param           size 申请内存大小
     */
    Memory              allocateLargeMemory(std::size_t size) const;

    /**
     * @brief           申请新的内存块
     * @return          内存指针
     * @param           size 申请内存大小
     */
    Memory              allocateNewMemory(std::size_t size) const;

public:
    /**
     * @brief           获取内存池id
     * @return          内存池id
     */
    inline std::string  getId() const { return m_id; }

    /**
     * @brief           判断内存池是否有效
     * @return          判断结果
     */
    inline bool         isValid() const { return m_pool != nullptr; }

private:
    //                  内存池数据指针
    MemoryPoolPtr       m_pool;

    //                  内存池ID
    std::string         m_id;
};

}; // namespace Memory