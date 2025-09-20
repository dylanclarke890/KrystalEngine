#pragma once

namespace Krys
{
  template <typename T>
  struct TypedBool
  {
    bool Value;

    constexpr TypedBool() noexcept : Value(false)
    {
    }

    constexpr TypedBool(bool value) noexcept : Value(value)
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
  };

  struct IsEnabled : TypedBool<IsEnabled>
  {
    explicit constexpr IsEnabled(bool value) noexcept : TypedBool<IsEnabled>(value)
    {
    }
  };
}