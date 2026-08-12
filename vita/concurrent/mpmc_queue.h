#pragma once

#include <atomic>
#include <stdexcept>
#include <vector>
namespace vita {

/**
 * align to 64 bytes for avoid false sharing cacheline
 */
template <typename T>
class MPMCQueue {
private:
    alignas(64) struct slot {
        T *data;
        std::atomic<size_t> seq;
        slot() : seq(0) {}
    };
    size_t cap_, mask_;
    std::unique_ptr<slot[]> slots_;
    alignas(64) std::atomic<size_t> head_;
    alignas(64) std::atomic<size_t> tail_;

public:
    explicit MPMCQueue(size_t capacity) : cap_(capacity), head_(0), tail_(0) {
        if (capacity < 2 || (capacity & (capacity - 1)) != 0) {
            throw std::invalid_argument("capacity must be a power of 2 and >= 2");
        }
        mask_ = cap_ - 1;
        slots_ = std::make_unique<slot[]>(cap_);
        for (size_t i = 0; i < cap_; i++) {
            slots_[i].seq.store(i, std::memory_order_relaxed);
        }
    }
    MPMCQueue(const MPMCQueue &) = delete;
    MPMCQueue &operator=(const MPMCQueue &) = delete;
    bool put(T *data) {
        size_t idx = tail_.load(std::memory_order_relaxed);

        while (true) {
            auto &node = slots_[idx & mask_];
            size_t seq = node.seq.load(std::memory_order_acquire);
            if (seq == idx) {
                if (tail_.compare_exchange_weak(idx, idx + 1, std::memory_order_relaxed)) {
                    node.data = data;
                    node.seq.store(idx + 1, std::memory_order_release);
                    return true;
                }
            } else if (seq < idx) {
                return false;
            } else {
                idx = tail_.load(std::memory_order_relaxed);
            }
        }
    }
    T *get() {
        size_t idx = head_.load(std::memory_order_relaxed);
        while (true) {
            auto &node = slots_[idx & mask_];
            size_t seq = node.seq.load(std::memory_order_acquire);
            if (seq == idx + 1) {
                if (head_.compare_exchange_weak(idx, idx + 1, std::memory_order_relaxed)) {
                    auto res = node.data;
                    node.seq.store(idx + cap_, std::memory_order_release);
                    return res;
                }
            } else if (seq < idx + 1) {
                return nullptr;
            } else {
                idx = head_.load(std::memory_order_relaxed);
            }
        }
    }
};

} // namespace vita
