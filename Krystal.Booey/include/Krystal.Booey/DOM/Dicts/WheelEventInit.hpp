#pragma once

#include "Krystal.Booey/DOM/Dicts/MouseEventInit.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::dom
{
  /// @see https://w3c.github.io/uievents/#dictdef-wheeleventinit
  struct WheelEventInit : MouseEventInit
  {
    double DeltaX {0};
    double DeltaY {0};
    double DeltaZ {0};
    uint32 DeltaMode {0};
  };
}
