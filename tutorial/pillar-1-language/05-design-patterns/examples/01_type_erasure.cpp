// 01_type_erasure.cpp
// Pedagogical std::function<void()> — the Concept+Model pattern.
//
// Three components:
//   Concept   — pure virtual interface (call + clone)
//   Model<F>  — wraps any callable F
//   AnyCallable — outer class with value semantics
//
// Compile: g++ -std=c++20 -Wall -Wextra -o /tmp/t14 01_type_erasure.cpp

#include <iostream>
#include <memory>
#include <utility>

// ---- Type-erased callable ------------------------------------------------

class AnyCallable {
    // Inner interface: the erased type must satisfy these two operations.
    struct Concept {
        virtual ~Concept() = default;
        virtual void call() = 0;
        virtual Concept* clone() const = 0;
    };

    // Inner model: wraps any callable F behind the Concept interface.
    template<typename F>
    struct Model final : Concept {
        F f_;
        explicit Model(F f) : f_(std::move(f)) {}
        void call() override { f_(); }
        // clone() enables copy semantics on the outer class.
        Concept* clone() const override { return new Model(f_); }
    };

    std::unique_ptr<Concept> self_;

public:
    AnyCallable() = default;

    // Construct from any callable — lambda, free fn pointer, functor.
    template<typename F>
    AnyCallable(F f)                                        // NOLINT(google-explicit-constructor)
        : self_(new Model<std::decay_t<F>>(std::move(f))) {}

    // Copy — uses clone() so each AnyCallable owns its own model.
    AnyCallable(const AnyCallable& other)
        : self_(other.self_ ? other.self_->clone() : nullptr) {}

    AnyCallable(AnyCallable&&) = default;

    AnyCallable& operator=(AnyCallable other) {
        std::swap(self_, other.self_);
        return *this;
    }

    void operator()() const { self_->call(); }

    explicit operator bool() const noexcept { return self_ != nullptr; }
};

// ---- Demo types ----------------------------------------------------------

// 1. A free function.
void greet_free() {
    std::cout << "Hello from free function\n";
}

// 2. A struct with operator()().
struct Greeter {
    std::string name;
    void operator()() const {
        std::cout << "Hello from Greeter: " << name << "\n";
    }
};

// ---- main ----------------------------------------------------------------

int main() {
    // Store a lambda.
    AnyCallable a{ []{ std::cout << "Hello from lambda\n"; } };

    // Store a free function pointer.
    AnyCallable b{ greet_free };

    // Store a struct with operator()().
    AnyCallable c{ Greeter{"Atlas"} };

    std::cout << "-- original calls --\n";
    a();
    b();
    c();

    // Copy semantics: each copy is independent.
    AnyCallable a2 = a;
    AnyCallable c2 = c;

    std::cout << "-- copy calls --\n";
    a2();   // same lambda, independent copy
    c2();   // same Greeter, independent copy

    // Demonstrate that copies are truly independent (modify original).
    a = []{ std::cout << "Hello from MODIFIED lambda\n"; };
    std::cout << "-- after modifying original --\n";
    a();    // modified
    a2();   // copy is unchanged

    return 0;
}
