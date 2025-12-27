#pragma once

#include <concepts>

namespace Krys
{
  template <typename T>
  concept Integral = std::is_integral_v<T>;

  template <typename T>
  concept FloatingPoint = std::is_floating_point_v<T>;

  template <typename T>
  concept SignedIntegral = std::is_signed_v<T> && Integral<T>;

  template <typename T>
  concept UnsignedIntegral = std::is_unsigned_v<T> && Integral<T>;

  template <typename T>
  concept Number = SignedIntegral<T> || UnsignedIntegral<T> || FloatingPoint<T>;

  template <typename T>
  concept Arithmetic = std::is_arithmetic_v<T>;

  template <typename T>
  concept Pointer = std::is_pointer_v<T>;

  template <typename F, class... Args>
  concept Callable = std::invocable<F, Args...>;

  /// @brief Checks type `T` is callable type that does not cause side effects.
  template <typename T, class... Args>
  concept RegularCallable = std::regular_invocable<T, Args...>;

  template <typename T, class... Args>
  concept Predicate = std::predicate<T, Args...>;

  template <typename T, typename U>
  concept ConvertibleTo = std::convertible_to<T, U>;

  template <typename T>
  concept DefaultConstructible = std::is_default_constructible_v<T>;

  template <typename T>
  concept Destructible = std::is_destructible_v<T>;

  template <typename T>
  concept CopyConstructible = std::is_copy_constructible_v<T>;

  template <typename T>
  concept CopyAssignable = std::is_copy_constructible_v<T>;

  template <typename T>
  concept MoveConstructible = std::is_move_constructible_v<T>;

  template <typename T>
  concept MoveAssignable = std::is_move_assignable_v<T>;

  template <typename D, typename B>
  concept DerivedFrom = std::derived_from<D, B>;

  template <typename T, typename U>
  concept SameType = std::is_same_v<T, U>;

  template <typename T, typename... Types>
  concept OneOf = std::disjunction_v<std::is_same<T, Types>...>;

  template <typename T>
  concept IsEnum = std::is_enum_v<T>;

  template <typename T>
  concept IsScopedEnum = std::is_scoped_enum_v<T>;

  template <typename T>
  concept IsArray = std::is_array_v<T>;

  template <typename T>
  concept HasStandardLayout = std::is_standard_layout_v<T>;

  template <typename T>
  concept EqualityComparable = requires(T a, T b) {
    { a == b } -> std::convertible_to<bool>;
  };

  template <typename T>
  concept InEqualityComparable = requires(T a, T b) {
    { a == b } -> std::convertible_to<bool>;
  };

  template <typename T>
  concept LessThanComparable = requires(T a, T b) {
    { a < b } -> std::convertible_to<bool>;
  };
  
  template <typename T>
  concept GreaterThanComparable = requires(T a, T b) {
    { a > b } -> std::convertible_to<bool>;
  };
}