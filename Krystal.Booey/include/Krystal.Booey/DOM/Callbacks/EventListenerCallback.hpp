#pragma once

#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"

namespace krys::boo::dom
{
  class Event;

  /// @see https://dom.spec.whatwg.org/#callbackdef-eventlistener
  class EventListenerCallback : public RefCounted<EventListenerCallback>,
                                public CanMakeWeakPtr<EventListenerCallback>
  {
  public:
    virtual ~EventListenerCallback() noexcept = default;

#pragma region Event Listener Callback - https://dom.spec.whatwg.org/#callbackdef-eventlistener

    /// @see https://dom.spec.whatwg.org/#dom-eventlistener-handleevent
    virtual void HandleEvent(Event &event) noexcept = 0;

#pragma endregion

    virtual bool operator==(const EventListenerCallback &other) const noexcept
    {
      return this == &other;
    }
  };
}