//
// Created by pyq on 5/7/24.
//
#pragma once
#ifndef SLIM_WEB_SERVER_TIMER_H
#define SLIM_WEB_SERVER_TIMER_H

#include <queue>
#include <unordered_map>
#include <algorithm>
#include <chrono>
#include <cassert>
#include <functional>
#include <time.h>
#include <arpa/inet.h>
#include <time.h>
#include "../log/log.h"

using TimeoutCallBack = std::function<void()>;
using HighResolutionClock = std::chrono::high_resolution_clock;
using Milliseconds = std::chrono::milliseconds;
using Timestamp = HighResolutionClock::time_point;

// A single timer instance with an expiration time and callback function.
struct TimerNode {
    int id;             // Unique identifier for the timer.
    Timestamp expires;  // Expiration time of the timer.
    TimeoutCallBack cb; // Callback function to execute when timer expires.
    // Overload the "<" to compare the two nodes based on expiration time.
    bool operator<(const TimerNode& t) {
        return expires < t.expires;
    }
};

// A class for managing timers using a min-heap to efficiently track the next timer to expire.
class Timer {
public:
    // Constructor to initialize the timer system.
    Timer();

    // Destructor to clean up resources.
    ~Timer();

    // Adjusts the expiration time of an existing timer.
    void Adjust(int id, int timeOut);

    // Adds a new timer.
    void Add(int id, int timeOut, const TimeoutCallBack& cb);
    
    // Executes the callback for a specific timer and removes it.
    void DoWork(int id);

    // Checks and executes all expired timers.
    void Tick();

    // Returns the time until the next timer expires.
    int GetNextTick();

    // Removes the top element from the heap.
    void Pop();

    // Clears all timers from the system.
    void Clear();
private:
    // Removes a timer from the heap.
    void Delete_(size_t i);

    // Swaps two nodes in the heap.
    void SwapNode_(size_t i, size_t j);

    // Moves a node up in the heap to maintain heap property.
    void SiftUp_(size_t i);

    // Moves a node down in the heap to maintain heap property.
    void SiftDown_(size_t i, size_t n);
    
    // Use vector to simulate minheap, 
    // parent node of i is (i - 1) / 2, 
    // left child node of i is 2 * i + 1 and right is 2 * i + 2
    std::vector<TimerNode> heap_;         
    std::unordered_map<int, size_t> ref_; // Maps timer IDs to their position in the heap for quick access.
};

#endif //SLIM_WEB_SERVER_TIMER_H
