// Compile: g++ -std=c++20 -Wall -Wextra -o /tmp/t04 01_rule_of_five.cpp
// Demonstrates: Rule of Five — destructor, copy ctor, copy assign (copy-and-swap), move ctor, move assign

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <utility>

class Buffer {
    char*  data_;
    size_t size_;
public:
    explicit Buffer(size_t n) : data_(new char[n]()), size_(n) {
        printf("Buffer(%zu) constructed at %p\n", n, static_cast<void*>(this));
    }

    ~Buffer() {
        printf("~Buffer(%zu) at %p\n", size_, static_cast<void*>(this));
        delete[] data_;
    }

    // Copy constructor: deep copy
    Buffer(const Buffer& o) : data_(new char[o.size_]), size_(o.size_) {
        std::copy(o.data_, o.data_ + o.size_, data_);
        printf("Buffer copied (%zu bytes)\n", size_);
    }

    // Copy assignment: copy-and-swap (handles self-assignment, exception-safe)
    Buffer& operator=(Buffer o) noexcept {
        printf("Buffer copy-assigned\n");
        std::swap(data_, o.data_);
        std::swap(size_, o.size_);
        return *this;
    }

    // Move constructor: steal pointer, null source
    Buffer(Buffer&& o) noexcept : data_(o.data_), size_(o.size_) {
        o.data_ = nullptr;
        o.size_ = 0;
        printf("Buffer moved\n");
    }

    size_t size() const noexcept { return size_; }
    char*  data() const noexcept { return data_; }
};

int main() {
    printf("--- Copy ---\n");
    Buffer a(16);
    a.data()[0] = 'A';
    Buffer b = a;  // copy constructor
    printf("a[0]=%c b[0]=%c (independent: %s)\n",
           a.data()[0], b.data()[0],
           (a.data() != b.data()) ? "yes" : "NO BUG");

    printf("\n--- Move ---\n");
    Buffer c(8);
    Buffer d = std::move(c);  // move constructor
    printf("c.size()=%zu (moved-from, should be 0)\n", c.size());
    printf("d.size()=%zu (holds the data)\n", d.size());

    printf("\n--- Self-assignment ---\n");
    Buffer e(4);
    e = e;  // should not crash or double-free
    printf("Self-assignment survived\n");

    printf("\n--- Destructors (LIFO order) ---\n");
    return 0;
}
