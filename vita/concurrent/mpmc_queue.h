#pragma once

#include <atomic>
#include <vector>

template <typename T>
class MPMCQueue {
private:
    struct slot {
        T data;
        std::atomic<size_t> seq;
        slot() : seq(0) {}
    };
    size_t cap_, mask_;
    std::vector<slot> slots_;
    std::atomic<size_t> head_, tail_;

public:
    explicit MPMCQueue(size_t capacity) : cap_(capacity), slots_(capacity), head_(0), tail_(0) {}
    MPMCQueue(const MPMCQueue &) = delete;
    MPMCQueue &operator=(const MPMCQueue &) = delete;
};
