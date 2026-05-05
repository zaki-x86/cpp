#include <foundation/memory/raii.hpp>
#include <foundation/memory/allocators.hpp>
#include <iostream>
#include <memory>

int main() {
    std::cout << "=== Memory Demo ===\n\n";

    // --- ScopeGuard ---
    std::cout << "-- ScopeGuard --\n";
    {
        auto g = foundation::make_scope_guard([]{ std::cout << "  ScopeGuard fired\n"; });
        std::cout << "  (inside scope)\n";
    }

    {
        auto g = foundation::make_scope_guard([]{ std::cout << "  This should NOT print\n"; });
        g.dismiss();
        std::cout << "  ScopeGuard dismissed\n";
    }

    // FOUNDATION_DEFER macro
    {
        FOUNDATION_DEFER(std::cout << "  DEFER fired\n");
        std::cout << "  (after defer setup)\n";
    }

    // --- Smart pointers ---
    std::cout << "\n-- Smart Pointers --\n";
    {
        auto p = std::make_unique<int>(42);
        std::cout << "  unique_ptr value: " << *p << "\n";
        auto q = std::move(p);
        std::cout << "  after move, original null: " << (p == nullptr) << "\n";
    }

    {
        auto a = std::make_shared<int>(100);
        std::cout << "  shared_ptr use_count=1: " << a.use_count() << "\n";
        {
            auto b = a;
            std::cout << "  shared_ptr use_count=2: " << a.use_count() << "\n";
        }
        std::cout << "  shared_ptr use_count=1 again: " << a.use_count() << "\n";
    }

    // Custom deleter
    {
        bool deleted = false;
        auto del = [&](int* p){ deleted = true; delete p; };
        {
            std::unique_ptr<int, decltype(del)> p(new int(7), del);
        }
        std::cout << "  custom deleter called: " << deleted << "\n";
    }

    // --- ArenaAllocator ---
    std::cout << "\n-- ArenaAllocator --\n";
    {
        foundation::ArenaAllocator arena(1024);
        int* ints = arena.alloc<int>(10);
        for (int i = 0; i < 10; ++i) ints[i] = i * i;
        std::cout << "  squares: ";
        for (int i = 0; i < 10; ++i) std::cout << ints[i] << " ";
        std::cout << "\n";

        arena.reset();
        int* after_reset = arena.alloc<int>(1);
        std::cout << "  same address after reset: " << (after_reset == ints) << "\n";
    }

    // --- PoolAllocator ---
    std::cout << "\n-- PoolAllocator --\n";
    {
        foundation::PoolAllocator<double, 8> pool;
        double* a = pool.allocate();
        double* b = pool.allocate();
        *a = 3.14; *b = 2.71;
        std::cout << "  a=" << *a << "  b=" << *b << "\n";
        pool.deallocate(a);
        double* c = pool.allocate();
        std::cout << "  reused slot (c==a): " << (c == a) << "\n";
        pool.deallocate(b);
        pool.deallocate(c);
    }

    std::cout << "\nDone.\n";
}
