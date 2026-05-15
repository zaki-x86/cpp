#pragma once
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace foundation {

// ---- Self-registering factory ----
// Each type registers itself via a static initializer.
// No central switch — open/closed principle in action.

struct Animal {
    virtual ~Animal() = default;
    virtual std::string speak() const = 0;
};

class AnimalFactory {
    using Creator = std::function<std::unique_ptr<Animal>()>;

    static std::unordered_map<std::string, Creator>& registry() {
        static std::unordered_map<std::string, Creator> reg;
        return reg;
    }

public:
    static bool register_type(const std::string& name, Creator creator) {
        return registry().emplace(name, std::move(creator)).second;
    }

    static std::unique_ptr<Animal> create(const std::string& name) {
        auto it = registry().find(name);
        if (it == registry().end()) return nullptr;
        return it->second();
    }
};

struct Dog : Animal {
    std::string speak() const override { return "Woof"; }
    static const bool registered_;
};
inline const bool Dog::registered_ =
    AnimalFactory::register_type("Dog", []{ return std::make_unique<Dog>(); });

struct Cat : Animal {
    std::string speak() const override { return "Meow"; }
    static const bool registered_;
};
inline const bool Cat::registered_ =
    AnimalFactory::register_type("Cat", []{ return std::make_unique<Cat>(); });

} // namespace foundation
