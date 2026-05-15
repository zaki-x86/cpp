#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

// Compute-heavy workload used for:
//   1. PGO: generate profile, rebuild with -fprofile-use
//   2. LTO demo: release build links with IPO enabled
//   3. Assembly output: cmake --build . --target asm_output

static long sum_range(const std::vector<int>& v) {
    long s = 0;
    for (auto x : v) s += x;
    return s;
}

static void sort_chunks(std::vector<int>& v, std::size_t chunk_size) {
    for (std::size_t i = 0; i + chunk_size <= v.size(); i += chunk_size)
        std::sort(v.begin() + static_cast<long>(i),
                  v.begin() + static_cast<long>(i + chunk_size));
}

int main() {
    constexpr int N = 500'000;
    constexpr int ITERATIONS = 10;

    std::mt19937 rng{42};
    std::uniform_int_distribution<int> dist{0, 10'000};

    using clock = std::chrono::high_resolution_clock;
    long total_sum = 0;
    long total_ns  = 0;

    for (int iter = 0; iter < ITERATIONS; ++iter) {
        std::vector<int> v(N);
        std::generate(v.begin(), v.end(), [&]{ return dist(rng); });

        auto t0 = clock::now();
        sort_chunks(v, 1000);
        total_sum += sum_range(v);
        auto t1 = clock::now();

        total_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    }

    std::cout << "Sum: " << total_sum << "\n";
    std::cout << "Avg: " << (total_ns / ITERATIONS / 1'000) << " µs/iter\n";
    return 0;
}
