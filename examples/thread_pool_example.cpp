#include "vita/thread_pool.h"

#include <chrono>
#include <cstddef>
#include <future>
#include <iomanip>
#include <iostream>
#include <vector>

namespace {

constexpr std::size_t kWorkers = 4;
constexpr std::size_t kTasks = 10000;

} // namespace

int main() {
    vita::ThreadPool pool(kWorkers);
    std::vector<std::future<int>> futures;
    futures.reserve(kTasks);

    const auto t0 = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < kTasks; ++i) {
        futures.emplace_back(pool.enqueue([](int x) { return x * x; }, static_cast<int>(i)));
    }

    long long sum = 0;
    for (auto &f : futures) {
        sum += f.get();
    }
    const auto t1 = std::chrono::steady_clock::now();

    const long long expected = [&] {
        long long s = 0;
        for (std::size_t i = 0; i < kTasks; ++i) {
            s += static_cast<long long>(i) * static_cast<long long>(i);
        }
        return s;
    }();

    const double elapsed_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    const double ops_per_sec = elapsed_ms > 0.0 ? (static_cast<double>(kTasks) * 1000.0 / elapsed_ms) : 0.0;

    std::cout << "thread_pool tasks=" << kTasks << " workers=" << kWorkers << " sum=" << sum
              << " elapsed_ms=" << std::fixed << std::setprecision(3) << elapsed_ms
              << " ops_per_sec=" << std::setprecision(0) << ops_per_sec << '\n';

    return sum == expected ? 0 : 1;
}
