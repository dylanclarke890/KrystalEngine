#pragma once

#include "Krystal.HTML/Utils/ExceptionOr.hpp"

namespace Krys::HTML
{
  class EventTarget;
  class Event;

  class EventDispatcher
  {
  public:
    EventDispatcher() = delete;
    ~EventDispatcher() = delete;

    static ExceptionOr<bool> DispatchToTarget(Event &event, EventTarget &target, bool legacyTargetOverrideFlag = false, bool legacyOutputDidListenersThrowFlag = false) noexcept;
  };
}