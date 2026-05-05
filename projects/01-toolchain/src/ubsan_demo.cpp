#include <climits>
#include <cstdint>
#include <iostream>

// MODE: clean | overflow | shift
// UBSan detects undefined behavior at runtime.
// (nullptr deref intentionally omitted — it also triggers a signal)

int main(int argc, char* argv[]) {
    const char* mode = (argc > 1) ? argv[1] : "clean";
    std::string m{mode};

    if (m == "overflow") {
        // Signed integer overflow — undefined behavior in C++
        volatile int x = INT_MAX;
        std::cout << "INT_MAX + 1 = " << (x + 1) << "\n";  // UB
    } else if (m == "shift") {
        // Shift past bit-width — undefined behavior
        uint32_t x = 1u;
        std::cout << (x << 33) << "\n";  // UB: shift >= width
    } else {
        std::cout << "UBSan demo: no UB triggered.\n";
        std::cout << "Run with: overflow | shift\n";
    }
    return 0;
}
