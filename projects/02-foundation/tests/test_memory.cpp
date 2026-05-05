#include <gtest/gtest.h>
#include <foundation/memory/raii.hpp>
#include <foundation/memory/allocators.hpp>
#include <memory>
#include <stdexcept>

// --- RAII / ScopeGuard ---

TEST(RAII, ScopeGuardRunsOnDestruction) {
    bool ran = false;
    {
        auto guard = foundation::make_scope_guard([&]{ ran = true; });
    }
    EXPECT_TRUE(ran);
}

TEST(RAII, ScopeGuardDismissSkipsCleanup) {
    bool ran = false;
    {
        auto guard = foundation::make_scope_guard([&]{ ran = true; });
        guard.dismiss();
    }
    EXPECT_FALSE(ran);
}

TEST(RAII, ScopeGuardRunsOnException) {
    bool released = false;
    try {
        auto guard = foundation::make_scope_guard([&]{ released = true; });
        throw std::runtime_error("test");
    } catch (...) {}
    EXPECT_TRUE(released);
}

// --- Smart pointers (stdlib, verified via behaviour) ---

TEST(SmartPtrs, UniqueOwnership) {
    auto p = std::make_unique<int>(42);
    EXPECT_EQ(*p, 42);
    auto q = std::move(p);
    EXPECT_EQ(p, nullptr);
    EXPECT_EQ(*q, 42);
}

TEST(SmartPtrs, SharedRefCount) {
    auto a = std::make_shared<int>(10);
    EXPECT_EQ(a.use_count(), 1);
    {
        auto b = a;
        EXPECT_EQ(a.use_count(), 2);
    }
    EXPECT_EQ(a.use_count(), 1);
}

TEST(SmartPtrs, WeakPtrBreaksCycle) {
    struct Node {
        std::shared_ptr<Node> next;
        std::weak_ptr<Node>   prev;
        int val;
    };
    auto n1 = std::make_shared<Node>(Node{nullptr, {}, 1});
    auto n2 = std::make_shared<Node>(Node{nullptr, {}, 2});
    n1->next = n2;
    n2->prev = n1;
    EXPECT_EQ(n1.use_count(), 1);  // weak ref from n2 doesn't count
    EXPECT_EQ(n2.use_count(), 2);  // n1->next + local n2
}

TEST(SmartPtrs, CustomDeleterCalled) {
    bool deleted = false;
    {
        auto deleter = [&](int* p){ deleted = true; delete p; };
        std::unique_ptr<int, decltype(deleter)> p(new int(5), deleter);
    }
    EXPECT_TRUE(deleted);
}

// --- ArenaAllocator ---

TEST(ArenaAllocator, AllocatesWithinArena) {
    foundation::ArenaAllocator arena(1024);
    int* a = arena.alloc<int>(10);
    ASSERT_NE(a, nullptr);
    a[0] = 42; a[9] = 99;
    EXPECT_EQ(a[0], 42);
    EXPECT_EQ(a[9], 99);
}

TEST(ArenaAllocator, ResetReclaims) {
    foundation::ArenaAllocator arena(256);
    void* p1 = arena.alloc_raw(100, 8);
    arena.reset();
    void* p2 = arena.alloc_raw(100, 8);
    EXPECT_EQ(p1, p2);
}

TEST(ArenaAllocator, ThrowsWhenFull) {
    foundation::ArenaAllocator arena(64);
    EXPECT_THROW(arena.alloc_raw(128, 1), std::bad_alloc);
}

// --- PoolAllocator ---

TEST(PoolAllocator, AllocAndFree) {
    foundation::PoolAllocator<int, 16> pool;
    int* a = pool.allocate();
    ASSERT_NE(a, nullptr);
    *a = 77;
    EXPECT_EQ(*a, 77);
    pool.deallocate(a);
}

TEST(PoolAllocator, ReusesFreeSlot) {
    foundation::PoolAllocator<int, 4> pool;
    int* a = pool.allocate();
    pool.deallocate(a);
    int* b = pool.allocate();
    EXPECT_EQ(a, b);  // same slot reused
}
