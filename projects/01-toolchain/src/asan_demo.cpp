#include <cstdlib>
#include <iostream>
#include <vector>

// MODE selects which bug to trigger (pass as argv[1]):
//   clean  - no bug (default)
//   uaf    - heap-use-after-free
//   leak   - memory leak
//   oob    - out-of-bounds read

int main(int argc, char* argv[]) {
    const char* mode = (argc > 1) ? argv[1] : "clean";
    std::string m{mode};

    if (m == "uaf") {
        int* p = new int(42);
        delete p;
        std::cout << *p << "\n";  // USE AFTER FREE — ASan catches this
    } else if (m == "leak") {
        int* p = new int[1024];
        (void)p;
        // intentionally not deleted — LSan (part of ASan) catches this
    } else if (m == "oob") {
        std::vector<int> v{1, 2, 3};
        std::cout << v[10] << "\n";  // OUT OF BOUNDS — ASan catches this
    } else {
        std::cout << "ASan demo: no bug triggered.\n";
        std::cout << "Run with: uaf | leak | oob\n";
    }

    return 0;
}
