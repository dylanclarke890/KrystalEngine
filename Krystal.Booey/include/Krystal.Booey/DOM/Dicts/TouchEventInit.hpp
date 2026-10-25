#pragma once

#include "Krystal.Booey/DOM/Dicts/EventModifierInit.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Touch;

  /// @see https://w3c.github.io/touch-events/#touchevent-interface
  struct TouchEventInit : EventModifierInit
  {
    List<Ref<Touch>> Touches;
    List<Ref<Touch>> TargetTouches;
    List<Ref<Touch>> ChangedTouches;
  };
}
