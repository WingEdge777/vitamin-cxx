#pragma once

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace vita {

/**
 * Lock-free object pool (Treiber free-list + tagged index).
 *
 * Contracts:
 * - All Ptr must be destroyed before the pool is destroyed.
 * - deallocate is not public; return objects only via Ptr.
 * - allocate returns an empty Ptr when the pool is exhausted (does not throw for OOM-of-pool).
 * - Pointers passed through Ptr must originate from this pool.
 */
template <typename T>
class LockFreeObjectPool {
public:
    struct Deleter {
        LockFreeObjectPool *pool;
        void operator()(T *ptr) const noexcept {
            if (pool && ptr) {
                pool->deallocate(ptr);
            }
        }
    };
    using Ptr = std::unique_ptr<T, Deleter>;

    explicit LockFreeObjectPool(size_t capacity) : cap_(capacity) {
        if (capacity == 0 || capacity >= NULL_IDX) {
            throw std::invalid_argument("Invalid capacity size");
        }
        pool_ = std::make_unique<Node[]>(cap_);
        for (size_t i = 0; i < cap_ - 1; i++) {
            pool_[i].next = static_cast<uint32_t>(i + 1);
        }
        pool_[cap_ - 1].next = NULL_IDX;
        free_head_.store(pack(0, 0), std::memory_order_relaxed);
    }

    ~LockFreeObjectPool() = default;

    LockFreeObjectPool(const LockFreeObjectPool &) = delete;
    LockFreeObjectPool &operator=(const LockFreeObjectPool &) = delete;

    template <typename... Args>
    Ptr allocate(Args &&...args) {
        uint64_t head = free_head_.load(std::memory_order_acquire);
        while (true) {
            uint32_t idx = get_idx(head);
            if (idx == NULL_IDX) {
                return Ptr(nullptr, Deleter{this});
            }
            uint32_t next_idx = pool_[idx].next;
            uint32_t tag = get_tag(head);
            uint64_t new_head = pack(tag + 1, next_idx); // avoid aba
            if (free_head_.compare_exchange_weak(
                    head, new_head, std::memory_order_acquire, std::memory_order_relaxed)) {
                T *ptr = reinterpret_cast<T *>(pool_[idx].storage);
                if constexpr (std::is_nothrow_constructible_v<T, Args...>) {
                    new (ptr) T(std::forward<Args>(args)...);
                    return Ptr(ptr, Deleter{this});
                } else {
                    try {
                        new (ptr) T(std::forward<Args>(args)...);
                        return Ptr(ptr, Deleter{this});
                    } catch (...) {
                        push_node_raw(idx);
                        throw;
                    }
                }
            }
        }
    }

private:
    struct Node {
        alignas(T) char storage[sizeof(T)];
        uint32_t next;
    };
    std::unique_ptr<Node[]> pool_;
    std::atomic<uint64_t> free_head_;
    size_t cap_;

    static constexpr uint32_t NULL_IDX = 0xffffffff;

    static constexpr uint64_t pack(uint32_t tag, uint32_t idx) { return (static_cast<uint64_t>(tag) << 32) | idx; }
    static constexpr uint32_t get_tag(uint64_t val) { return static_cast<uint32_t>(val >> 32); }
    static constexpr uint32_t get_idx(uint64_t val) { return static_cast<uint32_t>(val); }

    void deallocate(T *ptr) noexcept {
        if (!ptr) {
            return;
        }
        auto *node = reinterpret_cast<Node *>(ptr);
        const auto idx = static_cast<uint32_t>(node - pool_.get());
        // assert(idx < cap_ && "pointer does not belong to this LockFreeObjectPool");

        ptr->~T();
        push_node_raw(idx);
    }

    void push_node_raw(uint32_t idx) noexcept {
        uint64_t head = free_head_.load(std::memory_order_relaxed);
        while (true) {
            pool_[idx].next = get_idx(head);
            uint32_t tag = get_tag(head);
            uint64_t new_head = pack(tag + 1, idx);

            if (free_head_.compare_exchange_weak(
                    head, new_head, std::memory_order_release, std::memory_order_relaxed)) {
                break;
            }
        }
    }

    static_assert(offsetof(Node, storage) == 0, "Storage must be at the beginning of Node");
};

} // namespace vita
