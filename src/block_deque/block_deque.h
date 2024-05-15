//
// Created by pyq on 2024/5/5.
//
#pragma once
#ifndef SLIM_WEB_SERVER_BLOCK_DEQUE_H
#define SLIM_WEB_SERVER_BLOCK_DEQUE_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include <chrono>
#include <cassert>

// A thread-safe, blocking deque implementation that allows for synchronized access from multiple threads.
template<class T>
class BlockDeque {
public:
    // Constructor that initializes the deque with a specific capacity (default is 1000).
    explicit BlockDeque(size_t capacity = 1000);

    // Destructor that cleans up by closing and clearing the deque.
    ~BlockDeque();

    // Notifies one waiting consumer, potentially unblocking a thread waiting to pop from the deque.
    void flush();

    // Clears all elements from the deque, ensuring mutual exclusion during the operation.
    void clear();

    // Closes the deque, clears all elements, and notifies all waiting producers and consumers.
    void close();

    // Returns true if the deque is empty, false otherwise.
    bool empty();

    // Returns true if the deque is full (i.e., size has reached capacity), false otherwise.
    bool full();

    // Returns the current number of elements in the deque.
    size_t size();

    // Returns the maximum number of elements the deque can hold.
    size_t capacity();

    // Returns the front element of the deque. Throws runtime_error if the deque is empty.
    T front();

    // Returns the back element of the deque. Throws runtime_error if the deque is empty.
    T back();

    // Adds an element to the back of the deque. Waits if the deque is full until space becomes available.
    void push_back(const T &item);

    // Adds an element to the front of the deque. Waits if the deque is full until space becomes available.
    void push_front(const T &item);

    // Removes and returns the front element of the deque. Blocks if the deque is empty until an element is available.
    bool pop_front(T &item);

    // Removes and returns the front element of the deque with a timeout. Returns false if timed out or the deque was closed.
    bool pop_front(T &item, int timeout);

private:
    std::deque<T> deque_;             // Internal deque used for storage.
    size_t capacity_;                 // Maximum number of items the deque can hold.
    std::mutex mutex_;                // Mutex to protect access to the internal deque.
    bool isClose_;                    // Indicates whether the deque has been closed.
    std::condition_variable consumer_; // Condition variable to notify consumers.
    std::condition_variable producer_; // Condition variable to notify producers.
};

template<class T>
BlockDeque<T>::BlockDeque(size_t capacity) : capacity_(capacity), isClose_(false) {
    assert(capacity > 0);
}

template<class T>
BlockDeque<T>::~BlockDeque() {
    close();
}

template<class T>
void BlockDeque<T>::flush() {
    consumer_.notify_one();
}

template<class T>
void BlockDeque<T>::clear() {
    std::lock_guard<std::mutex> locker(mutex_);
    deque_.clear();
}

template<class T>
void BlockDeque<T>::close() {
    {
        std::lock_guard<std::mutex> locker(mutex_);
        deque_.clear();
        isClose_ = true;
    }
    producer_.notify_all();
    consumer_.notify_all();
}

template<class T>
bool BlockDeque<T>::empty() {
    std::lock_guard<std::mutex> locker(mutex_);
    return deque_.empty();
}

template<class T>
bool BlockDeque<T>::full() {
    std::lock_guard<std::mutex> locker(mutex_);
    return deque_.size() >= capacity_;
}

template<class T>
size_t BlockDeque<T>::size() {
    std::lock_guard<std::mutex> locker(mutex_);
    return deque_.size();
}

template<class T>
size_t BlockDeque<T>::capacity() {
    std::lock_guard<std::mutex> locker(mutex_);
    return capacity_;
}

template<class T>
T BlockDeque<T>::front() {
    std::lock_guard<std::mutex> locker(mutex_);
    return deque_.front();
}

template<class T>
T BlockDeque<T>::back() {
    std::lock_guard<std::mutex> locker(mutex_);
    return deque_.back();
}

template<class T>
void BlockDeque<T>::push_back(const T &item) {
    std::unique_lock<std::mutex> locker(mutex_);
    while (deque_.size() >= capacity_) {
        producer_.wait(locker);
    }
    deque_.push_back(item);
    consumer_.notify_one();
}

template<class T>
void BlockDeque<T>::push_front(const T &item) {
    std::unique_lock<std::mutex> locker(mutex_);
    while (deque_.size() >= capacity_) {
        producer_.wait(locker);
    }
    deque_.push_front(item);
    consumer_.notify_one();
}

template<class T>
bool BlockDeque<T>::pop_front(T &item) {
    std::unique_lock<std::mutex> locker(mutex_);
    while (deque_.empty()) {
        consumer_.wait(locker);
        if (isClose_) {
            return false;  
        }
    }
    item = deque_.front();
    deque_.pop_front();
    producer_.notify_one();
    return true;
}

template<class T>
bool BlockDeque<T>::pop_front(T &item, int timeout) {
    std::unique_lock<std::mutex> locker(mutex_);
    while (deque_.empty()) {
        if (consumer_.wait_for(locker, std::chrono::seconds(timeout)) == std::cv_status::timeout) {
            return false;  
        }
        if (isClose_) {
            return false;  
        }
    }
    item = deque_.front();
    deque_.pop_front();
    producer_.notify_one();
    return true;
}

#endif //SLIM_WEB_SERVER_BLOCK_DEQUE_H
