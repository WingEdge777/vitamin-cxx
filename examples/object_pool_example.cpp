#include "vita/object_pool.h"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

namespace {

constexpr std::size_t kCapacity = 1024;
constexpr int kThreads = 4;
constexpr std::size_t kOpsPerThread = 50000;

} // namespace

int main() {
    vita::LockFreeObjectPool<int> pool(kCapacity);
    std::atomic<std::size_t> alloc_ok{0};
    std::atomic<std::size_t> alloc_empty{0};
    std::atomic<long long> checksum{0};

    // Exhaust once: capacity allocations should succeed, next must be empty.
    {
        std::vector<vita::LockFreeObjectPool<int>::Ptr> held;
        held.reserve(kCapacity);
        for (std::size_t i = 0; i < kCapacity; ++i) {
            auto p = pool.allocate(static_cast<int>(i));
            if (!p) {
                std::cerr << "unexpected empty allocate while filling pool\n";
                return 1;
            }
            held.push_back(std::move(p));
        }
        if (pool.allocate(0)) {
            std::cerr << "expected empty Ptr when pool is exhausted\n";
            return 1;
        }
    }

    const auto t0 = std::chrono::steady_clock::now();

    std::vector<std::thread> workers;
    workers.reserve(static_cast<std::size_t>(kThreads));
    for (int t = 0; t < kThreads; ++t) {
        workers.emplace_back([&pool, &alloc_ok, &alloc_empty, &checksum, t] {
            long long local_sum = 0;
            std::size_t local_ok = 0;
            std::size_t local_empty = 0;
            for (std::size_t i = 0; i < kOpsPerThread; ++i) {
                const int value = static_cast<int>(t * static_cast<int>(kOpsPerThread) + static_cast<int>(i));
                auto p = pool.allocate(value);
                if (!p) {
                    ++local_empty;
                    std::this_thread::yield();
                    continue;
                }
                local_sum += *p;
                ++local_ok;
                // Ptr goes out of scope → returns to pool.
            }
            checksum.fetch_add(local_sum, std::memory_order_relaxed);
            alloc_ok.fetch_add(local_ok, std::memory_order_relaxed);
            alloc_empty.fetch_add(local_empty, std::memory_order_relaxed);
        });
    }

    for (auto &w : workers) {
        w.join();
    }

    const auto t1 = std::chrono::steady_clock::now();
    const double elapsed_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    const std::size_t ok = alloc_ok.load();
    const std::size_t empty = alloc_empty.load();
    const double ops_per_sec = elapsed_ms > 0.0 ? (static_cast<double>(ok) * 1000.0 / elapsed_ms) : 0.0;

    std::cout << "object_pool capacity=" << kCapacity << " threads=" << kThreads << " alloc_ok=" << ok
              << " alloc_empty=" << empty << " checksum=" << checksum.load() << " elapsed_ms=" << std::fixed
              << std::setprecision(3) << elapsed_ms << " ops_per_sec=" << std::setprecision(0) << ops_per_sec << '\n';

    return ok > 0 ? 0 : 1;
}
