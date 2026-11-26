#pragma once

#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Web/Css/Parser/TokenType.hpp"

namespace krys::web::css
{
  struct NumericValue
  {
    double Value;
    NumericValueType Type {NumericValueType::Integer};
    NumericSignChar SignCharacter {NumericSignChar::Missing};
  };
}
