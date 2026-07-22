#pragma once

#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class CSSPropertyId : uint16
  {
    /// @brief The property is not a built in CSS property (but may be a valid custom CSS property).
    Invalid = 0,

#pragma region Longhand Properties
    MarginTop,
    MarginBottom,
    MarginLeft,
    MarginRight,
#pragma endregion

#pragma region Shorthand Properties
    Margin,
#pragma endregion
  };

  KRYS_NODISCARD constexpr CSSPropertyId ParseCSSPropertyId(CSSOMStringView name) noexcept
  {
    if (name == u8"margin-top")
    {
      return CSSPropertyId::MarginTop;
    }
    else if (name == u8"margin-bottom")
    {
      return CSSPropertyId::MarginBottom;
    }
    else if (name == u8"margin-left")
    {
      return CSSPropertyId::MarginLeft;
    }
    else if (name == u8"margin-right")
    {
      return CSSPropertyId::MarginRight;
    }
    else if (name == u8"margin")
    {
      return CSSPropertyId::Margin;
    }
    return CSSPropertyId::Invalid;
  }
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::CSSPropertyId, 6uz);