#pragma once

#include "Krystal.HTML/CSS/Parser/Enums/CSSTokenType.hpp"
#include "Krystal.Lib/Types/Variant.hpp"

namespace Krys::HTML
{
  struct NumericValue
  {
    double Value;
    NumericValueType Type {NumericValueType::Integer};
    NumericSignChar SignCharacter {NumericSignChar::Missing};
  };
}