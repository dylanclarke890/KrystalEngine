#pragma once

#include "Krystal.HTML/CSS/Values/Enums/CSSUnitCategory.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSUnitType.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSValueType.hpp"

namespace Krys::HTML
{
  class CSSValue
  {
  private:
    CSSValueType _type : BitCount<CSSValueType>();
    CSSUnitType _unit : BitCount<CSSUnitType>();
  };
}