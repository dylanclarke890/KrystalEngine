#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  enum class ParserMode : uint8
  {
    HTMLStandard,
    HTMLQuirks,
    SVGAttribute,
    UASheet,
  };

  KRYS_NODISCARD constexpr bool IsQuirksModeBehavior(ParserMode mode) noexcept
  {
    return mode == ParserMode::HTMLQuirks;
  }

  KRYS_NODISCARD constexpr bool IsUASheetBehavior(ParserMode mode) noexcept
  {
    return mode == ParserMode::UASheet;
  }

  KRYS_NODISCARD constexpr bool IsUnitlessValueParsingForcedForMode(ParserMode mode) noexcept
  {
    return mode == ParserMode::SVGAttribute;
  }
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::ParserMode, 4uz);