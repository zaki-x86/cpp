#pragma once
#include <type_traits>

namespace foundation {

// ---- remove_cvref (mirrors C++20 std::remove_cvref, reimplemented for pedagogy) ----
template<typename T>
struct remove_cvref { using type = std::remove_cv_t<std::remove_reference_t<T>>; };

template<typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

// ---- Compile-time type list ----
template<typename... Ts>
struct TypeList {
    static constexpr std::size_t size = sizeof...(Ts);
};

// type_at<List, I>: get the I-th type
template<typename List, std::size_t I> struct type_at;

template<typename Head, typename... Tail, std::size_t I>
struct type_at<TypeList<Head, Tail...>, I> : type_at<TypeList<Tail...>, I - 1> {};

template<typename Head, typename... Tail>
struct type_at<TypeList<Head, Tail...>, 0> { using type = Head; };

template<typename List, std::size_t I>
using type_at_t = typename type_at<List, I>::type;

// type_list_contains<List, T>: membership check
template<typename List, typename T> struct type_list_contains : std::false_type {};

template<typename Head, typename... Tail, typename T>
struct type_list_contains<TypeList<Head, Tail...>, T>
    : std::conditional_t<std::is_same_v<Head, T>,
                         std::true_type,
                         type_list_contains<TypeList<Tail...>, T>> {};

template<typename List, typename T>
inline constexpr bool type_list_contains_v = type_list_contains<List, T>::value;

// ---- void_t (mirrors C++17, re-implemented for pedagogy) ----
template<typename...> using void_t = void;

// ---- has_size_member: SFINAE detection example ----
template<typename T, typename = void>
struct has_size : std::false_type {};

template<typename T>
struct has_size<T, void_t<decltype(std::declval<T>().size())>> : std::true_type {};

template<typename T>
inline constexpr bool has_size_v = has_size<T>::value;

// ---- Compile-time integer power ----
template<long long Base, unsigned Exp>
struct pow_ct {
    static constexpr long long value = Base * pow_ct<Base, Exp - 1>::value;
};
template<long long Base>
struct pow_ct<Base, 0> { static constexpr long long value = 1; };

template<long long Base, unsigned Exp>
inline constexpr long long pow_ct_v = pow_ct<Base, Exp>::value;

} // namespace foundation
