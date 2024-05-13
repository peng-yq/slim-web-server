//
// Created by pyq on 5/13/24.
//
#pragma once
#ifndef SLIM_WEB_SERVER_EPOLLER_H
#define SLIM_WEB_SERVER_EPOLLER_H

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <cassert>
#include <vector>
#include <sys/epoll.h>

// Epoller class encapsulates epoll-based event handling.
class Epoller {
public:
    // Initializes an epoll instance and pre-allocates space for events.
    explicit Epoller(int maxEvent = 1024);

    ~Epoller();

    // Adds a file descriptor to the epoll instance with specified events.
    bool AddFd(int fd, uint32_t events);

    // Modifies the event mask for the specified file descriptor in the epoll instance.
    bool ModFd(int fd, uint32_t events);

    // Removes a file descriptor from the epoll instance.
    bool DelFd(int fd);

    // Waits for events on the epoll file descriptor, with an optional timeout.
    int Wait(int timeoutMs = -1);

    // Retrieves the file descriptor associated with the ith event from the last epoll_wait call.
    int GetEventFd(size_t i) const;

    // Retrieves the events associated with the ith event from the last epoll_wait call.
    uint32_t GetEvents(size_t i) const;

private:
    int epollFd_;                       // Epoll file descriptor.
    std::vector<epoll_event> events_;   // Buffer where events are returned.
};

#endif //SLIM_WEB_SERVER_EPOLLER_H
