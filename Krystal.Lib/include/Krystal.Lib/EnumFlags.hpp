#pragma once

#include "Krystal.Lib/Concepts.hpp"

namespace Krys
{
  template <IsEnum TEnum>
  class Flags
  {
  private:
    T value;

  public:
    using UnderlyingType = std::underlying_type_t<TEnum>;

    constexpr Flags() noexcept : value(0)
    {
    }

    constexpr Flags(T flag) noexcept : value(static_cast<UnderlyingType>(flag))
    {
    }

    constexpr Flags(UnderlyingType val) noexcept : value(val)
    {
    }

    constexpr Flags &operator|=(T flag) noexcept
    {
      value |= static_cast<UnderlyingType>(flag);
      return *this;
    }

    constexpr Flags operator|(T flag) const noexcept
    {
      return Flags(value | static_cast<UnderlyingType>(flag));
    }

    constexpr Flags &operator&=(T flag) noexcept
    {
      value &= static_cast<UnderlyingType>(flag);
      return *this;
    }

    constexpr Flags operator&(T flag) const noexcept
    {
      return Flags(value & static_cast<UnderlyingType>(flag));
    }

    constexpr Flags &operator^=(T flag) noexcept
    {
      value ^= static_cast<UnderlyingType>(flag);
      return *this;
    }

    constexpr Flags operator^(T flag) const noexcept
    {
      return Flags(value ^ static_cast<UnderlyingType>(flag));
    }

    constexpr Flags operator~(T flag) const noexcept
    {
      return Flags(value & ~static_cast<UnderlyingType>(flag));
    }

    constexpr Flags &operator~=(T flag) noexcept
    {
      value &= ~static_cast<UnderlyingType>(flag);
      return *this;
    }

    constexpr bool operator==(T flag) const noexcept
    {
      return (value & static_cast<UnderlyingType>(flag)) != 0;
    }

    constexpr bool operator!=(T flag) const noexcept
    {
      return !(*this == flag);
    }

    constexpr UnderlyingType Value() const noexcept
    {
      return value;
    }

    constexpr explicit operator UnderlyingType() const noexcept
    {
      return value;
    }

    constexpr bool HasFlag(T flag) const noexcept
    {
      return (value & static_cast<UnderlyingType>(flag)) != 0;
    }
  };
}