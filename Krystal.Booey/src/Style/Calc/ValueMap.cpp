#include "Krystal.Booey/Style/Calc/ValueMap.hpp"

namespace krys::boo::style::calc
{
  ValueMap::ValueMap() noexcept = default;

  ValueMap &ValueMap::CalculationValues() noexcept
  {
    static NeverDestroyed<ValueMap> map;
    return map;
  }
}
