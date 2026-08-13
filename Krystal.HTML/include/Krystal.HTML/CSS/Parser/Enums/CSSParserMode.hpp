#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class CSSParserMode : uint8
  {
    HTMLStandard,
    HTMLQuirks,
  };

  KRYS_NODISCARD inline bool IsUnitlessValueParsingForcedForMode(CSSParserMode mode) noexcept
  {
    return false;
  }

}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::CSSParserMode, 2uz);