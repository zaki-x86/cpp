// Compile: g++ -std=c++20 -Wall -Wextra -o /tmp/t02 02_smart_pointers.cpp
// Demonstrates: unique_ptr with custom deleter, shared_ptr cycle leak, weak_ptr fix, make_shared

#include <cstdio>
#include <memory>
#include <string>

// --- Demo 1: unique_ptr with custom deleter ---
void demo_unique_ptr() {
    printf("\n--- Demo 1: unique_ptr with custom deleter (FILE*) ---\n");
    auto fp = std::unique_ptr<FILE, decltype(&fclose)>(
        fopen("/dev/null", "w"), &fclose);
    if (fp) {
        fprintf(fp.get(), "hello\n");
        printf("File written. Will be closed on scope exit.\n");
    }
    // fclose called automatically here
    printf("unique_ptr destroyed — fclose called\n");
}

// --- Demo 2: shared_ptr cycle (leak) ---
struct NodeLeak {
    std::string name;
    std::shared_ptr<NodeLeak> next;
    NodeLeak(const char* n) : name(n) {}
    ~NodeLeak() { printf("NodeLeak %s destroyed\n", name.c_str()); }
};

void demo_shared_ptr_cycle() {
    printf("\n--- Demo 2: shared_ptr cycle (destructors should NOT print) ---\n");
    {
        auto a = std::make_shared<NodeLeak>("A");
        auto b = std::make_shared<NodeLeak>("B");
        a->next = b;
        b->next = a;  // cycle: a->b->a
    }
    printf("Both nodes went out of scope. If no destructor printed: cycle leaked.\n");
}

// --- Demo 3: weak_ptr breaks cycle ---
struct NodeFixed {
    std::string name;
    std::shared_ptr<NodeFixed> next;
    std::weak_ptr<NodeFixed> prev;  // weak — does not own
    NodeFixed(const char* n) : name(n) {}
    ~NodeFixed() { printf("NodeFixed %s destroyed\n", name.c_str()); }
};

void demo_weak_ptr_fix() {
    printf("\n--- Demo 3: weak_ptr breaks cycle (both destructors SHOULD print) ---\n");
    {
        auto a = std::make_shared<NodeFixed>("A");
        auto b = std::make_shared<NodeFixed>("B");
        a->next = b;
        b->prev = a;  // weak — no cycle
    }
}

// --- Demo 4: make_shared vs shared_ptr(new T) ---
// make_shared<T>: one allocation — control block and T stored inline.
// shared_ptr(new T): two allocations — one for T (via new), one for the control block.
// The difference matters for cache locality and peak memory (see deep-dive.md).
struct Tracked {
    static int count;
    int id;
    Tracked() : id(++count) { printf("  Tracked #%d constructed\n", id); }
    ~Tracked() { printf("  Tracked #%d destroyed\n", id); }
};
int Tracked::count = 0;

void demo_make_shared() {
    printf("\n--- Demo 4: make_shared (1 alloc) vs shared_ptr(new T) (2 allocs) ---\n");
    printf("make_shared<Tracked>(): control block + object in one allocation\n");
    {
        auto p = std::make_shared<Tracked>();
        (void)p;
    }
    printf("shared_ptr(new Tracked): separate allocations for object and control block\n");
    {
        auto p = std::shared_ptr<Tracked>(new Tracked());
        (void)p;
    }
}

int main() {
    demo_unique_ptr();
    demo_shared_ptr_cycle();
    demo_weak_ptr_fix();
    demo_make_shared();
    return 0;
}
