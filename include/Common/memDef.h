#pragma once
#include <cstdint>
#include <functional>

namespace Common {

// 内存池申请首字节对齐大小
constexpr static uint32_t MEMORY_POOL_ALIGNMENT = 16;

// 内存申请首字节对齐大小
constexpr static uint32_t MEMORY_ALIGNMENT = sizeof(unsigned long);

/**
 * @brief 小块内存数据结构
 */
struct MemoryPoolDataType;
typedef struct MemoryPoolSmallBlockDataType {
    //                                  内存数据可用起始位置
    uint8_t*                            start;

    //                                  内存数据可用结束位置
    uint8_t*                            end;

    //                                  下一个内存块位置
    MemoryPoolDataType*                 next;

    //                                  内存块分配失败次数
    uint32_t                            failed;

} MemoryPoolSmallBlock_dt;

using MemoryPoolSmallBlockPtr = MemoryPoolSmallBlock_dt*;

/**
 * @brief 大块内存数据结构
 */
typedef struct MemoryPoolLargeBlockDataType {
    //                                  下一个大块内存块位置
    MemoryPoolLargeBlockDataType*       next;

    //                                  大块内存数据
    void*                               data;

} MemoryPoolLargeBlock_dt;

using MemoryPoolLargeBlockPtr = MemoryPoolLargeBlock_dt*;

/**
 * @brief 自定义内存清理数据结构
 */
using CleanCb = std::function<void(void*)>;

typedef struct MemoryPoolCleanDataType {
    //                                  自定义清理回调函数
    CleanCb                             cb;

    //                                  内存数据
    void*                               data;

    //                                  下一个自定义清理处理数据结构
    MemoryPoolCleanDataType*            next;

} MemoryPoolClean_dt;

using MemoryPoolCleanPtr = MemoryPoolClean_dt*;

/**
 * @brief 自定义关闭文件数据结构
 */
typedef struct MemoryPoolCleanFileDataType {
    //                                  文件描述符
    int                                 fd;

    //                                  文件名称
    uint8_t*                            name;

} MemoryPoolCleanFile_dt;

using MemoryPoolCleanFilePtr = MemoryPoolCleanFile_dt*;

/**
 * @brief 内存池数据结构
 */
typedef struct MemoryPoolDataType {
    //                                  内存数据
    MemoryPoolSmallBlock_dt             data;

    //                                  内存池最大可分配大小
    std::size_t                         maxSize;

    //                                  当前可用小块内存块位置
    MemoryPoolDataType*                 current;

    //                                  当前可用大块内存块位置
    MemoryPoolLargeBlockPtr             large;

    //                                  自定义清理处理数据结构
    MemoryPoolCleanPtr                  clean;

} MemoryPool_dt;

using MemoryPoolPtr = MemoryPool_dt*;

}; // namespace Common