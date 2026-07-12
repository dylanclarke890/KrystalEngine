#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"

namespace Krys
{
  /// @brief A simple wrapper for a non-type template parameter that may or may not have a value. Useful for
  /// cases where you want to use a non-type template parameter but also want to be able to handle the absence
  /// of a value without having to use a separate template parameter for that.
  /// @note Caller is responsible for ensuring that the value of `T` is valid if `HasValue` is true. This
  /// struct does not perform any validation on the value of `T` and simply assumes that if `HasValue` is
  /// true, then `Value` is valid and can be used.
  template <typename T>
  struct NTTPMaybe
  {
    T Value;
    bool HasValue{false};

    NTTPMaybe() noexcept = default;

    consteval NTTPMaybe(T value) noexcept : Value(value), HasValue(true)
    {
    }

    KRYS_NODISCARD consteval T operator*() const noexcept
    {
      return Value;
    }

    KRYS_NODISCARD consteval operator bool() const noexcept
    {
      return HasValue;
    }
  };
}