//
// Created by pyq on 5/9/24.
//
#include "thread_pool.h"

ThreadPool::ThreadPool(size_t threadNum) : pool_(std::make_shared<Pool>()) {
    assert(threadNum > 0);
    pool_->isClosed = false;
    for (size_t i = 0; i < threadNum; ++i) {
        workers.emplace_back([pool = pool_] {
            std::unique_lock<std::mutex> locker(pool->mutex_);
            while (true) {
                if (!pool->tasks.empty()) {
                    auto task = std::move(pool->tasks.front());
                    pool->tasks.pop();
                    locker.unlock();
                    task();
                    locker.lock();
                } else if (pool->isClosed) {
                    break;
                } else {
                    pool->cv.wait(locker);
                }
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    Close();
}

void ThreadPool::Close() {
    {
        std::lock_guard<std::mutex> locker(pool_->mutex_);
        pool_->isClosed = true;
    }
    pool_->cv.notify_all();
    for (std::thread& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    workers.clear();
}