#pragma once

#include <type_traits>

namespace Krys
{
  template <bool Test, typename TPass, typename TFail>
  using Conditional = std::conditional_t<Test, TPass, TFail>;

  template <typename T>
  using RemoveConst = typename std::remove_const<T>::type;

  template <typename T>
  using RemoveCv = typename std::remove_cv<T>::type;

  template <typename T>
  using RemoveRef = typename std::remove_reference<T>::type;

  template <typename T>
  using RemoveCvRef = typename std::remove_cvref_t<T>;

  template <typename T>
  using RemoveExtent = typename std::remove_extent<T>::type;

  template <typename T>
  constexpr auto Extent = std::extent<T>::value;

  template <typename T>
  using Decay = typename std::decay_t<T>;

  template <typename T>
  using AlwaysFalse = std::false_type;

  template <typename T>
  using AlwaysTrue = std::true_type;

  template <typename T>
  constexpr bool DependentFalse = AlwaysFalse<T>::value;

  template <typename T>
  constexpr bool DependentTrue = AlwaysTrue<T>::value;

  template <typename T>
  constexpr bool IsLValueRef = std::is_lvalue_reference<T>::value;
}