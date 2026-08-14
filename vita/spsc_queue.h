#pragma once

#include <atomic>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace vita {

/**
 * Bounded SPSC queue with free-running head/tail counters.
 * Capacity is the exact number of slots usable (power of 2, >= 1).
 * Empty: head == tail; full: tail - head == capacity (unsigned).
 * Index into the ring only when accessing the buffer: seq & (capacity - 1).
 */
template <typename T>
class SPSCQueue {
private:
    size_t cap_, mask_;
    std::vector<T *> buffer_;
    alignas(64) std::atomic<size_t> head_;
    size_t cached_tail_;
    alignas(64) std::atomic<size_t> tail_;
    size_t cached_head_;

public:
    explicit SPSCQueue(size_t capacity) : cap_(capacity), head_(0), cached_tail_(0), tail_(0), cached_head_(0) {
        if (capacity < 1 || (capacity & (capacity - 1)) != 0) {
            throw std::invalid_argument("capacity must be a power of 2 and >= 1");
        }
        mask_ = cap_ - 1;
        buffer_.resize(cap_);
    }

    SPSCQueue(const SPSCQueue &) = delete;
    SPSCQueue &operator=(const SPSCQueue &) = delete;

    bool try_put(T *data) {
        const size_t t = tail_.load(std::memory_order_relaxed);
        if (t - cached_head_ == cap_) {
            cached_head_ = head_.load(std::memory_order_acquire);
            if (t - cached_head_ == cap_) {
                return false;
            }
        }
        buffer_[t & mask_] = data;
        tail_.store(t + 1, std::memory_order_release);
        return true;
    }

    T *try_get() {
        const size_t h = head_.load(std::memory_order_relaxed);
        if (h == cached_tail_) {
            cached_tail_ = tail_.load(std::memory_order_acquire);
            if (h == cached_tail_) {
                return nullptr;
            }
        }
        T *res = buffer_[h & mask_];
        head_.store(h + 1, std::memory_order_release);
        return res;
    }
};

} // namespace vita
