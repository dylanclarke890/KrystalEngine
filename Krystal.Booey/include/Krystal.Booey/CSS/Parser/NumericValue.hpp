#pragma once

#include "Krystal.Booey/CSS/Parser/TokenType.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::css
{
  struct NumericValue
  {
    double Value;
    NumericValueType Type {NumericValueType::Integer};
    NumericSignChar SignCharacter {NumericSignChar::Missing};
  };
}