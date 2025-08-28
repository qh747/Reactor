#pragma once
#include <cstdint>
#include <functional>
#include <semaphore.h>

namespace Common {

// 内存池申请首字节对齐大小
constexpr static uint32_t MEMORY_POOL_ALIGNMENT = 16;

// 内存申请首字节对齐大小
constexpr static uint32_t MEMORY_ALIGNMENT = sizeof(unsigned long);

// 共享内存默认大小
constexpr static std::size_t SHARE_MEMORY_DFT_SIZE = 4096;

/**
 * @brief 小块内存数据结构
 */
struct MemoryPoolDataType;
typedef struct MemoryPoolSmallBlockDataType {
    //                                  内存数据可用起始位置
    uint8_t*                            start { nullptr };

    //                                  内存数据可用结束位置
    uint8_t*                            end { nullptr };

    //                                  下一个内存块位置
    MemoryPoolDataType*                 next { nullptr };

    //                                  内存块分配失败次数
    uint32_t                            failed { 0 };

} MemoryPoolSmallBlock_dt;

using MemoryPoolSmallBlockPtr = MemoryPoolSmallBlock_dt*;

/**
 * @brief 大块内存数据结构
 */
typedef struct MemoryPoolLargeBlockDataType {
    //                                  下一个大块内存块位置
    MemoryPoolLargeBlockDataType*       next { nullptr };

    //                                  大块内存数据
    void*                               data { nullptr };

} MemoryPoolLargeBlock_dt;

using MemoryPoolLargeBlockPtr = MemoryPoolLargeBlock_dt*;

/**
 * @brief 自定义内存清理数据结构
 */
using CleanCb = std::function<void(void*)>;

typedef struct MemoryPoolCleanDataType {
    //                                  自定义清理回调函数
    CleanCb                             cb { nullptr };

    //                                  内存数据
    void*                               data { nullptr };

    //                                  下一个自定义清理处理数据结构
    MemoryPoolCleanDataType*            next { nullptr };

} MemoryPoolClean_dt;

using MemoryPoolCleanPtr = MemoryPoolClean_dt*;

/**
 * @brief 自定义关闭文件数据结构
 */
typedef struct MemoryPoolCleanFileDataType {
    //                                  文件描述符
    int                                 fd { -1 };

    //                                  文件名称
    uint8_t*                            name { nullptr };

} MemoryPoolCleanFile_dt;

using MemoryPoolCleanFilePtr = MemoryPoolCleanFile_dt*;

/**
 * @brief 内存池数据结构
 */
typedef struct MemoryPoolDataType {
    //                                  内存数据
    MemoryPoolSmallBlock_dt             data;

    //                                  内存池最大可分配大小
    std::size_t                         maxSize { 0 };

    //                                  当前可用小块内存块位置
    MemoryPoolDataType*                 current { nullptr };

    //                                  当前可用大块内存块位置
    MemoryPoolLargeBlockPtr             large { nullptr };

    //                                  自定义清理处理数据结构
    MemoryPoolCleanPtr                  clean { nullptr };

} MemoryPool_dt;

using MemoryPoolPtr = MemoryPool_dt*;

/**
 * @brief 共享内存名称数据结构
 */
typedef struct ShareMemoryNameDataType {
    //                                  字符串数据
    const uint8_t*                      data { nullptr };

    //                                  字符串长度
    std::size_t                         size { 0 };

} ShareMemoryName_dt;

/**
 * @brief 共享内存数据结构
 */
typedef struct ShareMemoryDataType {
    //                                  共享内存地址
    uint8_t*                            data { nullptr };

    //                                  共享内存大小
    std::size_t                         size { 0 };

    //                                  共享内存是否存在
    bool                                exist { false };

    //                                  共享内存名称
    ShareMemoryName_dt                  name;

} ShareMemory_dt;

using ShareMemoryPtr = ShareMemory_dt*;

/**
 * @note  共享内存空间使用
 * @brief 共享内存互斥锁数据结构
 */
typedef struct ShareMemoryLockDataType {
    //                                 标识是否被占用，0 - 为占用，pid - 已被占用进程id
    uint32_t                           lock { 0 };

    //                                 阻塞等待进程数量
    uint32_t                           wait { 0 };

} ShareMemoryLock_dt;

/**
 * @note  进程独占
 * @brief 共享内存互斥锁数据结构
 */
typedef struct ShareMemoryPrivateLockDataType {
    //                                 原子锁，0 - 未占用，进程pid - 占用
    uint32_t*                          lock { nullptr };

    //                                 阻塞等待进程数量
    uint32_t*                          wait { nullptr };

    //                                 信号量创建成功标识
    uint32_t                           semaphore { 0 };

    //                                 信号量
    sem_t                              sem { 0 };

    //                                 自旋锁
    uint32_t                           spin { 0 };

} ShareMemoryPrivateLock_dt;

}; // namespace Common