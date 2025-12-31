#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include <type_traits>
#include <utility>

namespace Krys
{
  namespace Impl
  {
    template <typename T>
    struct TypeIdentity
    {
      using type = T;
    };
  }

  /// @brief A type trait that yields the type passed to it unchanged.
  template <typename T>
  using TypeIdentity = typename Impl::TypeIdentity<T>::type;

  /// @brief Selects one of two types based on a boolean condition.
  template <bool Test, typename TPass, typename TFail>
  using Conditional = std::conditional_t<Test, TPass, TFail>;

  /// @brief Removes const qualifier from the given type.
  template <typename T>
  using RemoveConst = typename std::remove_const<T>::type;

  /// @brief Removes volatile qualifier from the given type.
  template <typename T>
  using RemoveCv = typename std::remove_cv<T>::type;

  /// @brief Removes references (both l-value and r-value references) from the given type.
  template <typename T>
  using RemoveRef = typename std::remove_reference<T>::type;

  /// @brief Removes const, volatile, and references (both l-value and r-value references) from the given
  /// type.
  template <typename T>
  using RemoveCvRef = typename std::remove_cvref_t<T>;

  /// @brief Removes one level of array extent from the given type.
  template <typename T>
  using RemoveExtent = typename std::remove_extent<T>::type;

  /// @brief Retrieves the underlying type of an enumeration type.
  template <typename T>
  using UnderlyingType = typename std::underlying_type<T>::type;

  /// @brief Retrieves the size of the first dimension of an array type.
  template <typename T>
  constexpr auto Extent = std::extent<T>::value;

  /// @brief Applies l-value reference, r-value reference, and cv-qualifiers removal, as well as
  /// array-to-pointer decay to the given type.
  template <typename T>
  using Decay = typename std::decay_t<T>;

  /// @brief A type that is always false; useful for static assertions dependent on template parameters.
  template <typename T>
  using AlwaysFalse = std::false_type;

  /// @brief A type that is always true; useful for static assertions dependent on template parameters.
  template <typename T>
  using AlwaysTrue = std::true_type;

  /// @brief A boolean constant that is always false; useful for static assertions dependent on template
  /// parameters.
  template <typename T>
  constexpr bool DependentFalse = AlwaysFalse<T>::value;

  /// @brief A boolean constant that is always true; useful for static assertions dependent on template
  /// parameters.
  template <typename T>
  constexpr bool DependentTrue = AlwaysTrue<T>::value;

  /// @brief Determines whether the given type is an l-value reference.
  template <typename T>
  constexpr bool IsLValueRef = std::is_lvalue_reference<T>::value;

  /// @brief Check whether a given type is an `initializer_list`.
  template <typename T>
  constexpr bool IsInitializerList = std::is_same_v<T, std::initializer_list<typename T::value_type>>;

  /// @brief Gets the underlying integer value of an enumeration type.
  template <typename T>
  KRYS_NODISCARD constexpr auto ToUnderlying(T value) noexcept
  {
    return std::to_underlying<T>(value);
  }
}