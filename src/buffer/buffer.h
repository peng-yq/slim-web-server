//
// Created by peng yongqiang on 2024/5/3.
//
#pragma once
#ifndef SLIM_WEB_SERVER_BUFFER_H
#define SLIM_WEB_SERVER_BUFFER_H

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/uio.h>
#include <vector>
#include <mutex>
#include <cassert>

// A thread-safe buffer class for managing a dynamic array of bytes.
class Buffer {
public:
    // Constructor that initializes the buffer with a given size (default is 1024 bytes).
    explicit Buffer(int bufferSize = 1024);

    // Default destructor.
    ~Buffer() = default;

    // Returns the number of writable bytes remaining in the buffer.
    size_t GetWritableBytes() const;

    // Returns the number of readable bytes available in the buffer.
    size_t GetReadableBytes() const;

    // Returns the number of bytes that can be prepended to the buffer.
    size_t GetPrependableBytes() const;

    // Returns a pointer to the beginning of the readable data.
    const char* BeginRead() const;

    // Ensures that there is enough space to write at least 'len' bytes.
    void EnsureWritable(size_t len);

    // Advances the write pointer by 'len' bytes.
    void AdvanceWritePointer(size_t len);

    // Advances the read pointer by 'len' bytes.
    void AdvanceReadPointer(size_t len);

    // Returns a constant pointer to the beginning of the writable area.
    const char* BeginWrite() const;

    // Returns a modifiable pointer to the beginning of the writable area.
    char* BeginWrite();

    // Clears the buffer, resetting both read and write positions.
    void RetrieveAll();

    // Retrieves all readable data as a std::string and clears the buffer.
    std::string RetrieveAllAsString();

    // Appends a std::string to the buffer.
    void Append(const std::string& str);

    // Appends raw data to the buffer.
    void Append(const char* data, size_t len);

    // Appends raw data from a void pointer to the buffer.
    void Append(const void* data, size_t len);

    // Appends data from another Buffer instance.
    void Append(const Buffer& buff);

    // Reads data from a file descriptor into the buffer.
    ssize_t ReadFromFd(int fd, int* error);

    // Writes data from the buffer to a file descriptor.
    ssize_t WriteToFd(int fd, int* error);

private:
    // Returns a non-const pointer to the beginning of the buffer.
    char* BeginPtr_();

    // Returns a const pointer to the beginning of the buffer.
    const char* BeginPtr_() const;

    // Resizes or rearranges the buffer to make space for at least 'len' bytes.
    void MakeSpace_(size_t len);

    std::vector<char> buffer_;  // The underlying buffer storage.
    std::size_t readPos_;       // Current read position in the buffer.
    std::size_t writePos_;      // Current write position in the buffer.
    mutable std::mutex mutex_;  // Mutex for protecting access to the buffer.
};

#endif // SLIM_WEB_SERVER_BUFFER_H
