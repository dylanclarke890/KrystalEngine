#pragma once

#include "Krystal.Booey/DOM/DataTransfer.hpp"
#include "Krystal.Booey/DOM/Dicts/UIEventInit.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  /// @see https://w3c.github.io/uievents/#dictdef-inputeventinit
  struct InputEventInit : UIEventInit
  {
    Maybe<DOMString> Data;
    RefPtr<dom::DataTransfer> DataTransfer;
    bool IsComposing {false};
    DOMString InputType;
  };
}
