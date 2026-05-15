#include <gtest/gtest.h>
#include <foundation/patterns/type_erasure.hpp>
#include <foundation/patterns/observer.hpp>
#include <foundation/patterns/factory.hpp>
#include <functional>
#include <string>

// --- Type erasure (AnyCallable) ---

TEST(TypeErasure, WrapsLambda) {
    foundation::AnyCallable<int(int)> f([](int x){ return x * 2; });
    EXPECT_EQ(f(5), 10);
}

TEST(TypeErasure, WrapsStdFunction) {
    std::function<std::string(const std::string&)> fn = [](const std::string& s){ return s + "!"; };
    foundation::AnyCallable<std::string(const std::string&)> f(fn);
    EXPECT_EQ(f("hello"), "hello!");
}

TEST(TypeErasure, Reassignable) {
    foundation::AnyCallable<int()> f([]{ return 1; });
    EXPECT_EQ(f(), 1);
    f = []{ return 2; };
    EXPECT_EQ(f(), 2);
}

// --- EventEmitter (Observer) ---

TEST(Observer, SubscribeAndEmit) {
    foundation::EventEmitter<int> emitter;
    int received = -1;
    emitter.subscribe([&](const int& v){ received = v; });
    emitter.emit(42);
    EXPECT_EQ(received, 42);
}

TEST(Observer, MultipleSubscribers) {
    foundation::EventEmitter<std::string> emitter;
    std::vector<std::string> log;
    emitter.subscribe([&](const std::string& s){ log.push_back("A:" + s); });
    emitter.subscribe([&](const std::string& s){ log.push_back("B:" + s); });
    emitter.emit("hello");
    EXPECT_EQ(log.size(), 2u);
}

TEST(Observer, UnsubscribeStopsDelivery) {
    foundation::EventEmitter<int> emitter;
    int count = 0;
    auto tok = emitter.subscribe([&](const int&){ ++count; });
    emitter.emit(1);
    emitter.unsubscribe(tok);
    emitter.emit(2);
    EXPECT_EQ(count, 1);
}

// --- Self-registering Factory ---

TEST(Factory, CreateDog) {
    auto a = foundation::AnimalFactory::create("Dog");
    ASSERT_NE(a, nullptr);
    EXPECT_EQ(a->speak(), "Woof");
}

TEST(Factory, CreateCat) {
    auto a = foundation::AnimalFactory::create("Cat");
    ASSERT_NE(a, nullptr);
    EXPECT_EQ(a->speak(), "Meow");
}

TEST(Factory, UnknownReturnsNullptr) {
    auto a = foundation::AnimalFactory::create("Fish");
    EXPECT_EQ(a, nullptr);
}
