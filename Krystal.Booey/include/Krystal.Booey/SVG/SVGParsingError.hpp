#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::svg
{
  enum class SVGParsingError : uint8
  {
    None,
    ParsingFailed,
    ForbiddenNegativeValue,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::svg::SVGParsingError, 3uz);
