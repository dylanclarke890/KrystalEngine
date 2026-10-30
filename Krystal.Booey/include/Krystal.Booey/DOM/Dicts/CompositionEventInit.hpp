#pragma once

#include "Krystal.Booey/DOM/Dicts/UIEventInit.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"

namespace krys::boo::dom
{
  /// @see https://w3c.github.io/uievents/#dictdef-compositioneventinit
  struct CompositionEventInit : UIEventInit
  {
    DOMString Data;
  };
}
