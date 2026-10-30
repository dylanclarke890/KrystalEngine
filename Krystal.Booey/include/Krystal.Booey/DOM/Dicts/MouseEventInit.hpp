#pragma once

#include "Krystal.Booey/DOM/Dicts/EventModifierInit.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class EventTarget;

  /// @see https://w3c.github.io/uievents/#dictdef-mouseeventinit
  struct MouseEventInit : EventModifierInit
  {
    double ScreenX {0};
    double ScreenY {0};
    /// @see https://w3c.github.io/pointerlock/#extensions-to-the-mouseeventinit-dictionary
    double MovementX {0};
    double MovementY {0};
    double ClientX {0};
    double ClientY {0};
    int16 Button {0};
    uint16 Buttons {0};
    RefPtr<EventTarget> RelatedTarget;
  };
}
