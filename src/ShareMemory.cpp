#include <cstdio>
#include <cstring>
#include <utility>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "Utils/Logger.h"
#include "Memory/ShareMemory.h"

namespace Memory {

ShareMemory::ShareMemory(std::string id, const std::string& name, std::size_t size) : m_id(std::move(id)), m_name(name) {
    // 共享内存空间最小不能小于SHARE_MEMORY_DFT_SIZE
    size = std::min(size, SHARE_MEMORY_DFT_SIZE);

    // name非空时，通过文件关联内存映射
    bool isOpenFile = !name.empty();
    if (isOpenFile) {
        struct stat buffer = {0};
        if (0 != stat(name.c_str(), &buffer)) {
            // 文件不存在则创建并打开
            m_fd = open(name.c_str(), O_RDWR | O_CREAT, 0666);
            m_isCreat = true;

            // 保证文件有足够空间
            ftruncate(m_fd, static_cast<off_t>(size));
        }
        else {
            // 文件存在则打开
            m_fd = open(name.c_str(), O_RDWR, 0666);
        }

        if (m_fd < 0) {
            LOG_ERROR << "Share memory construct error. open file failed. id: " << m_id << " name: " << name;
            throw std::runtime_error("open file failed.");
        }
    }

    // 映射共享内存
    Memory shm = nullptr;
    if (isOpenFile) {
        shm = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
    }
    else {
        shm = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, m_fd, 0);
    }

    if (MAP_FAILED == shm) {
        if (isOpenFile) {
            close(m_fd);
        }

        LOG_ERROR << "Share memory construct error. mmap failed. id: " << m_id << " name: " << name;
        throw std::runtime_error("mmap failed.");
    }

    // 保存共享内存信息
    m_shm.data  = static_cast<uint8_t*>(shm);
    m_shm.size  = size;
    m_shm.exist = true;
    m_shm.name.data = reinterpret_cast<const uint8_t*>(m_name.c_str());
    m_shm.name.size = m_name.size();

    LOG_DEBUG << "Share memory construct success. id: " << m_id << " name: " << name;
}

ShareMemory::~ShareMemory() {
    // 解除共享内存映射
    if (m_shm.exist) {
        munmap(m_shm.data, m_shm.size);
        m_shm.exist = false;
    }

    // 关闭文件句柄
    if (-1 != m_fd) {
        close(m_fd);

        if (m_isCreat) {
            std::remove(m_name.c_str());
        }
    }

    LOG_DEBUG << "Share memory destruct success. id: " << m_id << " name: " << m_name;
}

std::size_t ShareMemory::write(Memory data, std::size_t size, bool force) const {
    if (!m_shm.exist) {
        LOG_ERROR << "Share memory write error. not mmap or mmap failed. id: " << m_id << " name: " << m_name;
        return 0;
    }

    // 判断写入长度是否超过共享内存映射空间长度
    if (size > m_shm.size) {
        if (force) {
            size = m_shm.size;
            LOG_WARN << "Share memory write warning. size is too large. id: " << m_id << " name: " << m_name << " write size: " << size;
        }
        else {
            LOG_ERROR << "Share memory write error. size is too large. id: " << m_id << " name: " << m_name;
            return 0;
        }
    }

    // 写入数据
    memcpy(m_shm.data, data, size);

    // 确保同步到磁盘
    if (-1 != m_fd) {
        msync(m_shm.data, m_shm.size, MS_SYNC);
    }

    LOG_DEBUG << "Share memory write success. id: " << m_id << " name: " << m_name << " write size: " << size;
    return size;
}

std::size_t ShareMemory::read(Memory data, std::size_t size) const {
    if (!m_shm.exist) {
        LOG_ERROR << "Share memory read error. not mmap or mmap failed. id: " << m_id << " name: " << m_name;
        return 0;
    }

    size = std::min(size, m_shm.size);
    memcpy(data, m_shm.data, size);

    LOG_DEBUG << "Share memory read success. id: " << m_id << " name: " << m_name << " read size: " << size;
    return size;
}

}; // namespace Memory
