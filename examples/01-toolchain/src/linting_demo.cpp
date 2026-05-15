// Intentional lint issues for clang-tidy demonstration.
// Run: cmake --preset clang-tidy && cmake --build --preset clang-tidy
// Expected: warnings for NULL, missing virtual destructor, C-style cast.

#include <cstdio>

void badNull(int* p) {
    if (p == NULL) return;      // clang-tidy: modernize-use-nullptr
    printf("%d\n", *p);         // clang-tidy: prefer std::cout (readability)
}

class Base {
public:
    virtual void doSomething() {}
    // Missing: virtual ~Base() = default;   <- clang-tidy flags this
};

class Derived : public Base {
public:
    void doSomething() override {}
};

void castDemo() {
    double d = 3.14;
    int i = (int)d;    // clang-tidy: cppcoreguidelines-pro-type-cstyle-cast
    (void)i;
}

int main() {
    badNull(nullptr);
    Derived d;
    d.doSomething();
    castDemo();
    return 0;
}
