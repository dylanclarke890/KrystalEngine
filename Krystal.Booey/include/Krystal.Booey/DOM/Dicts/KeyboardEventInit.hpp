#pragma once

#include "Krystal.Booey/DOM/Dicts/EventModifierInit.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::dom
{
  /// @see https://w3c.github.io/uievents/#dictdef-keyboardeventinit
  struct KeyboardEventInit : EventModifierInit
  {
    DOMString Key;
    DOMString Code;
    uint32 Location {0};
    bool Repeat {false};
    bool IsComposing {false};

    // Legacy.
    DOMString KeyIdentifier;
    /// @see https://w3c.github.io/uievents/#dom-keyboardeventinit-charcode
    uint32 CharCode {0};
    /// @see https://w3c.github.io/uievents/#dom-keyboardeventinit-keycode
    uint32 KeyCode {0};
    /// @see https://w3c.github.io/uievents/#dom-uieventinit-which
    uint32 Which {0};
  };
}
