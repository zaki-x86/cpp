// Compile: g++ -std=c++20 -Wall -Wextra -o /tmp/t03 03_arena_allocator.cpp
// Demonstrates: bump-pointer arena allocator

#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <stdexcept>
#include <new>

class ArenaAllocator {
    alignas(std::max_align_t) char buffer_[4096];
    std::size_t offset_ = 0;

public:
    void* allocate(std::size_t bytes, std::size_t align = alignof(std::max_align_t)) {
        // Round offset up to alignment boundary
        std::size_t aligned = (offset_ + align - 1) & ~(align - 1);
        if (aligned + bytes > sizeof(buffer_))
            throw std::bad_alloc{};
        offset_ = aligned + bytes;
        return buffer_ + aligned;
    }

    void reset() noexcept { offset_ = 0; }

    std::size_t bytes_used() const noexcept { return offset_; }

    const char* base() const noexcept { return buffer_; }
};

struct Point { float x, y; };

int main() {
    ArenaAllocator arena;

    printf("Arena base: %p\n", static_cast<const void*>(arena.base()));
    printf("\n--- First pass of allocations ---\n");

    // Allocate an int
    int* i = static_cast<int*>(arena.allocate(sizeof(int), alignof(int)));
    *i = 42;
    printf("int*    at %p (offset %zu, alignof(int)=%zu): value=%d\n",
           static_cast<void*>(i),
           static_cast<std::size_t>(reinterpret_cast<char*>(i) - arena.base()),
           alignof(int), *i);
    printf("  8-byte aligned? %s\n", (reinterpret_cast<uintptr_t>(i) % alignof(double)) == 0 ? "yes (lucky)" : "no");

    // Allocate a double
    double* d = static_cast<double*>(arena.allocate(sizeof(double), alignof(double)));
    *d = 3.14;
    printf("double* at %p (offset %zu, alignof(double)=%zu): value=%.2f\n",
           static_cast<void*>(d),
           static_cast<std::size_t>(reinterpret_cast<char*>(d) - arena.base()),
           alignof(double), *d);
    printf("  8-byte aligned? %s\n", (reinterpret_cast<uintptr_t>(d) % alignof(double)) == 0 ? "yes" : "NO — BUG");

    // Allocate a Point
    Point* p = static_cast<Point*>(arena.allocate(sizeof(Point), alignof(Point)));
    p->x = 1.0f; p->y = 2.0f;
    printf("Point*  at %p (offset %zu, alignof(Point)=%zu): (%g, %g)\n",
           static_cast<void*>(p),
           static_cast<std::size_t>(reinterpret_cast<char*>(p) - arena.base()),
           alignof(Point), p->x, p->y);
    printf("  4-byte aligned? %s\n", (reinterpret_cast<uintptr_t>(p) % alignof(Point)) == 0 ? "yes" : "NO — BUG");

    printf("\nTotal bytes used: %zu / 4096\n", arena.bytes_used());

    // All pointers are within the buffer
    printf("\nAll within buffer? %s\n",
        (reinterpret_cast<char*>(i) >= arena.base() &&
         reinterpret_cast<char*>(d) >= arena.base() &&
         reinterpret_cast<char*>(p) >= arena.base() &&
         reinterpret_cast<char*>(p) + sizeof(Point) <= arena.base() + 4096)
        ? "YES" : "NO — BUG");

    // Reset and reuse
    printf("\n--- After reset() ---\n");
    void* first_before = static_cast<void*>(i);  // save address
    arena.reset();
    printf("bytes_used after reset: %zu\n", arena.bytes_used());

    int* i2 = static_cast<int*>(arena.allocate(sizeof(int), alignof(int)));
    printf("First alloc after reset: %p (same as before? %s)\n",
           static_cast<void*>(i2),
           (static_cast<void*>(i2) == first_before) ? "YES — buffer reused" : "NO");

    return 0;
}
