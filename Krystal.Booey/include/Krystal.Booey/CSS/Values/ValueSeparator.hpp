#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  /// @brief What goes between the items of a list of component values when it's serialised.
  enum class ValueSeparator : uint8
  {
    /// @brief `a b`
    Space,
    /// @brief `a, b`
    Comma,
    /// @brief `a / b`
    Slash,
  };

  consteval ValueSeparator MaxEnumValue(ValueSeparator) noexcept
  {
    return ValueSeparator::Slash;
  }
}
