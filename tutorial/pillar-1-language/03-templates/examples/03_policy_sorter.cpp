// Compile: g++ -std=c++20 -Wall -Wextra -o /tmp/t09 03_policy_sorter.cpp
// Demonstrates: policy-based design — compile-time strategy selection, zero virtual overhead

#include <algorithm>
#include <cstdio>
#include <vector>

struct AscendingPolicy {
    template<typename T>
    static bool compare(const T& a, const T& b) { return a < b; }
};

struct DescendingPolicy {
    template<typename T>
    static bool compare(const T& a, const T& b) { return a > b; }
};

template<typename Policy>
class Sorter {
public:
    template<typename It>
    void sort(It first, It last) {
        std::sort(first, last, [](const auto& a, const auto& b){
            return Policy::compare(a, b);
        });
    }
};

void print_vec(const char* label, const std::vector<int>& v) {
    printf("%s: [", label);
    for (size_t i = 0; i < v.size(); ++i) {
        printf("%d%s", v[i], (i + 1 < v.size()) ? ", " : "");
    }
    printf("]\n");
}

int main() {
    std::vector<int> data = {5, 2, 8, 1, 9, 3};
    print_vec("Original", data);

    Sorter<AscendingPolicy> asc;
    asc.sort(data.begin(), data.end());
    print_vec("Ascending", data);

    Sorter<DescendingPolicy> desc;
    desc.sort(data.begin(), data.end());
    print_vec("Descending", data);

    // Policy is resolved at compile time — zero virtual overhead
    printf("\nsizeof(Sorter<AscendingPolicy>)  = %zu (no data members)\n",
           sizeof(Sorter<AscendingPolicy>));
    printf("sizeof(Sorter<DescendingPolicy>) = %zu (no data members)\n",
           sizeof(Sorter<DescendingPolicy>));
    return 0;
}
