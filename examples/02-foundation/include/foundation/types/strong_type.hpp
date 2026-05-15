#pragma once
#include <compare>
#include <ostream>
#include <utility>
#include <type_traits>

namespace foundation {

// StrongType<T, Tag>: named wrapper — different tags = incompatible types.
// Prevents silent int/double aliasing errors at compile time.
template<typename T, typename Tag>
class StrongType {
    T value_;
public:
    explicit constexpr StrongType(T v) : value_{std::move(v)} {}
    constexpr T  value() const noexcept { return value_; }
    constexpr T& value()       noexcept { return value_; }

    constexpr auto operator<=>(const StrongType&) const = default;

    friend std::ostream& operator<<(std::ostream& os, const StrongType& st) {
        return os << st.value_;
    }
};

// Phantom types: encode state in the type system
// — prevents calling Verified functions on Unverified data
struct Unverified {};
struct Verified   {};

template<typename State = Unverified>
class UserInput {
    std::string raw_;
    explicit UserInput(std::string s) : raw_{std::move(s)} {}
    template<typename S> friend class UserInput;
public:
    static UserInput<Unverified> from_raw(std::string s) {
        return UserInput<Unverified>{std::move(s)};
    }

    // Can only call verify() on an Unverified input — compiler error otherwise
    UserInput<Verified> verify() const
        requires std::is_same_v<State, Unverified>
    {
        // real validation would go here
        return UserInput<Verified>{raw_};
    }

    const std::string& get() const { return raw_; }
};

// Pre-defined strong types
using Kilometers = StrongType<double, struct KilometersTag>;
using Seconds    = StrongType<double, struct SecondsTag>;
using Meters     = StrongType<double, struct MetersTag>;

// User-defined literals
namespace literals {
    Kilometers operator""_km(long double v) { return Kilometers{static_cast<double>(v)}; }
    Seconds    operator""_s (long double v) { return Seconds   {static_cast<double>(v)}; }
    Meters     operator""_m (long double v) { return Meters    {static_cast<double>(v)}; }
}

} // namespace foundation
