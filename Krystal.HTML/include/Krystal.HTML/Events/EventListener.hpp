#pragma once

#include "Krystal.HTML/Events/Event.hpp"
#include "Krystal.Lib/Mixins/CanMakeWeakPtr.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Types/Func.hpp"

namespace Krys::HTML
{
  class EventListener : public RefCounted<EventListener>, public CanMakeWeakPtr<EventListener>
  {
  public:
    virtual ~EventListener() noexcept = default;

#pragma region EventListener - https://dom.spec.whatwg.org/#eventlistener

    virtual void HandleEvent(Event &event) noexcept = 0;

#pragma endregion

    virtual bool operator==(const EventListener &other) const noexcept
    {
      return this == &other;
    }
  };
}