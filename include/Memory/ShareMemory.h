#pragma once
#include <string>
#include <memory>
#include "Utils/Utils.h"
#include "Common/memDef.h"
using namespace Utils;
using namespace Common;

namespace Memory {

/**
 * @brief 共享内存类
 */
class ShareMemory : public Noncopyable, public std::enable_shared_from_this<ShareMemory> {
public:
    using Ptr = std::shared_ptr<ShareMemory>;
    using WkPtr = std::weak_ptr<ShareMemory>;
    using Memory = void*;

public:
    explicit ShareMemory(std::string id, const std::string& name = "", std::size_t size = 4096);
    ~ShareMemory();

public:
     /**
      * @brief  写入数据
      * @return 写入的字节数
      * @param  data 数据指针
      * @param  size 数据大小
      * @param  force 是否强制写入
      */
     std::size_t write(Memory data, std::size_t size, bool force = false) const;

     /**
      * @brief  读取数据
      * @return 读取的字节数
      * @param  data 数据指针
      * @param  size 数据大小
      */
     std::size_t read(Memory data, std::size_t size) const;

private:
    //                  共享内存id
    std::string         m_id;

    //                  是否创建文件
    bool                m_isCreat { false };

    //                  共享内存文件描述符
    int                 m_fd { -1 };

    //                  共享内存名称
    std::string         m_name;

    //                  共享内存数据指针
    ShareMemory_dt      m_shm;
};

}; // namespace Memory