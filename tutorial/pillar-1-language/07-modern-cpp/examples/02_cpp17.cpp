// Chapter 07 — Example 2: C++17 Features
// Compile: g++ -std=c++20 -Wall -Wextra -o /tmp/t21
//   tutorial/pillar-1-language/07-modern-cpp/examples/02_cpp17.cpp && /tmp/t21

#include <any>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

using namespace std::string_literals;

// ============================================================
// === Demo: structured bindings ===
// ============================================================
struct Point3D { int x; int y; int z; };  // aggregate

static void demo_structured_bindings() {
    printf("=== Demo: structured bindings ===\n");

    // Pair decomposition
    std::pair<int, std::string> entry{42, "answer"};
    auto [num, word] = entry;
    printf("  pair: num=%d, word=%s\n", num, word.c_str());

    // Aggregate struct decomposition
    Point3D p{1, 2, 3};
    auto [px, py, pz] = p;
    printf("  Point3D: %d %d %d\n", px, py, pz);

    // Map range-for — the idiomatic use case
    std::map<std::string, int> scores{{"alice", 95}, {"bob", 87}, {"carol", 91}};
    printf("  map:\n");
    for (const auto& [name, score] : scores) {
        printf("    %s -> %d\n", name.c_str(), score);
    }

    // Also works with set::insert return value
    auto [it, inserted] = scores.emplace("dave", 78);
    printf("  inserted dave: %s, score=%d\n\n", inserted ? "yes" : "no", it->second);
}

// ============================================================
// === Demo: if constexpr ===
// ============================================================
template<typename T>
static std::string describe_type(T val) {
    if constexpr (std::is_integral_v<T>) {
        return "integer: " + std::to_string(static_cast<long long>(val));
    } else if constexpr (std::is_floating_point_v<T>) {
        return "floating point: " + std::to_string(val);
    } else {
        // This branch is NOT instantiated for integral or fp types,
        // so val.c_str() would not be an error even for T=int:
        return "other";
    }
}

// Explicit instantiation for std::string to test the "other" branch
template<>
std::string describe_type<std::string>(std::string val) {
    return "other (string): " + val;
}

static void demo_if_constexpr() {
    printf("=== Demo: if constexpr ===\n");
    printf("  describe(42)    = %s\n", describe_type(42).c_str());
    printf("  describe(3.14)  = %s\n", describe_type(3.14).c_str());
    printf("  describe(\"hi\")  = %s\n\n", describe_type(std::string{"hi"}).c_str());
}

// ============================================================
// === Demo: std::optional — chain of transformations ===
// ============================================================

// Simulated user database
struct User { std::string name; std::string email; };
static std::map<int, User> user_db{
    {1, {"Alice", "alice@example.com"}},
    {2, {"Bob",   "bob@corp.org"}},
};

static std::optional<User> find_user(int id) {
    auto it = user_db.find(id);
    if (it == user_db.end()) return std::nullopt;
    return it->second;
}

static std::optional<std::string> get_email(const User& u) {
    if (u.email.empty()) return std::nullopt;
    return u.email;
}

static std::optional<std::string> get_domain(const std::string& email) {
    auto pos = email.find('@');
    if (pos == std::string::npos) return std::nullopt;
    return email.substr(pos + 1);
}

static void demo_optional() {
    printf("=== Demo: std::optional (chain of optional transformations) ===\n");

    for (int id : {1, 2, 99}) {
        // Manual chaining (C++17 — monadic .and_then is C++23)
        std::optional<std::string> domain;
        if (auto user = find_user(id)) {
            if (auto email = get_email(*user)) {
                domain = get_domain(*email);
            }
        }

        if (domain) {
            printf("  id=%d -> domain=%s\n", id, domain->c_str());
        } else {
            printf("  id=%d -> not found or no domain\n", id);
        }
    }

    // value_or for default (manual transform — .transform() is C++23, not C++20 GCC 11)
    std::optional<std::string> fallback_name;
    if (auto u = find_user(99)) fallback_name = u->name;
    printf("  unknown user name: %s\n\n",
           fallback_name.value_or("(unknown)").c_str());
}

// ============================================================
// === Demo: std::variant + overloaded visitor ===
// ============================================================
struct Circle    { double radius; };
struct Rectangle { double width; double height; };

using Shape = std::variant<Circle, Rectangle>;

// Overloaded helper — combines multiple lambdas into one visitor
template<typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
// Deduction guide (not needed in C++20 but kept for clarity)
template<typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

static double area(const Shape& s) {
    return std::visit(overloaded{
        [](const Circle& c)    { return 3.14159 * c.radius * c.radius; },
        [](const Rectangle& r) { return r.width * r.height; }
    }, s);
}

static void demo_variant() {
    printf("=== Demo: std::variant + overloaded visitor ===\n");

    std::vector<Shape> shapes{
        Circle{5.0},
        Rectangle{3.0, 4.0},
        Circle{1.0},
    };

    for (const auto& s : shapes) {
        std::visit(overloaded{
            [](const Circle& c)    { printf("  Circle(r=%.1f)", c.radius); },
            [](const Rectangle& r) { printf("  Rect(%.1fx%.1f)", r.width, r.height); }
        }, s);
        printf(" -> area=%.2f\n", area(s));
    }
    printf("\n");
}

// ============================================================
// === Demo: std::string_view ===
// ============================================================
static std::size_t count_words(std::string_view text) {
    // Zero-copy — text is a non-owning reference
    std::size_t count = 0;
    bool in_word = false;
    for (char c : text) {
        if (c == ' ' || c == '\t' || c == '\n') {
            in_word = false;
        } else if (!in_word) {
            ++count;
            in_word = true;
        }
    }
    return count;
}

static void demo_string_view() {
    printf("=== Demo: std::string_view (zero-copy) ===\n");

    const char* literal    = "the quick brown fox";
    std::string owned      = "the quick brown fox jumps";

    printf("  count_words(literal) = %zu\n", count_words(literal));
    printf("  count_words(owned)   = %zu\n", count_words(owned));

    // Substring view — no allocation
    std::string_view sv{owned};
    std::string_view first_three = sv.substr(0, 9);  // "the quick"
    printf("  first 9 chars: '%.*s'\n", (int)first_three.size(), first_three.data());

    // compare without allocation
    std::string_view a = "hello";
    std::string_view b = "hello";
    printf("  a == b (string_view compare): %s\n\n", (a == b) ? "true" : "false");
}

// ============================================================
// === Demo: fold expressions ===
// ============================================================
template<typename... Ts>
auto sum_all(Ts... args) {
    return (args + ...);   // unary right fold
}

template<typename... Ts>
auto product_all(Ts... args) {
    return (... * args);   // unary left fold
}

template<typename... Ts>
bool all_positive(Ts... args) {
    return ((args > 0) && ...);  // fold with &&
}

static void demo_fold_expressions() {
    printf("=== Demo: fold expressions ===\n");
    printf("  sum(1,2,3,4,5)       = %d\n",   sum_all(1, 2, 3, 4, 5));
    printf("  product(1,2,3,4,5)   = %d\n",   product_all(1, 2, 3, 4, 5));
    printf("  all_positive(1,2,3)  = %s\n",   all_positive(1, 2, 3) ? "true" : "false");
    printf("  all_positive(1,-1,3) = %s\n\n", all_positive(1, -1, 3) ? "true" : "false");
}

// ============================================================
// === Demo: CTAD (Class Template Argument Deduction) ===
// ============================================================
static void demo_ctad() {
    printf("=== Demo: CTAD (C++17 class template argument deduction) ===\n");

    // std::pair — no need to write std::pair<int, std::string>
    std::pair p{42, "hello"s};
    static_assert(std::is_same_v<decltype(p), std::pair<int, std::string>>);
    printf("  pair<int,string>: %d, %s\n", p.first, p.second.c_str());

    // std::vector — deduced from initializer list
    std::vector v{1, 2, 3, 4, 5};
    static_assert(std::is_same_v<decltype(v), std::vector<int>>);
    printf("  vector<int> size: %zu\n", v.size());

    // std::map — deduced from initializer list
    std::map m{std::pair{1, "one"s}, std::pair{2, "two"s}};
    static_assert(std::is_same_v<decltype(m), std::map<int, std::string>>);
    printf("  map<int,string>[2]: %s\n\n", m[2].c_str());
}

// ============================================================
int main() {
    printf("=== Chapter 07: C++17 Features ===\n\n");

    demo_structured_bindings();
    demo_if_constexpr();
    demo_optional();
    demo_variant();
    demo_string_view();
    demo_fold_expressions();
    demo_ctad();

    printf("All demos complete.\n");
    return 0;
}
