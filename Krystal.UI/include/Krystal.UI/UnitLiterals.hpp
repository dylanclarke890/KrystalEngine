#pragma once

#include "Krystal.UI.Styles/Values/UnitValue.hpp"

namespace Krys::UI
{
  constexpr Styles::UnitValue operator"" _px(long double value)
  {
    return Styles::UnitValue {(float)value, Styles::Unit::Point};
  }

  constexpr Styles::UnitValue operator"" _pct(long double value)
  {
    return Styles::UnitValue {(float)value, Styles::Unit::Percent};
  }
}