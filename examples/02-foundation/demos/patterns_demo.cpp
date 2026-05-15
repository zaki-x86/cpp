#include <foundation/patterns/type_erasure.hpp>
#include <foundation/patterns/observer.hpp>
#include <foundation/patterns/factory.hpp>
#include <iostream>
#include <functional>
#include <vector>

int main() {
    std::cout << "=== Design Patterns Demo ===\n\n";

    // --- Type erasure (AnyCallable) ---
    std::cout << "-- Type Erasure (AnyCallable<Ret(Args...)>) --\n";
    {
        foundation::AnyCallable<int(int, int)> op;

        op = [](int a, int b){ return a + b; };
        std::cout << "  add(3,4) = " << op(3, 4) << "\n";

        op = [](int a, int b){ return a * b; };
        std::cout << "  mul(3,4) = " << op(3, 4) << "\n";

        // Store different callables in a vector
        std::vector<foundation::AnyCallable<int(int)>> transforms;
        transforms.emplace_back([](int x){ return x * 2; });
        transforms.emplace_back([](int x){ return x + 10; });
        transforms.emplace_back([](int x){ return x * x; });

        int val = 3;
        std::cout << "  pipeline " << val;
        for (auto& fn : transforms) {
            val = fn(val);
            std::cout << " -> " << val;
        }
        std::cout << "\n";
    }

    // --- Observer / EventEmitter ---
    std::cout << "\n-- Observer (EventEmitter<T>) --\n";
    {
        struct UserEvent { std::string name; int id; };
        foundation::EventEmitter<UserEvent> bus;

        auto tok1 = bus.subscribe([](const UserEvent& e){
            std::cout << "  Logger: user '" << e.name << "' (id=" << e.id << ") joined\n";
        });
        auto tok2 = bus.subscribe([](const UserEvent& e){
            std::cout << "  Notifier: welcome email sent to " << e.name << "\n";
        });

        bus.emit({"Alice", 1});
        bus.emit({"Bob", 2});

        std::cout << "  (unsubscribing logger)\n";
        bus.unsubscribe(tok1);
        bus.emit({"Carol", 3});  // only notifier fires
    }

    // --- Self-registering Factory ---
    std::cout << "\n-- Self-Registering Factory (AnimalFactory) --\n";
    {
        for (const char* name : {"Dog", "Cat", "Fish"}) {
            auto a = foundation::AnimalFactory::create(name);
            if (a)
                std::cout << "  " << name << " says: " << a->speak() << "\n";
            else
                std::cout << "  " << name << ": unknown animal\n";
        }
    }

    std::cout << "\nDone.\n";
}
