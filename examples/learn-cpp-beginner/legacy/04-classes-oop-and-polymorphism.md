# Classes, OOP, and Polymorphism (Beginner)

Object-oriented programming in C++ combines **data** and **behavior** in `class` types, **encapsulation** via access control, and **polymorphism** when you need to swap implementations behind one interface.

## 1. Class basics

```cpp
#include <string>

class Counter {
public:
    explicit Counter(int start = 0) : value_{start} {}

    void inc() { ++value_; }
    int get() const { return value_; }   // const method: won't mutate *this

private:
    int value_;
};
```

- **`public`:** users of the class can access.
- **`private`:** only member functions can access — **encapsulation**.
- **`explicit`** on constructors avoids accidental implicit conversions.

---

## 2. Special member functions (what the compiler generates)

For a simple class with only plain members, the compiler can generate:

- Default constructor (if nothing prevents it)
- Destructor
- Copy/move constructors and assignments

When you add a **raw resource** (pointer owning memory), defaults may become wrong — prefer smart pointers to stay simple.

---

## 3. `const` correctness

```cpp
class Buffer {
public:
    std::size_t size() const { return data_.size(); }  // won't modify object
    void clear() { data_.clear(); }                    // modifies
private:
    std::vector<int> data_;
};
```

**Best practice:** mark every member function `const` if it logically does not change observable state.

---

## 4. Inheritance (is-a)

```cpp
struct Shape {
    virtual double area() const = 0;   // pure virtual — Shape is abstract
    virtual ~Shape() = default;        // virtual destructor — important!
};

struct Circle : Shape {
    explicit Circle(double r) : r_{r} {}
    double area() const override { return 3.1415926535 * r_ * r_; }
private:
    double r_;
};
```

**`virtual`:** dynamic dispatch — which `area()` runs is chosen at **runtime** based on object type.

**`override`:** catches signature mistakes at compile time.

**Virtual destructor:** deleting through base pointer must call derived destructor.

```cpp
Shape* s = new Circle{1.0};
delete s;  // must be safe — requires virtual ~Shape
```

---

## 5. Slicing (beginner trap)

```cpp
Circle c{2.0};
Shape s = c;   // BAD idea if Shape were concrete — “slices” off Circle part
```

With **abstract** `Shape` this won’t compile; the real trap is copying polymorphic objects by value. Use **`unique_ptr<Shape>`** or references.

---

## 6. NVI (Non-Virtual Interface) — pattern sketch

Public non-virtual function calls private virtual “hook”:

```cpp
class Reader {
public:
    int read() {          // template method
        before();
        int x = do_read();
        after();
        return x;
    }
    virtual ~Reader() = default;
private:
    virtual int do_read() = 0;
    void before() { /* logging */ }
    void after()  { /* logging */ }
};
```

**Connect:** `virtual_design.hpp` in foundation.

---

## 7. CRTP (curious recurring template pattern) — one-line intuition

Static polymorphism: derive from a base that is templated on the derived type, so calls can inline without vtables.

**When:** hot paths, generic algorithms. **Cost:** harder to read.

**Connect:** `crtp.hpp` in foundation — read only after templates chapter.

---

## 8. `enum class` (strong enums)

```cpp
enum class Color { Red, Green, Blue };
Color c = Color::Red;
```

Safer than old `enum` — no implicit conversion to `int`.

---

## 9. Operator overloading (sanity rules)

You can define `operator+`, `operator==`, etc. **Symmetry tip:** prefer **free functions** for binary ops if both sides should convert equally.

```cpp
struct Vec2 { double x, y; };

Vec2 operator+(Vec2 a, Vec2 b) { return {a.x + b.x, a.y + b.y}; }
bool operator==(Vec2 a, Vec2 b) { return a.x == b.x && a.y == b.y; }
```

---

## 10. Step-by-step: tiny polymorphic drawing API

1. Define abstract `Shape` with `virtual double area() const = 0;`.
2. Implement `Circle`, `Rectangle`.
3. Store shapes in `std::vector<std::unique_ptr<Shape>>`.
4. Sum areas in a loop.

```cpp
#include <memory>
#include <vector>

double total_area(const std::vector<std::unique_ptr<Shape>>& shapes) {
    double s = 0.0;
    for (const auto& p : shapes) {
        s += p->area();
    }
    return s;
}
```

---

## 11. Best practices

1. **Public API** small; **private** helpers.
2. **Virtual destructor** if any virtual methods.
3. **Avoid deep inheritance trees**; prefer composition.
4. **Smart pointers** for owned polymorphic objects.
5. **`override`** on every virtual override.

## Connect to this repo

- `projects/02-foundation/docs/oop.md`
- Demos: `demo_oop.cpp`

---

*Next:* [05-templates-and-concepts-intro.md](05-templates-and-concepts-intro.md)
