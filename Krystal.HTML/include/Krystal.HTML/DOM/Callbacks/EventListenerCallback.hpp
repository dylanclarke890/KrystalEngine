#pragma once

#include "Krystal.Lib/Mixins/CanMakeWeakPtr.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"

namespace Krys::HTML
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