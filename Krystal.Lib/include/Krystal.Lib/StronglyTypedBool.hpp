#pragma once

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
}