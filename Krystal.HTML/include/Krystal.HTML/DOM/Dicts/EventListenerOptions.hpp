#pragma once

#include "Krystal.Lib/Types/BoolOr.hpp"

namespace Krys::HTML
{
  /// @see https://dom.spec.whatwg.org/#dictdef-eventlisteneroptions
  struct EventListenerOptions
  {
    bool Capture = false;
  };

  using EventListenerOptionsOrBool = BoolOr<EventListenerOptions>;
}