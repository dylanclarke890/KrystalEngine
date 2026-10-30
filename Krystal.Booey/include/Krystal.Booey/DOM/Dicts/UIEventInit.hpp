#pragma once

#include "Krystal.Booey/DOM/Dicts/EventInit.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::dom
{
  /// @see https://w3c.github.io/uievents/#dictdef-uieventinit
  // FIXME: The view (a Window).
  struct UIEventInit : EventInit
  {
    int32 Detail {0};
  };
}
