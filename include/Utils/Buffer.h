#pragma once
#include <mutex>
#include <vector>
#include <memory>
#include "Common/ConfigDef.h"
using namespace Common;

namespace Utils {

/**
 * @brief 缓冲区类
 * @note  缓冲区结构：
 * +-------------------+------------------+------------------+
 * | prependable bytes |  readable bytes  |  writable bytes  |
 * |                   |     (CONTENT)    |                  |
 * +-------------------+------------------+------------------+
 * 0        <=     m_readIdx    <=    m_writeIdx   <=    m_buffer.size()
 */
class Buffer {
public:
    using Ptr = std::shared_ptr<Buffer>;
    using WkPtr = std::weak_ptr<Buffer>;

public:
    explicit Buffer(std::size_t initSize = BUFFER_INIT_SIZE);
    ~Buffer() = default;

public:
    /**
     * @brief  buffer交换
     * @param  other 交换对象
     */
    void swap(Buffer& other) noexcept;

    /**
     * @brief 拓展缓冲区大小
     * @param len 调整后的大小
     */
    void extend(std::size_t len);

    /**
     * @brief 收缩缓冲区大小
     * @param len 调整后的大小
     */
    void shrink(std::size_t len);

    /**
     * @brief  获取缓冲区中可读数据起始地址
     * @return 可读数据起始地址
     * @param  size 可读数据长度
     */
    const uint8_t* peek(std::size_t& size) const;

    /**
     * @brief 从缓冲区中读取数据
     * @param buffer 存储读取数据的缓冲区
     * @param len 存储读取数据的长度
     */
    void read(std::vector<uint8_t>& buffer, std::size_t& len);

    /**
     * @brief  将数据写入缓冲区
     * @param  data 数据指针
     * @param  len  数据长度
     */
    void write(const uint8_t* data, std::size_t len);

    /**
     * @brief  从缓冲区中读取固定长度数据
     * @return 读取结果
     * @param  buffer 存储读取数据的缓冲区
     * @param  len 固定长度大小
     */
    bool readFixSize(std::vector<uint8_t>& buffer, std::size_t len);

    /**
     * @brief  读取网络数据
     * @return 读取数据长度
     * @param  fd 网络文件描述符
     * @param  err 错误码
     */
    ssize_t readFd(int fd, int& err);

    /**
     * @brief  写入网络数据
     * @return 写入数据长度
     * @param  fd 网络文件描述符
     * @param  err 错误码
     */
    ssize_t writeFd(int fd, int& err);

public:
    /**
     * @brief  获取缓冲区可读数据起始地址
     * @return 可读数据起始地址
     */
    inline const uint8_t* readBegin() const {
        return begin() + m_readIdx;
    }

    /**
     * @note   内存拷贝前检查可用空间是否足够：std::size_t readableBytes()
     * @brief  获取缓冲区可读数据起始地址
     * @return 可读数据起始地址
     */
    inline uint8_t* readBegin() {
        return begin() + m_readIdx;
    }

    /**
     * @brief  获取缓冲区可写数据起始地址
     * @return 可写数据起始地址
     */
    inline const uint8_t* writeBegin() const {
        return begin() + m_writeIdx;
    }

    /**
     * @note   内存拷贝前检查可用空间是否足够：std::size_t writableBytes()
     * @brief  获取缓冲区可写数据起始地址
     * @return 可写数据起始地址
     */
    inline uint8_t* writeBegin() {
        return begin() + m_writeIdx;
    }

public:
    /**
     * @brief  获取可读数据大小
     * @return 可读数据大小
     */
     inline std::size_t readableBytes() const {
        return m_writeIdx - m_readIdx;
     }

     /**
     * @brief  获取可写数据大小
     * @return 可写数据大小
     */
    inline std::size_t writableBytes() const {
        return m_buffer.size() - m_writeIdx;
    }

    /**
     * @brief  获取缓冲区中前置数据大小
     * @return 前置数据大小
     */
    inline std::size_t prependableBytes() const {
        return m_readIdx;
    }

    /**
     * @brief 确保数据写入空间足够
     * @param len 写入数据长度
     */
    inline void ensureWritableBytes(std::size_t len) {
        if (this->writableBytes() < len) {
            this->extend(len);
        }
    }

public:
    /**
     * @brief  获取缓冲区起始地址
     * @return 缓冲区起始地址
     */
    inline const uint8_t* begin() const {
        return m_buffer.data();
    }

    /**
     * @brief  获取缓冲区起始地址
     * @return 缓冲区起始地址
     */
    inline uint8_t* begin() {
        return m_buffer.data();
    }

    /**
     * @brief 移动读缓冲区起始位置
     * @param len 移动长度
     */
    inline void moveReadStartPos(std::size_t len) {
        if (len >= readableBytes()) {
            m_readIdx = m_writeIdx = BUFFER_PREPEND_SIZE;
        }
        else {
            m_readIdx += len;
        }
    }

    /**
     * @brief 移动写缓冲区中起始位置
     * @param len 移动长度
     */
    inline void moveWriteStartPos(std::size_t len) {
        if (len >= writableBytes()) {
            m_writeIdx += this->writableBytes();
        }
        else {
            m_writeIdx += len;
        }
    }

private:
    // 缓冲区数据存储容器
    std::vector<uint8_t> m_buffer;

    // 缓冲区可读数据的起始位置索引
    std::size_t m_readIdx;

    // 缓冲区可写数据的起始位置索引
    std::size_t m_writeIdx;
};

}; // namespace Utils