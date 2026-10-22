#pragma once

#include "Krystal.Booey/DOM/Dicts/UIEventInit.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  /// @see https://w3c.github.io/uievents/#dictdef-inputeventinit
  // FIXME: The data transfer (dataTransfer), with DataTransfer.
  struct InputEventInit : UIEventInit
  {
    Maybe<DOMString> Data;
    bool IsComposing {false};
    DOMString InputType;
  };
}
