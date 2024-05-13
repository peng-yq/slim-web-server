//
// Created by pyq on 5/13/24.
//
#include "epoller.h"

Epoller::Epoller(int maxEvent) : epollFd_(epoll_create1(0)), events_(maxEvent) {
    assert(epollFd_ >= 0 && events_.size() > 0);
}

Epoller::~Epoller() {
    close(epollFd_);
}

bool Epoller::AddFd(int fd, uint32_t events) {
    if (fd < 0) {
        return false;
    }
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev) == 0;
}

bool Epoller::ModFd(int fd, uint32_t events) {
    if (fd < 0) {
        return false;
    }
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev) == 0;
}

bool Epoller::DelFd(int fd) {
    if (fd < 0) {
        return false;
    }
    epoll_event ev = {0};
    return epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &ev) == 0;
}

int Epoller::Wait(int timeoutMs) {
    // default value of timeoutMs is -1
    // which means Wait indefinitely until an event occurs

    // Check all file descriptors registered to epollFd_. 
    // If events occur on multiple file descriptors, 
    // epoll_wait will fill the information of these events into the memory area starting from &events_[0], 
    // up to events_.size() epoll_event structures.
    return epoll_wait(epollFd_, &events_[0], static_cast<int>(events_.size()), timeoutMs);
}

int Epoller::GetEventFd(size_t i) const {
    assert(i >= 0 && i < events_.size());
    return events_[i].data.fd;
}

uint32_t Epoller::GetEvents(size_t i) const {
    assert(i >= 0 && i < events_.size());
    return events_[i].events;
}