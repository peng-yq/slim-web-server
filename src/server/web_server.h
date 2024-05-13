//
// Created by pyq on 5/13/24.
//
#pragma once
#ifndef SLIM_WEB_SERVER_WEB_SERVER_H
#define SLIM_WEB_SERVER_WEB_SERVER_H

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <cassert>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "epoller.h"
#include "../log/log.h"
#include "../timer/timer.h"
#include "../sql_connect/sql_connect.h"
#include "../sql_connect/sql_connect_raii.h"
#include "../thread_pool/thread_pool.h"
#include "../http/http_connect.h"

// WebServer integrates logging, database connection pooling, thread pooling, and HTTP processing
// to create a high-performance, epoll-based (Reactor) web server.
class WebServer {
public:
    WebServer(
        int port, int trigMode, int timeoutMs, bool optLinger,
        int sqlPort, const char* sqlUser, const char* sqlPwd,
        const char* dbName, int sqlConnPoolNum, int threadNum,
        bool enableLog, int logLevel, int logQueSize);
    
    ~WebServer();

    // Starts the server,
    void Start();

private:
    int port_;                    // Port number on which the server will listen for incoming connections
    bool openLinger_;             // Flag to specify if the SO_LINGER option is enabled for sockets
    int timeoutMs_;               // Timeout in milliseconds for client connections; used for connection timing out
    bool isClose_;                // Flag to indicate if the server should shut down
    int listenFd_;                // File descriptor for the listening socket
    char* srcDir_;                // Directory path that holds the server's resource files
    uint32_t listenEvent_;        // Event types configured for the listening socket (e.g., EPOLLIN, EPOLLET)
    uint32_t connEvent_;          // Event types configured for client connection sockets

    // Unique pointers to manage resources automatically
    std::unique_ptr<Timer> timer_;              // Pointer to the Timer object, used for managing connection timeouts
    std::unique_ptr<ThreadPool> threadPool_;    // Pointer to the ThreadPool object, used for managing worker threads
    std::unique_ptr<Epoller> epoller_;          // Pointer to the Epoller object, used for handling epoll-based event notification
    
    std::unordered_map<int, HttpConn> users_;   // Map of file descriptors to HTTP connection handlers
    static const int MAX_FD = 65536;            // Maximum number of file descriptors that the server can handle

    // Initializes the listening socket
    bool InitListenSocket_();

    // Initializes event modes based on the configuration
    void InitEventMode_(int trigMode);

    // Adds a new client connection
    void AddClient_(int fd, sockaddr_in addr);

    // Handles new connections on the listening socket
    void DealListen_();

    // Handles read events for a given client
    void DealRead_(HttpConn* client);

    // Handles write events for a given client
    void DealWrite_(HttpConn* client);

    // Sends an error message to the specified file descriptor
    void SendError_(int fd, const char* info);

    // Extends the timer for a client to prevent timeout
    void ExtentTime_(HttpConn* client);

    // Closes a client connection
    void CloseConn_(HttpConn* client);

    // Processes read events, reading data from the client
    void OnRead_(HttpConn* client);

    // Processes write events, sending data to the client
    void OnWrite_(HttpConn* client);

    // Main processing function for handling HTTP requests and responses
    void OnProcess_(HttpConn* client);

    // Sets a file descriptor to non-blocking mode
    static int SetFdNonBlock(int fd);
};

#endif //SLIM_WEB_SERVER_WEB_SERVER_H
