//
// Created by pyq on 2024/5/3.
//

#include "buffer.h"

// Constructor: Initializes the buffer to the specified size and resets positions.
Buffer::Buffer(int bufferSize) : buffer_(bufferSize), readPos_(0), writePos_(0) {}

// Returns the number of writable bytes in the buffer.
size_t Buffer::GetWritableBytes() const {
    std::lock_guard<std::mutex> locker(mutex_);
    return buffer_.size() - writePos_;
}

// Returns the number of readable bytes in the buffer.
size_t Buffer::GetReadableBytes() const {
    std::lock_guard<std::mutex> locker(mutex_);
    return writePos_ - readPos_;
}

// Returns the number of bytes available before the read position.
size_t Buffer::GetPrependableBytes() const {
    std::lock_guard<std::mutex> locker(mutex_);
    return readPos_;
}

// Returns a pointer to the start of readable data.
const char* Buffer::BeginRead() const {
    std::lock_guard<std::mutex> locker(mutex_);
    return BeginPtr_() + readPos_;
}

// Ensures there is enough space to write 'len' bytes, resizing if necessary.
void Buffer::EnsureWritable(size_t len) {
    std::lock_guard<std::mutex> locker(mutex_);
    if (GetWritableBytes() < len) {
        MakeSpace_(len);
    }
    assert(GetWritableBytes() >= len);
}

// Advances the write pointer forward by 'len' bytes.
void Buffer::AdvanceWritePointer(size_t len) {
    std::lock_guard<std::mutex> locker(mutex_);
    writePos_ += len;
}

// Advances the read pointer forward by 'len' bytes.
void Buffer::AdvanceReadPointer(size_t len) {
    std::lock_guard<std::mutex> locker(mutex_);
    readPos_ += len;
}

// Returns a constant pointer to the start of writable data.
const char* Buffer::BeginWriteConst() const {
    std::lock_guard<std::mutex> locker(mutex_);
    return BeginPtr_() + writePos_;
}

// Returns a modifiable pointer to the start of writable data.
char* Buffer::BeginWrite() {
    std::lock_guard<std::mutex> locker(mutex_);
    return BeginPtr_() + writePos_;
}

// Clears all data in the buffer, resetting positions.
void Buffer::RetrieveAll() {
    std::lock_guard<std::mutex> locker(mutex_);
    std::fill(buffer_.begin(), buffer_.end(), 0);
    readPos_ = 0;
    writePos_ = 0;
}

// Retrieves and returns all readable data as a string, then clears the buffer.
std::string Buffer::RetrieveAllAsString() {
    std::lock_guard<std::mutex> locker(mutex_);
    std::string str(BeginPtr_() + readPos_, GetReadableBytes());
    RetrieveAll();
    return str;
}

// Appends a string to the buffer.
void Buffer::Append(const std::string& str) {
    Append(str.data(), str.size());
}

// Appends raw data to the buffer.
void Buffer::Append(const char* data, size_t len) {
    assert(data);
    EnsureWritable(len);
    std::copy(data, data + len, BeginWrite());
    AdvanceWritePointer(len);
}

// Appends raw data from a void pointer to the buffer.
void Buffer::Append(const void* data, size_t len) {
    assert(data);
    Append(static_cast<const char*>(data), len);
}

// Appends data from another buffer.
void Buffer::Append(const Buffer& buff) {
    Append(buff.BeginRead(), buff.GetReadableBytes());
}

// Reads data from a file descriptor into the buffer, handling overflow.
ssize_t Buffer::ReadFromFd(int fd, int* error) {
    char tempBuffer[65536];
    struct iovec iov[2];
    const size_t writable = GetWritableBytes();

    iov[0].iov_base = BeginWrite();
    iov[0].iov_len = writable;
    iov[1].iov_base = tempBuffer;
    iov[1].iov_len = sizeof(tempBuffer);

    const ssize_t len = readv(fd, iov, 2);
    if (len < 0) {
        *error = errno;
    } else {
        if (static_cast<size_t>(len) <= writable) {
            AdvanceWritePointer(len);
        } else {
            AdvanceWritePointer(writable);
            Append(tempBuffer, len - writable);
        }
    }
    return len;
}

// Writes data from the buffer to a file descriptor, advancing the read pointer.
ssize_t Buffer::WriteToFd(int fd, int* error) {
    ssize_t len = write(fd, BeginRead(), GetReadableBytes());
    if (len < 0) {
        *error = errno;
    } else {
        AdvanceReadPointer(len);
    }
    return len;
}

// Returns a non-const pointer to the beginning of the buffer.
char* Buffer::BeginPtr_() {
    return &*buffer_.begin();
}

// Returns a const pointer to the beginning of the buffer.
const char* Buffer::BeginPtr_() const {
    return &*buffer_.begin();
}

// Resizes or rearranges the buffer to make space for new data.
void Buffer::MakeSpace_(size_t len) {
    if (GetWritableBytes() + GetPrependableBytes() < len) {
        buffer_.resize(writePos_ + len + 1);
    } else {
        size_t readable = GetReadableBytes();
        std::copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_());
        readPos_ = 0;
        writePos_ = readable;
    }
}
