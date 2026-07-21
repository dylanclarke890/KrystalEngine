#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class CSSValueType : uint8
  {
    Invalid = 0,
    Keyword,
    Length,
    Percentage,
    Color,
    Function,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::CSSValueType, 6uz);
