#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <new>
#include <utility>

namespace foundation {

// ---- Arena Allocator ----
// Linear O(1) alloc, O(1) reset. No per-object free.
// Ideal for per-frame or per-request scoped allocations.
class ArenaAllocator {
    std::byte* base_;
    std::size_t capacity_;
    std::size_t used_{0};

    static std::size_t align_up(std::size_t v, std::size_t a) noexcept {
        return (v + a - 1) & ~(a - 1);
    }

public:
    explicit ArenaAllocator(std::size_t capacity)
        : base_{new std::byte[capacity]}, capacity_{capacity} {}
    ~ArenaAllocator() { delete[] base_; }

    ArenaAllocator(const ArenaAllocator&) = delete;
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;

    void* alloc_raw(std::size_t size,
                    std::size_t alignment = alignof(std::max_align_t)) {
        std::size_t aligned = align_up(used_, alignment);
        if (aligned + size > capacity_) throw std::bad_alloc{};
        void* ptr = base_ + aligned;
        used_ = aligned + size;
        return ptr;
    }

    template<typename T>
    T* alloc(std::size_t count = 1) {
        return static_cast<T*>(alloc_raw(sizeof(T) * count, alignof(T)));
    }

    template<typename T, typename... Args>
    T* construct(Args&&... args) {
        return ::new(alloc<T>()) T{std::forward<Args>(args)...};
    }

    void reset() noexcept { used_ = 0; }
    std::size_t used()      const noexcept { return used_; }
    std::size_t remaining() const noexcept { return capacity_ - used_; }
};

// ---- Pool Allocator ----
// Fixed-size object pool with O(1) alloc/free via free-list.
// Zero fragmentation. All blocks same size.
template<typename T, std::size_t Capacity>
class PoolAllocator {
    union Block {
        alignas(T) std::byte storage[sizeof(T)];
        Block* next{nullptr};
    };

    Block blocks_[Capacity];
    Block* free_head_{nullptr};
    std::size_t available_{Capacity};

public:
    PoolAllocator() {
        for (std::size_t i = 0; i + 1 < Capacity; ++i)
            blocks_[i].next = &blocks_[i + 1];
        blocks_[Capacity - 1].next = nullptr;
        free_head_ = &blocks_[0];
    }

    T* allocate() {
        if (!free_head_) throw std::bad_alloc{};
        Block* blk = free_head_;
        free_head_ = blk->next;
        --available_;
        return reinterpret_cast<T*>(blk->storage);
    }

    void deallocate(T* p) noexcept {
        auto* blk = reinterpret_cast<Block*>(p);
        blk->next = free_head_;
        free_head_ = blk;
        ++available_;
    }

    std::size_t available() const noexcept { return available_; }
};

// ---- Stack Allocator ----
// LIFO allocator on a compile-time fixed buffer (zero heap).
template<std::size_t N>
class StackAllocator {
    alignas(std::max_align_t) std::byte buf_[N];
    std::size_t top_{0};
public:
    void* alloc(std::size_t size,
                std::size_t align = alignof(std::max_align_t)) {
        std::size_t aligned = (top_ + align - 1) & ~(align - 1);
        if (aligned + size > N) throw std::bad_alloc{};
        void* p = buf_ + aligned;
        top_ = aligned + size;
        return p;
    }
    void reset() noexcept { top_ = 0; }
    std::size_t used() const noexcept { return top_; }
};

} // namespace foundation
