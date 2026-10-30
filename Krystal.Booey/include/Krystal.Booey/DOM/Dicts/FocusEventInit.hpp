#pragma once

#include "Krystal.Booey/DOM/Dicts/UIEventInit.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class EventTarget;

  /// @see https://w3c.github.io/uievents/#dictdef-focuseventinit
  struct FocusEventInit : UIEventInit
  {
    RefPtr<EventTarget> RelatedTarget;
  };
}
