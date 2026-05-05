#include <iostream>
#include <thread>

// MODE: clean | race
// TSan detects the data race in "race" mode.

int shared_counter = 0;  // intentionally not atomic

void increment(int n) {
    for (int i = 0; i < n; ++i)
        ++shared_counter;  // DATA RACE: concurrent unsynchronized write
}

int main(int argc, char* argv[]) {
    const char* mode = (argc > 1) ? argv[1] : "clean";
    std::string m{mode};

    if (m == "race") {
        shared_counter = 0;
        std::thread t1(increment, 100'000);
        std::thread t2(increment, 100'000);
        t1.join();
        t2.join();
        std::cout << "Final counter: " << shared_counter
                  << " (expected 200000 — will differ due to race)\n";
    } else {
        std::cout << "TSan demo: no race triggered.\n";
        std::cout << "Run with: race\n";
    }
    return 0;
}
