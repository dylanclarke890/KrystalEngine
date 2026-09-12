#pragma once

#include "Krystal.Core/Types/BoolOr.hpp"

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#dictdef-eventlisteneroptions
  struct EventListenerOptions
  {
    bool Capture = false;
  };

  using EventListenerOptionsOrBool = BoolOr<EventListenerOptions>;
}