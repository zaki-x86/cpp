#include <gtest/gtest.h>
#include <foundation/concurrency/lock_free_queue.hpp>
#include <foundation/concurrency/thread_pool.hpp>
#include <foundation/concurrency/coroutine_task.hpp>
#include <atomic>
#include <thread>
#include <vector>
#include <future>
#include <numeric>

// --- SPSCQueue ---

TEST(SPSCQueue, PushAndPop) {
    foundation::SPSCQueue<int, 8> q;
    EXPECT_TRUE(q.push(10));
    int out = 0;
    EXPECT_TRUE(q.pop(out));
    EXPECT_EQ(out, 10);
}

TEST(SPSCQueue, EmptyOnStart) {
    foundation::SPSCQueue<int, 4> q;
    int out = 0;
    EXPECT_FALSE(q.pop(out));
    EXPECT_TRUE(q.empty());
}

TEST(SPSCQueue, FullReturnsFalse) {
    foundation::SPSCQueue<int, 4> q;  // capacity = 3 (N-1)
    EXPECT_TRUE(q.push(1));
    EXPECT_TRUE(q.push(2));
    EXPECT_TRUE(q.push(3));
    EXPECT_FALSE(q.push(4));  // full
}

TEST(SPSCQueue, ProducerConsumerSingleThread) {
    foundation::SPSCQueue<int, 16> q;
    for (int i = 0; i < 10; ++i) q.push(i);
    for (int i = 0; i < 10; ++i) {
        int v = -1;
        ASSERT_TRUE(q.pop(v));
        EXPECT_EQ(v, i);
    }
}

TEST(SPSCQueue, ConcurrentProducerConsumer) {
    foundation::SPSCQueue<int, 1024> q;
    constexpr int N = 500;
    std::atomic<int> sum{0};

    std::thread producer([&]{
        for (int i = 0; i < N; ++i) {
            while (!q.push(i)) {}
        }
    });

    std::thread consumer([&]{
        int received = 0;
        while (received < N) {
            int v;
            if (q.pop(v)) { sum += v; ++received; }
        }
    });

    producer.join();
    consumer.join();

    int expected = N * (N - 1) / 2;
    EXPECT_EQ(sum.load(), expected);
}

// --- ThreadPool ---

TEST(ThreadPool, SubmitSingleTask) {
    foundation::ThreadPool pool(2);
    auto fut = pool.submit([]{ return 42; });
    EXPECT_EQ(fut.get(), 42);
}

TEST(ThreadPool, ParallelSum) {
    foundation::ThreadPool pool(4);
    constexpr int N = 100;
    std::vector<std::future<int>> futures;
    futures.reserve(N);
    for (int i = 0; i < N; ++i)
        futures.push_back(pool.submit([i]{ return i; }));
    int total = 0;
    for (auto& f : futures) total += f.get();
    EXPECT_EQ(total, N * (N - 1) / 2);
}

TEST(ThreadPool, ExceptionPropagates) {
    foundation::ThreadPool pool(1);
    auto fut = pool.submit([]() -> int {
        throw std::runtime_error("task error");
        return 0;
    });
    EXPECT_THROW(fut.get(), std::runtime_error);
}

// --- Coroutine Task ---

TEST(CoroutineTask, AsyncSquare) {
    auto t = foundation::async_square(7);
    EXPECT_TRUE(t.done());
    EXPECT_EQ(t.result(), 49);
}

TEST(CoroutineTask, EagerExecution) {
    int side_effect = 0;
    auto make_task = [&]() -> foundation::Task<int> {
        side_effect = 1;
        co_return 99;
    };
    auto t = make_task();
    // Eager: runs synchronously on creation before result() is called
    EXPECT_EQ(side_effect, 1);
    EXPECT_EQ(t.result(), 99);
}

TEST(CoroutineTask, DoneAfterReturn) {
    auto t = foundation::async_square(3);
    EXPECT_TRUE(t.done());
}
