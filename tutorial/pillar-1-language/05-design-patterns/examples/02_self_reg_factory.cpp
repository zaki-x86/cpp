// 02_self_reg_factory.cpp
// Self-registering factory — Meyers singleton registry, static initializers.
//
// Key ideas:
//   - factory() returns a Meyers singleton map — avoids init-order fiasco.
//   - REGISTER_SHAPE macro: defines a static Registrar whose ctor registers
//     the type before main().
//   - No if-else chain in the factory — just a map lookup.
//
// Compile: g++ -std=c++20 -Wall -Wextra -o /tmp/t15 02_self_reg_factory.cpp

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

// ---- Abstract product ----------------------------------------------------

struct Shape {
    virtual ~Shape() = default;
    virtual void draw() const = 0;
};

// ---- Meyers singleton registry -------------------------------------------
// Local static is initialized on first call — thread-safe (C++11).
// Using a function avoids the static initialization order fiasco:
// registrations from other TUs call factory() before main(), and the map
// is always initialized before the first insertion.

using CreateFn = std::function<std::unique_ptr<Shape>()>;

auto& factory() {
    static std::unordered_map<std::string, CreateFn> reg;
    return reg;
}

// ---- Registrar helper ----------------------------------------------------

struct Registrar {
    Registrar(const std::string& name, CreateFn fn) {
        factory()[name] = std::move(fn);
    }
};

// ---- Registration macro --------------------------------------------------
// Defines a file-scope static Registrar — its constructor runs before main().

#define REGISTER_SHAPE(name, type)                                  \
    static Registrar reg_##type{                                    \
        name, []{ return std::make_unique<type>(); }                \
    }

// ---- Concrete products ---------------------------------------------------

struct Circle : Shape {
    void draw() const override { std::cout << "Drawing Circle\n"; }
};

struct Rectangle : Shape {
    void draw() const override { std::cout << "Drawing Rectangle\n"; }
};

// These lines run before main() — each calls factory()["name"] = lambda.
REGISTER_SHAPE("circle",    Circle);
REGISTER_SHAPE("rectangle", Rectangle);

// ---- main ----------------------------------------------------------------

int main() {
    // No if-else. No switch. Just a map lookup.
    const std::string keys[] = {"circle", "rectangle", "triangle"};

    for (const auto& key : keys) {
        auto it = factory().find(key);
        if (it == factory().end()) {
            std::cout << "'" << key << "' not registered\n";
            continue;
        }
        auto shape = it->second();   // call the factory function
        shape->draw();
    }

    // Show that the registry is populated — number of registered types.
    std::cout << "\nRegistered shapes: " << factory().size() << "\n";

    return 0;
}
