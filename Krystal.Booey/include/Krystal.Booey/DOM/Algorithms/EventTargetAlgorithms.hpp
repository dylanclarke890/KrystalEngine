#pragma once

#include "Krystal.Booey/DOM/Dicts/AddEventListenerOptions.hpp"
#include "Krystal.Booey/DOM/Dicts/EventListenerOptions.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::dom
{
  class EventTarget;
  class EventListener;

  /// @brief Implementations of the EventTarget interface algorithms.
  /// @see https://dom.spec.whatwg.org/#eventtarget
  class EventTargetAlgorithms
  {
  public:
    /// @see https://dom.spec.whatwg.org/#concept-flatten-options
    KRYS_NODISCARD static EventListenerOptions
      FlattenOptions(const EventListenerOptionsOrBool &optionsOrBool) noexcept;

    /// @see https://dom.spec.whatwg.org/#event-flatten-more
    KRYS_NODISCARD static AddEventListenerOptions
      FlattenMoreOptions(const AddEventListenerOptionsOrBool &optionsOrBool) noexcept;

    /// @see https://dom.spec.whatwg.org/#default-passive-value
    KRYS_NODISCARD static bool DefaultPassiveValue(const DOMStringAtom &type,
                                                   const EventTarget &eventTarget) noexcept;

    /// @see https://dom.spec.whatwg.org/#add-an-event-listener
    static void AddEventListener(EventTarget &eventTarget, EventListener &listener) noexcept;

    /// @see https://dom.spec.whatwg.org/#remove-an-event-listener
    static void RemoveEventListener(EventTarget &eventTarget, EventListener &listener) noexcept;

    /// @see https://dom.spec.whatwg.org/#remove-all-event-listeners
    static void RemoveAllEventListeners(EventTarget &eventTarget) noexcept;

    /// @see https://dom.spec.whatwg.org/#retarget
    KRYS_NODISCARD static EventTarget *Retarget(EventTarget *a, EventTarget &b) noexcept;
  };
}