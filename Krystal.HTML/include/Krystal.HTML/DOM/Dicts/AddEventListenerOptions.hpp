#pragma once

#include "Krystal.HTML/DOM/Dicts/EventListenerOptions.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/BoolOr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class AbortSignal;

  /// @see https://dom.spec.whatwg.org/#dictdef-addeventlisteneroptions
  struct AddEventListenerOptions : public EventListenerOptions
  {
    bool Once = false;
    Maybe<bool> Passive {};
    RefPtr<AbortSignal> Signal;
  };

  using AddEventListenerOptionsOrBool = BoolOr<AddEventListenerOptions>;
}