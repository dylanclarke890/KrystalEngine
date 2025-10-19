#pragma once

#include "Krystal.UI.Layout/UnitValue.hpp"

namespace Krys::UI
{
  constexpr Layout::UnitValue operator"" _px(long double value)
  {
    return Layout::UnitValue {(float)value, Layout::Unit::Point};
  }

  constexpr Layout::UnitValue operator"" _pct(long double value)
  {
    return Layout::UnitValue {(float)value, Layout::Unit::Percent};
  }
}