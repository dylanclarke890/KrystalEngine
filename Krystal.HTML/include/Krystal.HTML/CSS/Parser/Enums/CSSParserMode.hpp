#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class CSSParserMode : uint8
  {
    HTMLStandard,
    HTMLQuirks,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::CSSParserMode, 2uz);