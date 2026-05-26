#pragma once

#include "Krystal.HTML/DOM/Event/Event.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  /// @see https://dom.spec.whatwg.org/#constructing-events
  class EventFactory
  {
  public:
    /// @see https://dom.spec.whatwg.org/#concept-event-create
    template <DerivedFrom<Event> TEvent = Event>
    KRYS_NODISCARD static Ref<TEvent> Create() noexcept
    {
      using dictionary_type = typename TEvent::dictionary_type;
      dictionary_type dictionary {};

      if constexpr (Constructible<TEvent, const dictionary_type &>)
      {
        auto event = CreateRef<TEvent>(dictionary);
        event->_isTrusted = true;
        event->_initialized = true;
        return event;
      }
      else if constexpr (Constructible<TEvent, DOMStringAtom, const dictionary_type &>)
      {
        // Later steps of the algorithm will set the event's type, so we can pass a null atom here.
        auto event = CreateRef<TEvent>(DOMStringAtom::Null(), dictionary);
        event->_isTrusted = true;
        event->_initialized = true;

        return event;
      }
      else
      {
        static_assert(false, "TEvent must be constructible with either (DOMStringAtom, const dictionary_type "
                             "&) or (const dictionary_type &).");
      }
    }
  };
}