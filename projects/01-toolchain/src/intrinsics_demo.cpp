#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>

static bool cpu_has_avx2() {
    uint32_t eax, ebx, ecx, edx;
    // CPUID leaf 7, subleaf 0 → EBX bit 5 = AVX2
    __asm__ volatile(
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(7u), "c"(0u)
    );
    return (ebx & (1u << 5)) != 0;
}

static float dot_avx2(const float* a, const float* b, int n) {
    __m256 acc = _mm256_setzero_ps();
    int i = 0;
    for (; i + 8 <= n; i += 8) {
        __m256 va = _mm256_loadu_ps(a + i);
        __m256 vb = _mm256_loadu_ps(b + i);
        acc = _mm256_fmadd_ps(va, vb, acc);
    }
    // Horizontal sum of 8-wide accumulator
    __m128 hi  = _mm256_extractf128_ps(acc, 1);
    __m128 lo  = _mm256_castps256_ps128(acc);
    lo = _mm_add_ps(lo, hi);
    lo = _mm_hadd_ps(lo, lo);
    lo = _mm_hadd_ps(lo, lo);
    float result = _mm_cvtss_f32(lo);
    for (; i < n; ++i) result += a[i] * b[i];
    return result;
}
#else
static bool cpu_has_avx2() { return false; }
#endif

static float dot_scalar(const float* a, const float* b, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) sum += a[i] * b[i];
    return sum;
}

int main() {
    constexpr int N = 1 << 20;  // 1M floats
    std::vector<float> a(N, 1.0f), b(N, 2.0f);

    using clock = std::chrono::high_resolution_clock;
    std::cout << "CPU has AVX2: " << (cpu_has_avx2() ? "yes" : "no") << "\n\n";

    auto t0 = clock::now();
    float s = dot_scalar(a.data(), b.data(), N);
    auto t1 = clock::now();
    auto scalar_us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    std::cout << "Scalar: result=" << s << "  time=" << scalar_us << " µs\n";

#if defined(__x86_64__) || defined(__i386__)
    if (cpu_has_avx2()) {
        auto t2 = clock::now();
        float v = dot_avx2(a.data(), b.data(), N);
        auto t3 = clock::now();
        auto avx2_us = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
        std::cout << "AVX2:   result=" << v << "  time=" << avx2_us << " µs\n";
        if (avx2_us > 0)
            std::cout << "Speedup: " << static_cast<float>(scalar_us) / static_cast<float>(avx2_us) << "x\n";
    }
#endif
    return 0;
}
