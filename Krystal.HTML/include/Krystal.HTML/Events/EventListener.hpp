#pragma once

#include "Krystal.HTML/Events/Event.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RefCounted.hpp"
#include "Krystal.Lib/Types/Func.hpp"

namespace Krys::HTML
{
  class EventListener : public RefCounted<EventListener>
  {
  public:
    virtual ~EventListener() noexcept = default;

    virtual bool operator==(const EventListener &other) const noexcept
    {
      return this == &other;
    }

    virtual void HandleEvent(Event &event) noexcept = 0;
  };
}