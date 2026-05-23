#pragma once

#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/Variant.hpp"

namespace Krys::HTML
{
  class AbortSignal;

  struct EventListenerOptions
  {
    bool Capture = false;
  };

  using EventListenerOptionsOrBool = Variant<EventListenerOptions, bool>;

  struct AddEventListenerOptions : public EventListenerOptions
  {
    bool Once = false;
    Maybe<bool> Passive {};
    RefPtr<AbortSignal> Signal;
  };

  using AddEventListenerOptionsOrBool = Variant<AddEventListenerOptions, bool>;
}