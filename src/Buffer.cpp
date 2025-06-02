#include <algorithm>
#include <cerrno>
#include <unistd.h>
#include <sys/uio.h>
#include "Utils/Socketop.h"
#include "Utils/Buffer.h"

namespace Utils {

Buffer::Buffer(std::size_t initSize)
    : m_buffer(BUFFER_PREPEND_SIZE + initSize), m_readIdx(BUFFER_PREPEND_SIZE), m_writeIdx(BUFFER_PREPEND_SIZE) {
}

void Buffer::swap(Buffer& other) noexcept {
    m_buffer.swap(other.m_buffer);
    std::swap(m_readIdx, other.m_readIdx);
    std::swap(m_writeIdx, other.m_writeIdx);
}

void Buffer::extend(std::size_t len) {
    std::size_t readable = this->readableBytes();

    if (this->writableBytes() + this->prependableBytes() < len + BUFFER_PREPEND_SIZE) {
        // 拷贝缓冲区原有数据
        std::vector<uint8_t> newBuffer(m_buffer.size() + len);
        std::copy_n(m_buffer.begin() + static_cast<int>(m_readIdx), readable, newBuffer.begin() + BUFFER_PREPEND_SIZE);

        // 重置缓冲区
        m_buffer = std::move(newBuffer);

    }
    else {
        // 将可读数据拷贝到缓冲区前端
        std::copy_n(m_buffer.begin() + static_cast<int>(m_readIdx), readable, m_buffer.begin() + BUFFER_PREPEND_SIZE);
    }

    m_readIdx = BUFFER_PREPEND_SIZE;
    m_writeIdx = m_readIdx + readable;
}

void Buffer::shrink(std::size_t len) {
    Buffer other(BUFFER_PREPEND_SIZE + this->readableBytes() + len);
    other.write(this->readBegin(), this->readableBytes());
    this->swap(other);
}

const uint8_t* Buffer::peek(std::size_t& size) const {
    size = this->readableBytes();
    return this->readBegin();
}

void Buffer::read(std::vector<uint8_t>& buffer, std::size_t& len) {
    len = this->readableBytes();
    if (buffer.size() < len) {
        buffer.resize(len);
    }

    std::copy_n(this->readBegin(), len, buffer.data());
    this->moveReadStartPos(len);
}

void Buffer::write(const uint8_t* data, std::size_t len) {
    this->ensureWritableBytes(len);
    std::copy_n(data, len, this->writeBegin());
    this->moveWriteStartPos(len);
}

bool Buffer::readFixSize(std::vector<uint8_t>& buffer, std::size_t len) {
    std::size_t readable = this->readableBytes();
    if (readable < len) {
        return false;
    }

    if (buffer.size() < len) {
        buffer.resize(len);
    }

    std::copy_n(this->readBegin(), len, buffer.data());
    this->moveReadStartPos(len);
    return true;
}

ssize_t Buffer::readFd(int fd, int& err) {
    iovec vec[2] = {};

    // 设置主缓冲区
    const std::size_t writable = this->writableBytes();
    vec[0].iov_base = this->writeBegin();
    vec[0].iov_len = writable;

    // 设置备用缓冲区
    uint8_t backBuf[65536] = {0};
    constexpr std::size_t backBufSize = sizeof(backBuf);
    vec[1].iov_base = backBuf;
    vec[1].iov_len = backBufSize;

    // 读取数据
    const int iovcnt = (writable < backBufSize) ? 2 : 1;
    ssize_t len = Socketop::Readv(fd, vec, iovcnt);
    if (-1 == len) {
        err = errno;
        return len;
    }
    else if (len <= writable) {
        this->moveWriteStartPos(len);
    }
    else {
        this->moveWriteStartPos(writable);
        this->write(backBuf, len - writable);
    }

    return len;
}

ssize_t Buffer::writeFd(int fd, int& err) {
    ssize_t len = Socketop::Write(fd, this->readBegin(), this->readableBytes());
    if (-1 == len) {
       err = errno;
       return len;
    }

    this->moveReadStartPos(len);
    return len;
}

} // namespace Utils