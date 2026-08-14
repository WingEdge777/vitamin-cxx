#include "vita/mpmc_queue.h"

#include <atomic>
#include <cstddef>
#include <iostream>
#include <thread>
#include <vector>

namespace {

constexpr std::size_t kCapacity = 1024;
constexpr int kProducers = 2;
constexpr int kConsumers = 2;
constexpr std::size_t kPerProducer = 10000;
constexpr std::size_t kExpected = static_cast<std::size_t>(kProducers) * kPerProducer;

} // namespace

int main() {
    vita::MPMCQueue<int> queue(kCapacity);
    std::atomic<std::size_t> produced{0};
    std::atomic<std::size_t> consumed{0};

    std::vector<std::thread> producers;
    producers.reserve(static_cast<std::size_t>(kProducers));
    for (int p = 0; p < kProducers; ++p) {
        producers.emplace_back([&queue, &produced, p] {
            for (std::size_t i = 0; i < kPerProducer; ++i) {
                auto *value = new int(static_cast<int>(p * static_cast<int>(kPerProducer) + static_cast<int>(i)));
                while (!queue.put(value)) {
                    std::this_thread::yield();
                }
                produced.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    std::vector<std::thread> consumers;
    consumers.reserve(static_cast<std::size_t>(kConsumers));
    for (int c = 0; c < kConsumers; ++c) {
        (void)c;
        consumers.emplace_back([&queue, &consumed] {
            while (consumed.load(std::memory_order_acquire) < kExpected) {
                int *value = queue.get();
                if (value == nullptr) {
                    std::this_thread::yield();
                    continue;
                }
                delete value;
                consumed.fetch_add(1, std::memory_order_release);
            }
        });
    }

    for (auto &t : producers) {
        t.join();
    }
    for (auto &t : consumers) {
        t.join();
    }

    const std::size_t produced_total = produced.load();
    const std::size_t consumed_total = consumed.load();
    std::cout << "produced=" << produced_total << " consumed=" << consumed_total << '\n';

    if (produced_total != kExpected || consumed_total != kExpected) {
        return 1;
    }
    return 0;
}
