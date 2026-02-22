#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Utils/Hijack.hpp"
#include "Krystal.Lib/Pointers/ReferenceWrapper.hpp"
#include "Krystal.Lib/Utils/ToAddress.hpp"
#include <iterator>
#include <utility>

namespace Krys
{
  /// @brief Test whether a type can have `Unwrap(...)` called on it.
  template <typename T>
  concept UnwrappableValue = requires(T value) { Unwrap(value); };

  /// @brief Test whether a type can have `UnwrapIteratorValue(...)` called on it.
  template <typename T>
  concept UnwrappableIteratorValue = requires(T value) { UnwrapIteratorValue(value); };

  /// @brief Test whether a type can have `UnwrapIterator(...)` called on it.
  template <typename T>
  concept UnwrappableIterator = requires(T value) { UnwrapIterator(value); };

  namespace Impl
  {
    class UnwrapFn : public Hijack::Token<UnwrapFn>, public ::KrysHijackGlobalToken<UnwrapFn>
    {
    public:
      template <typename T>
      constexpr decltype(auto) operator()(T &&value) const noexcept
      {
        if constexpr (::Krys::IsSpecializationOf<remove_cvref_t<T>, ReferenceWrapper>)
        {
          return value.get();
        }
        else if constexpr (UnwrappableValue<T>)
        {
          return Unwrap(std::forward<T>(value));
        }
        else
        {
          return std::forward<T>(value);
        }
      }
    };

    class UnwrapIteratorFn : public Hijack::Token<UnwrapIteratorFn>,
                             public ::KrysHijackGlobalToken<UnwrapIteratorFn>
    {
    public:
      template <typename T>
      constexpr decltype(auto) operator()(T &&value) const noexcept
      {
        if constexpr (::Krys::IsSpecializationOf<remove_cvref_t<T>, std::basic_const_iterator>)
        {
          // peel off the const iterator where possible
          return std::forward<T>(value).base();
        }
        else
        {
          return std::forward<T>(value);
        }
      }
    };

    class UnwrapIteratorValueFn : public Hijack::Token<UnwrapIteratorValueFn>,
                                  public ::KrysHijackGlobalToken<UnwrapIteratorValueFn>
    {
    public:
      template <typename T>
      constexpr decltype(auto) operator()(T &&value) const noexcept
      {
        if constexpr (UnwrappableIteratorValue<T>)
        {
          return UnwrapIteratorValue(std::forward<T>(value));
        }
        else
        {
          return Unwrap(*std::forward<T>(value));
        }
      }
    };
  }

  /// @brief Unwraps a value, if possible. Otherwise, simply forwards the input value through.
  /// @returns The unwrapped value.
  inline constexpr Impl::UnwrapFn Unwrap = {};

  /// @brief Peels layers off of an iterator that may be wrapped in implementation-defined and
  /// standards-defined abstraction layers.
  /// @returns The iterator's unwrapped value.
  inline constexpr Impl::UnwrapIteratorFn UnwrapIterator = {};

  /// @brief Unwraps either an iterator, or unwraps the value and returns its address, or forwards the input
  /// value through.
  /// @returns The iterator's unwrapped value.
  inline constexpr Impl::UnwrapIteratorValueFn UnwrapIteratorValue = {};

  /// @brief Retrives the unwrapped type if the object were put through a call to Unwrap.
  /// @remarks Typically used to get the type underlying a `std::reference_wrapper` or similar.
  template <typename T>
  using unwrap_t = decltype(Unwrap(std::declval<T>()));

  /// @brief Retrives the unwrapped type if the object were put through a call to Unwrap.
  /// @remarks Typically used to get the type underlying a `std::reference_wrapper` or similar.
  template <typename T>
  using unwrap_remove_cvref_t = remove_cvref_t<unwrap_t<T>>;

  /// @brief Retrives the unwrapped type if the object were put through a call to Unwrap.
  /// @remarks Typically used to get the type underlying a `std::reference_wrapper` or similar.
  template <typename T>
  using unwrap_remove_ref_t = remove_ref_t<unwrap_t<T>>;

  /// @brief Retrives the unwrapped type if the object were put through a call to UnwrapIterator.
  /// @remarks Typically used to peel off various layers of implementation-defiend and other boilerplate
  /// around iterator types.
  template <typename T>
  using unwrap_iterator_t = decltype(UnwrapIterator(std::declval<T>()));

  /// @brief Retrives the unwrapped type if the object were put through a call to UnwrapIteratorValue.
  /// @remarks Typically used to get the type underlying an iterator whose value may be wrapped up in certain
  /// abstractions.
  template <typename T>
  using unwrap_iterator_value_t = decltype(UnwrapIteratorValue(std::declval<T>()));
}
