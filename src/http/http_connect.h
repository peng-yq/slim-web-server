//
// Created by pyq on 5/10/24.
//
#pragma once
#ifndef SLIM_WEB_SERVER_HTTP_CONNECT_H
#define SLIM_WEB_SERVER_HTTP_CONNECT_H

#include <atomic>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/uio.h>     
#include <arpa/inet.h> 
#include "http_request.h"
#include "http_response.h"  
#include "../log/log.h"
#include "../buffer/buffer.h"
#include "../sql_connect/sql_connect_raii.h"

// Class representing an HTTP connection, handling both requests and responses.
class HttpConn {
public:
    HttpConn();

    ~HttpConn();

    // Initializes the connection with a socket file descriptor and client address.
    void Init(int sockFd, const sockaddr_in& addr);

    // Closes the connection, cleans up resources, and logs the closure.
    void Close();

    // Returns the file descriptor associated with this connection.
    int GetFd() const;

    // Returns the client's address as a sockaddr_in structure.
    sockaddr_in GetAddr() const;

    // Returns the IP address of the client as a string.
    const char* GetIP() const;

    // Returns the port number of the client.
    int GetPort() const;

    // Returns true if the connection should be kept alive.
    bool IsKeepAlive() const;

    // Reads data from the socket into the read buffer.
    ssize_t Read(int* saveErrno);
    
    // Writes data from the write buffer to the socket using scatter-gather I/O.
    ssize_t Write(int* saveErrno);

    // Calculates the number of bytes that still need to be written to the socket.
    int ToWriteBytes();

    // Processes the request from the read buffer, forms a response, and prepares it for sending.
    bool Process();

    static bool isET;                   // Flag indicating if the socket is using Edge Triggered mode.
    static const char* srcDir;          // Directory path for serving files.
    static std::atomic<int> userCount;  // Counter for the number of active users/connections.
private:
    int fd_;                            // File descriptor for the socket.
    bool isClose_;                      // Flag to check if the connection is closed.
    int iovCnt_;                        // Number of IOV structures being used.
    iovec iov_[2];                      // Array of IOV structures for writev operations.
    sockaddr_in addr_;                  // Client's address.
    Buffer readBuff_;                   // Buffer for reading data from the socket.
    Buffer writeBuff_;                  // Buffer for writing data to the socket.
    HttpRequest httpRequest_;           // HTTP request parser.
    HttpResponse httpResponse_;         // HTTP response generator.
};

#endif //SLIM_WEB_SERVER_HTTP_CONNECT_H
