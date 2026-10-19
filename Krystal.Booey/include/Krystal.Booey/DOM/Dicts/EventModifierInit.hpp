#pragma once

#include "Krystal.Booey/DOM/Dicts/UIEventInit.hpp"

namespace krys::boo::dom
{
  /// @see https://w3c.github.io/uievents/#dictdef-eventmodifierinit
  // FIXME: The specification also has modifierFn, modifierFnLock, modifierHyper, modifierNumLock,
  // modifierScrollLock, modifierSuper, modifierSymbol and modifierSymbolLock.
  struct EventModifierInit : UIEventInit
  {
    bool CtrlKey {false};
    bool ShiftKey {false};
    bool AltKey {false};
    bool MetaKey {false};

    bool ModifierAltGraph {false};
    bool ModifierCapsLock {false};
  };
}
