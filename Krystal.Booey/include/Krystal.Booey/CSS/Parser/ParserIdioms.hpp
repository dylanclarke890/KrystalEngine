#pragma once

#include "Krystal.Booey/CSS/Values/GlobalKeyword.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

/// @see https://drafts.csswg.org/cssom/#common-parsing-idioms
namespace krys::boo::css
{
  /// @brief Space characters as defined by the CSS specification.
  /// @see http://www.w3.org/TR/css3-syntax/#whitespace
  template <typename CharacterType>
  KRYS_NODISCARD constexpr bool IsCSSSpace(CharacterType c) noexcept
  {
    return c == ' ' || c == '\t' || c == '\n';
  }

  /// @see http://dev.w3.org/csswg/css-syntax/#name-start-code-point
  template <typename CharacterType>
  KRYS_NODISCARD constexpr bool IsNameStartCodePoint(CharacterType c) noexcept
  {
    return krys::text::IsASCIIAlpha(c) || c == '_' || !krys::text::IsASCII(c);
  }

  /// @see http://dev.w3.org/csswg/css-syntax/#name-code-point
  template <typename CharacterType>
  KRYS_NODISCARD constexpr bool IsNameCodePoint(CharacterType c) noexcept
  {
    return IsNameStartCodePoint(c) || krys::text::IsASCIIDigit(c) || c == '-';
  }

  KRYS_NODISCARD constexpr bool IsValidCustomIdentifier(ValueId valueId) noexcept
  {
    // "default" is obsolete as a CSS-wide keyword but is still not allowed as a custom identifier.
    return !IsGlobalKeyword(valueId) && valueId != ValueId::Default;
  }

  /// @see https://drafts.csswg.org/css-conditional-5/#propdef-container-name
  KRYS_NODISCARD constexpr bool IsValidContainerNameIdentifier(ValueId valueId) noexcept
  {
    switch (valueId)
    {
      case ValueId::None:
      case ValueId::And:
      case ValueId::Or:
      case ValueId::Not:  return false;
      default:            return IsValidCustomIdentifier(valueId);
    }
  }
}