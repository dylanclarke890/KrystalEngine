#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

namespace Krys
{
  /// @brief A compile-time constant representing a value of type T.
  template <typename T, T Value>
  using IntegralConstant = std::integral_constant<T, Value>;

  /// @brief A compile-time constant representing a boolean value.
  template <bool Value>
  using BoolConstant = IntegralConstant<bool, Value>;

  /// @brief A type that is always false; useful for static assertions dependent on template parameters.
  template <typename T>
  using FalseType = BoolConstant<false>;

  /// @brief A type that is always true; useful for static assertions dependent on template parameters.
  template <typename T>
  using TrueType = BoolConstant<true>;

  /// @brief A boolean constant that is always false; useful for static assertions dependent on template
  /// parameters.
  template <typename T>
  constexpr bool DependentFalse = FalseType<T>::value;

  /// @brief A boolean constant that is always true; useful for static assertions dependent on template
  /// parameters.
  template <typename T>
  constexpr bool DependentTrue = TrueType<T>::value;

  template <bool Value, typename... Args>
  constexpr bool DependentBool = Value;

  /// @brief Retrieves the size of the first dimension of an array type.
  template <typename T>
  constexpr auto Extent = std::extent<T>::value;

  template <typename T>
  constexpr auto AlignmentOf = std::alignment_of<T>::value;

  template <bool Test, typename T = void>
  using enable_if_t = typename std::enable_if_t<Test, T>;

  /// @brief A helper type for SFINAE that yields `void` for any type passed to it.
  template <typename T>
  using void_t = typename std::void_t<T>;

  /// @brief Selects one of two types based on a boolean condition.
  template <bool Test, typename TPass, typename TFail>
  using conditional_t = typename std::conditional_t<Test, TPass, TFail>;

  template <typename T>
  using add_pointer_t = typename std::add_pointer<T>::type;

  template <typename T>
  using add_lvalue_ref_t = typename std::add_lvalue_reference<T>::type;

  template <typename T>
  using add_const_t = typename std::add_const<T>::type;

  template <typename T>
  using add_rvalue_ref_t = typename std::add_rvalue_reference<T>::type;

  /// @brief Removes const qualifier from the given type.
  template <typename T>
  using remove_const_t = typename std::remove_const<T>::type;

  /// @brief Removes volatile qualifier from the given type.
  template <typename T>
  using remove_cv_t = typename std::remove_cv<T>::type;

  /// @brief Removes references (both l-value and r-value references) from the given type.
  template <typename T>
  using remove_ref_t = typename std::remove_reference<T>::type;

  /// @brief Removes const, volatile, and references (both l-value and r-value references) from the given
  /// type.
  template <typename T>
  using remove_cvref_t = typename std::remove_cvref_t<T>;

  /// @brief Removes one level of array extent from the given type.
  template <typename T>
  using remove_extent_t = typename std::remove_extent<T>::type;

  template <typename T>
  using remove_pointer_t = typename std::remove_pointer<T>::type;

  /// @brief Retrieves the underlying type of an enumeration type.
  template <typename T>
  using underlying_t = typename std::underlying_type<T>::type;

  /// @brief Applies l-value reference, r-value reference, and cv-qualifiers removal, as well as
  /// array-to-pointer decay to the given type.
  template <typename T>
  using decay_t = typename std::decay_t<T>;

  /// @brief Converts an integral type to its signed counterpart.
  template <typename T>
  using make_signed_t = typename std::make_signed<T>::type;

  /// @brief Converts an integral type to its unsigned counterpart.
  template <typename T>
  using make_unsigned_t = typename std::make_unsigned<T>::type;

  // Based on 'Detecting in C++ whether a type is defined, part 3: SFINAE and incomplete types'
  // <https://devblogs.microsoft.com/oldnewthing/20190710-00/?p=102678>
  template <typename, typename = void>
  constexpr bool IsTypeComplete = false;

  template <typename T>
  constexpr bool IsTypeComplete<T, void_t<decltype(sizeof(T))>> = true;

  template <bool... Bs>
  constexpr bool AllTrue =
    std::is_same<std::integer_sequence<bool, true, Bs...>, std::integer_sequence<bool, Bs..., true>>::value;

  template <std::size_t>
  struct SizedUnsignedTrait;

  template <>
  struct SizedUnsignedTrait<1uz>
  {
    using type = uint8_t;
  };

  template <>
  struct SizedUnsignedTrait<2uz>
  {
    using type = uint16_t;
  };

  template <>
  struct SizedUnsignedTrait<4uz>
  {
    using type = uint32_t;
  };

  template <>
  struct SizedUnsignedTrait<8uz>
  {
    using type = uint64_t;
  };

  template <std::size_t N>
  using sized_unsigned_t = SizedUnsignedTrait<N>::type;
}