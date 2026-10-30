#pragma once

#include "Krystal.Booey/CSS/Values/Grid/GridNamedAreaMap.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  using GridNamedAreaMap = css::GridNamedAreaMap;
  template <>
  inline constexpr bool TreatAsNonConverting<GridNamedAreaMap> = true;
}
