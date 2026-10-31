#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#dom-event-eventphase
  enum class EventPhaseType : uint8
  {
    /// @see https://dom.spec.whatwg.org/#dom-event-none
    NONE = 0,
    /// @see https://dom.spec.whatwg.org/#dom-event-capturing_phase
    CAPTURING_PHASE = 1,
    /// @see https://dom.spec.whatwg.org/#dom-event-at_target
    AT_TARGET = 2,
    /// @see https://dom.spec.whatwg.org/#dom-event-bubbling_phase
    BUBBLING_PHASE = 3
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::dom::EventPhaseType, 4u)
