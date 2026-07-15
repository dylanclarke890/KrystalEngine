#pragma once

#include "Krystal.Lib/Types/Variant.hpp"
#include "Krystal.HTML/CSS/Parser/Enums/CSSTokenType.hpp"

namespace Krys::HTML
{
  using Int64OrDouble = Variant<int64, double>;

  struct ParsedInt64OrDouble
  {
    Int64OrDouble Value;
    NumericTokenType Type {NumericTokenType::Integer};
  };
}