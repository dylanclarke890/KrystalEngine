#pragma once

#include "Krystal.Booey/DOM/Dicts/EventListenerOptions.hpp"
#include "Krystal.Core/Types/BoolOr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
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