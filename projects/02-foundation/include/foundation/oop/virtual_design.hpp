#pragma once

namespace foundation {

// ---- NVI: Non-Virtual Interface ----
// Public non-virtual functions define the invariant (pre/post hooks).
// Derived classes override private/protected virtual functions only.
class Reader {
public:
    // The NVI public interface — not virtual
    void read() {
        ++pre_count_;
        do_read();   // customization point
        ++post_count_;
    }

    int pre_count()  const noexcept { return pre_count_; }
    int post_count() const noexcept { return post_count_; }

    virtual ~Reader() = default;

protected:
    virtual void do_read() = 0;  // customization point

private:
    int pre_count_{0};
    int post_count_{0};
};

class LoggingReader : public Reader {
    int impl_count_{0};
protected:
    void do_read() override { ++impl_count_; }
public:
    int impl_count() const noexcept { return impl_count_; }
};

// ---- Virtual Inheritance: Diamond Problem ----
//
//       Base
//      /    \
//   Left    Right
//      \    /
//   DiamondDerived
//
// Without 'virtual': two Base subobjects → set_value() is ambiguous.
// With 'virtual':    one shared Base subobject → no ambiguity.

class Base {
    int value_{0};
public:
    void set_value(int v) noexcept { value_ = v; }
    int  get_value()      const noexcept { return value_; }
    virtual ~Base() = default;
};

class Left  : public virtual Base {};
class Right : public virtual Base {};

class DiamondDerived : public Left, public Right {
    // Single shared Base subobject — set_value/get_value unambiguous
};

} // namespace foundation
