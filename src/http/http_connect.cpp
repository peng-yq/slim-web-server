//
// Created by pyq on 5/10/24.
//
#include "http_connect.h"

// *srcDir can't be changed, value of srcDir can be changed
const char* HttpConn::srcDir;

std::atomic<int> HttpConn::userCount = 0;

bool HttpConn::isET;

HttpConn::HttpConn() : fd_(-1), addr_({0}), isClose_(true) {}

HttpConn::~HttpConn() {
    Close();
}

void HttpConn::Init(int sockFd, const sockaddr_in& addr) {
    assert(sockFd > 0);
    userCount++;
    addr_ = addr;
    fd_ = sockFd;
    writeBuff_.RetrieveAll();
    readBuff_.RetrieveAll();
    isClose_ = false;
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
}

void HttpConn::Close() {
    httpResponse_.UnmapFile();
    if (isClose_ == false) {
        isClose_ = true;
        userCount--;
        close(fd_);
        LOG_INFO("Client[%d](%s:%d) quit, userCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
    }
}

int HttpConn::GetFd() const {
    return fd_;
}

sockaddr_in HttpConn::GetAddr() const {
    return addr_;
}

const char* HttpConn::GetIP() const {
    return inet_ntoa(addr_.sin_addr);
}

int HttpConn::GetPort() const {
    return addr_.sin_port;
}

bool HttpConn::IsKeepAlive() const {
    return httpRequest_.IsKeepAlive();
}

ssize_t HttpConn::Read(int* saveErrno) {
    // read data from fd_ into readBuff_
    ssize_t len = -1;
    // do-while loop guarantees that whether it is ET or LT, 
    // a read will be performed.
    do {
        len = readBuff_.ReadFromFd(fd_, saveErrno);
        // ET mode will read data as much as possible until 
        // there is no data or an error occurs.
        if (len <= 0) {
            break;
        }
    } while (isET);
    return len;
}

ssize_t HttpConn::Write(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = writev(fd_, iov_, iovCnt_);
        if (len <= 0) {
            *saveErrno = errno;
            break;
        }
        if (iov_[0].iov_len + iov_[1].iov_len == 0) {
            // all of the data in writeBuff_ has been written
            break;
        } else if (static_cast<size_t>(len) > iov_[0].iov_len) {
            // iov_[1] has been written
            iov_[1].iov_base = (uint8_t*) iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if (iov_[0].iov_len > 0) {
                writeBuff_.RetrieveAll();
                iov_[0].iov_len = 0;
            } 
        } else {
            // only use iov_[1]
            iov_[0].iov_base = (uint8_t*) iov_[0].iov_base + len;
            iov_[0].iov_len -= len;
            writeBuff_.AdvanceReadPointer(len);
        }
    } while (isET || ToWriteBytes() > 10240); // ET mode ordata to be written is large (> 10240B), 
    // write data as much as possible to reduce system calls.
    return len;
}

// calculate and return how many bytes currently need to be written
int HttpConn::ToWriteBytes() {
    return iov_[0].iov_len + iov_[1].iov_len;
}

bool HttpConn::Process() {
    httpRequest_.Init();
    if (readBuff_.GetReadableBytes() <= 0) {
        return false;
    } else if (httpRequest_.ParseHttpRequest(readBuff_)) {
        LOG_DEBUG("HttpRequest Path: %s", httpRequest_.Path().c_str());
        httpResponse_.Init(srcDir, httpRequest_.Path(), httpRequest_.IsKeepAlive(), 200);
    } else {
        httpResponse_.Init(srcDir, httpRequest_.Path(), false, 400);
    }

    // iov_[0] stores the data of http response except response body
    httpResponse_.MakeResponse(writeBuff_);
    iov_[0].iov_base = const_cast<char*> (writeBuff_.BeginRead());
    iov_[0].iov_len = writeBuff_.GetReadableBytes();
    iovCnt_ = 1;

    // iov_[1] stores the data of response body
    if (httpResponse_.GetFile() && httpResponse_.GetFileLen() > 0) {
        iov_[1].iov_base = httpResponse_.GetFile();
        iov_[1].iov_len = httpResponse_.GetFileLen();
        iovCnt_ = 2;
    }
    LOG_DEBUG("File Size: %d, %d to %d", httpResponse_.GetFileLen(), iovCnt_, ToWriteBytes());
    return true;
}