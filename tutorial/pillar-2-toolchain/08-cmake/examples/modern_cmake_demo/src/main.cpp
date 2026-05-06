#include "demo/math.hpp"
#include <array>
#include <iostream>

int main() {
    // GCD and LCM
    std::cout << "gcd(48, 18) = " << demo::gcd(48, 18) << "\n";  // 6
    std::cout << "lcm(4, 6)   = " << demo::lcm(4, 6)   << "\n";  // 12

    // Dot product
    std::array<double, 3> a{1.0, 2.0, 3.0};
    std::array<double, 3> b{4.0, 5.0, 6.0};
    std::cout << "dot([1,2,3],[4,5,6]) = "
              << demo::dot<double>(a, b) << "\n";  // 32

    // Clamp
    std::cout << "clamp(150, 0, 100) = "
              << demo::clamp(150, 0, 100) << "\n";  // 100

    return 0;
}
