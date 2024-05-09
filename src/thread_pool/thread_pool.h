//
// Created by pyq on 5/9/24.
//
#pragma once
#ifndef SLIM_WEB_SERVER_THREAD_POOL_H
#define SLIM_WEB_SERVER_THREAD_POOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
#include <cassert>
#include <vector>
#include <atomic>

// A class that manages a pool of worker threads that can execute tasks concurrently.
class ThreadPool {
public:
    // Constructor that initializes the thread pool with a specified number of threads.
    explicit ThreadPool(size_t threadNum = 8);

    // Destructor that waits for all tasks to finish before exiting.
    ~ThreadPool();

    // Adds a new task to the thread pool.
    template<class T>
    void AddTask(T&& task){
        {
            std::lock_guard<std::mutex> locker(pool_->mutex_);
            pool_->tasks.emplace(std::forward<T>(task));
        }
        pool_->cv.notify_one();
    }

    // Closes the thread pool and joins all threads.
    void Close();
private:
     // Nested class that holds the queue of tasks and synchronization primitives.
    struct Pool {
        std::mutex mutex_;                          // Mutex to protect access to the task queue.
        std::condition_variable cv;                 // Condition variable for task synchronization.
        std::queue<std::function<void()>> tasks;    // Queue of tasks.
        bool isClosed;                              // Flag to indicate if the pool is shutting down.
    };
    std::shared_ptr<Pool> pool_;         // Shared pointer to the pool to ensure it lives as long as any thread needs it.
    std::vector<std::thread> workers;    // Vector of worker threads.
};

#endif //SLIM_WEB_SERVER_THREAD_POOL_H