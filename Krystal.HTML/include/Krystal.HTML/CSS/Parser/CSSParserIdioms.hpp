#pragma once

#include "Krystal.HTML/CSS/Values/CSSWideKeyword.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"

namespace Krys::HTML
{
  /// @see https://drafts.csswg.org/cssom/#common-parsing-idioms
  class CSSParserIdioms
  {
  public:
    /// @brief Space characters as defined by the CSS specification.
    /// @see http://www.w3.org/TR/css3-syntax/#whitespace
    template <typename CharacterType>
    KRYS_NODISCARD constexpr static bool IsCSSSpace(CharacterType c) noexcept
    {
      return c == ' ' || c == '\t' || c == '\n';
    }

    /// @see http://dev.w3.org/csswg/css-syntax/#name-start-code-point
    template <typename CharacterType>
    KRYS_NODISCARD constexpr static bool IsNameStartCodePoint(CharacterType c) noexcept
    {
      return Krys::Text::IsASCIIAlpha(c) || c == '_' || !Krys::Text::IsASCII(c);
    }

    /// @see http://dev.w3.org/csswg/css-syntax/#name-code-point
    template <typename CharacterType>
    KRYS_NODISCARD constexpr static bool IsNameCodePoint(CharacterType c) noexcept
    {
      return IsNameStartCodePoint(c) || Krys::Text::IsASCIIDigit(c) || c == '-';
    }

    KRYS_NODISCARD constexpr static bool IsValidCustomIdentifier(CSSValueId valueId) noexcept
    {
      // "default" is obsolete as a CSS-wide keyword but is still not allowed as a custom identifier.
      return !IsCSSWideKeyword(valueId) && valueId != CSSValueId::Default;
    }

    /// @see https://drafts.csswg.org/css-conditional-5/#propdef-container-name
    KRYS_NODISCARD constexpr static bool IsValidContainerNameIdentifier(CSSValueId valueId) noexcept
    {
      switch (valueId)
      {
        case CSSValueId::None:
        case CSSValueId::And:
        case CSSValueId::Or:
        case CSSValueId::Not:  return false;
        default:               return IsValidCustomIdentifier(valueId);
      }
    }
  };
}