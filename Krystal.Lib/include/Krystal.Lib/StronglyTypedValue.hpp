#pragma once

#include "Krystal.Lib/Concepts.hpp"
#include <compare>
#include <xhash>

namespace Krys
{
  template <typename Derived>
  struct StronglyTypedBool
  {
    bool Value;

    constexpr StronglyTypedBool() noexcept : Value(false)
    {
    }

    explicit constexpr StronglyTypedBool(bool value) noexcept : Value(value)
    {
    }

    constexpr operator bool() const noexcept
    {
      return Value;
    }

    constexpr bool operator!() const noexcept
    {
      return !Value;
    }

    constexpr auto operator<=>(const StronglyTypedBool &other) const noexcept = default;
  };

  template <typename Derived>
  struct StronglyTypedTrue : public std::true_type
  {
    using std::true_type::true_type;
  };

  template <typename Derived>
  struct StronglyTypedFalse : public std::false_type
  {
    using std::false_type::false_type;
  };

  struct IsEnabled : public StronglyTypedBool<IsEnabled>
  {
  };

  struct IsDisabled : public StronglyTypedBool<IsDisabled>
  {
  };

  template <typename Derived, Number TNumber>
  struct StronglyTypedNumber
  {
    using Base = StronglyTypedNumber<Derived, TNumber>;

    TNumber Value;

    constexpr StronglyTypedNumber() noexcept : Value(0)
    {
    }

    explicit constexpr StronglyTypedNumber(TNumber value) noexcept : Value(value)
    {
    }

    constexpr operator TNumber() const noexcept
    {
      return Value;
    }

    constexpr auto operator<=>(const StronglyTypedNumber &other) const noexcept = default;
  };

  template <typename Derived>
  struct StronglyTypedBoolHasher
  {
    std::size_t operator()(const StronglyTypedBool<Derived> &value) const noexcept
    {
      return std::hash<bool> {}(value.Value);
    }
  };

  template <typename Derived>
  struct StronglyTypedNumberHasher
  {
    std::size_t operator()(const Derived::Base &value) const noexcept
    {
      return std::hash<bool> {}(value.Value);
    }
  };
}
