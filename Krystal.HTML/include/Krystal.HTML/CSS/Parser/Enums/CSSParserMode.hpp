#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class CSSParserMode : uint8
  {
    HTMLStandard,
    HTMLQuirks,
    SVGAttribute,
    UASheet,
  };

  KRYS_NODISCARD constexpr bool IsQuirksModeBehavior(CSSParserMode mode) noexcept
  {
    return mode == CSSParserMode::HTMLQuirks;
  }

  KRYS_NODISCARD constexpr bool IsUASheetBehavior(CSSParserMode mode) noexcept
  {
    return mode == CSSParserMode::UASheet;
  }

  KRYS_NODISCARD constexpr bool IsUnitlessValueParsingForcedForMode(CSSParserMode mode) noexcept
  {
    return mode == CSSParserMode::SVGAttribute;
  }
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::CSSParserMode, 4uz);