#pragma once

#include "Krystal.HTML/DOM/Dicts/AddEventListenerOptions.hpp"
#include "Krystal.HTML/DOM/Dicts/EventListenerOptions.hpp"
#include "Krystal.HTML/Types/DOMStringAtom.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"

namespace Krys::HTML
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
    KRYS_NODISCARD static RawPtr<EventTarget> Retarget(RawPtr<EventTarget> a, EventTarget &b) noexcept;
  };
}