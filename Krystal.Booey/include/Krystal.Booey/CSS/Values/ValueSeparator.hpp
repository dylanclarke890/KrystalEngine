#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Strings/String.hpp"

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

  /// @brief @p separator as it's serialised between two values.
  KRYS_NODISCARD constexpr utf8_stringview ToCssText(ValueSeparator separator) noexcept
  {
    switch (separator)
    {
      case ValueSeparator::Space: return u8" ";
      case ValueSeparator::Comma: return u8", ";
      case ValueSeparator::Slash: return u8" / ";
    }
    krys_unreachable();
  }
}
