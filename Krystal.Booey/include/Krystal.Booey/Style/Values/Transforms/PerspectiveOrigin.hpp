#pragma once

#include "Krystal.Booey/Style/Values/Primitives/Position.hpp"

namespace krys::boo::style
{
  // <'perspective-origin'> = <position>
  // https://www.w3.org/TR/css-transforms-2/#propdef-perspective-origin
  using PerspectiveOrigin = Position;
  using PerspectiveOriginX = PositionX;
  using PerspectiveOriginY = PositionY;
}
