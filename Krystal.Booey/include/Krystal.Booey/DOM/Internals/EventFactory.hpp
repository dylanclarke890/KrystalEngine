#pragma once

#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#constructing-events
  class EventFactory
  {
  public:
    /// @brief A trusted event of interface @p TEvent, created by the user agent rather than its constructor,
    /// with every attribute at its default and a null type for the caller to set.
    /// @see https://dom.spec.whatwg.org/#concept-event-create
    template <DerivedFrom<Event> TEvent = Event>
    requires Constructible<TEvent, DOMStringAtom, const typename TEvent::dictionary_type &>
    KRYS_NODISCARD static Ref<TEvent> Create() noexcept
    {
      // 1. If realm is not given, then set it to null. (Skipped: there are no realms.)

      // 2. Let dictionary be the result of converting the JavaScript value undefined to the dictionary type
      // accepted by eventInterface's constructor.
      typename TEvent::dictionary_type dictionary {};

      // 3. Let event be the result of running the inner event creation steps with eventInterface, realm, the
      // time of the occurrence that the event is signaling, and dictionary.
      // The constructor runs the inner event creation steps, with the time now.
      Ref<TEvent> event = CreateRef<TEvent>(DOMStringAtom::Null(), dictionary);

      // 4. Initialize event's isTrusted attribute to true.
      static_cast<Event &>(*event)._isTrusted = true;

      // 5. Return event.
      return event;
    }
  };
}