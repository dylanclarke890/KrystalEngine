#pragma once

#include "Krystal.HTML/CSS/Properties/Enums/CSSUnitCategory.hpp"
#include "Krystal.HTML/CSS/Properties/Enums/CSSUnitType.hpp"
#include "Krystal.HTML/CSS/Properties/Enums/CSSValueType.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"

namespace Krys::HTML
{
  class CSSValue : public RefCounted<CSSValue>
  {
  private:
    CSSValueType _type : BitCount<CSSValueType>();
    CSSUnitType _unit : BitCount<CSSUnitType>();
  };
}