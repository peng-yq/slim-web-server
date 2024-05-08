//
// Created by pyq on 5/7/24.
//
#include "timer.h"

Timer::Timer() {
  heap_.reserve(64);
}

Timer::~Timer() {
    Clear();
}

void Timer::Adjust(int id, int timeOut) {
    if (!heap_.empty() && ref_.count(id) > 0) {
        size_t i = ref_[id];
        auto oldExpires = heap_[i].expires;
        auto newExpires = HighResolutionClock::now() + Milliseconds(timeOut);
        heap_[i].expires = newExpires;
        // do sift function depends on the new expires and old expires
        if (newExpires < oldExpires) {
            SiftUp_(i);
        } else if (newExpires > oldExpires) {
            SiftDown_(i, heap_.size());
        }
    }
}

void Timer::Add(int id, int timeOut, const TimeoutCallBack& cb) {
    assert(id > 0);
    size_t i;
    if (ref_.count(id) == 0) {
        // push back the new node and shift up
        i = heap_.size();
        ref_[id] = i;
        heap_.push_back({id, HighResolutionClock::now() + Milliseconds(timeOut), cb});
        SiftUp_(i);
    } else {
        // update the node
        i = ref_[id];
        auto oldExpires = heap_[i].expires;
        auto newExpires = HighResolutionClock::now() + Milliseconds(timeOut);
        heap_[i].expires = newExpires;
        heap_[i].cb = cb;
        // do sift function depends on the new expires and old expires
        if (newExpires < oldExpires) {
            SiftUp_(i);
        } else if (newExpires > oldExpires) {
            SiftDown_(i, heap_.size());
        }
    }
}

void Timer::DoWork(int id) {
    if (heap_.empty() || ref_.count(id) == 0) {
        return;
    }
    size_t i = ref_[id];
    TimerNode node = heap_[i];
    node.cb();
    Delete_(i);
}

void Timer::Tick() {
    while (!heap_.empty()) {
        TimerNode node = heap_.front();
        // Check the whether timernode is out of date
        if (std::chrono::duration_cast<Milliseconds>(node.expires - HighResolutionClock::now()).count() > 0) {
            break;
        }
        node.cb();
        Pop();
    }
}

int Timer::GetNextTick() {
    Tick();
    int res = -1;
    if (!heap_.empty()) {
        res = std::chrono::duration_cast<Milliseconds>(heap_.front().expires - HighResolutionClock::now()).count();
        if (res < 0) {
            res = 0;
        }
    }
    return res;
}

void Timer::Pop() {
    if (!heap_.empty()) {
        Delete_(0);
    }
}

void Timer::Clear() {
    ref_.clear();
    heap_.clear();
}

void Timer::Delete_(size_t i) {
    assert(!heap_.empty() && i >= 0 && i < heap_.size());
    size_t n = heap_.size() - 1;
    if (i < n) {
        // swap the target node and last node, delete the last node and sift
        TimerNode node = heap_[i];
        SwapNode_(i, n);
        if (heap_[i] < node) {
            SiftUp_(i);
        } else {
            SiftDown_(i, n);
        }
    }
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

void Timer::SwapNode_(size_t i, size_t j) {
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i], heap_[j]);
    // Update the ref_ because the content of the same index has changed
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
}

void Timer::SiftUp_(size_t i) {
    assert(i >= 0 && i < heap_.size());
    // Calculate the index of the parent node
    while (i > 0) {
        size_t j = (i - 1) / 2;
        if (heap_[j] < heap_[i]) {
            break;
        }
        // Swap the nodes while value of parent node is bigger
        SwapNode_(i, j);
        i = j;
    }
}

// n is the upper limit of adjusting the index
void Timer::SiftDown_(size_t i, size_t n) {
    assert(i >= 0 && i < heap_.size());
    assert(n >= 0 && n <= heap_.size());
    // Calculate the index of the left child node
    size_t j = i * 2 + 1;
    while(j < n) {
        if(j + 1 < n && heap_[j + 1] < heap_[j]) {
            j++;
        }
        if(heap_[i] < heap_[j]) {
            break;
        }
        // Swap the nodes while value of parent node is bigger
        SwapNode_(i, j);
        i = j;
        j = i * 2 + 1;
    }
}

