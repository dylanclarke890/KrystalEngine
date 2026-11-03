#include "Krystal.Booey/DOM/Events/BeforeTextInsertedEvent.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"

namespace krys::boo::dom
{
  BeforeTextInsertedEvent::BeforeTextInsertedEvent(DOMStringView text) noexcept
      : Event(DOMStringAtom(EventNames::WebkitBeforeTextInserted), {.Bubbles = false, .Cancelable = true}),
        _text(text)
  {
  }

  BeforeTextInsertedEvent::~BeforeTextInsertedEvent() noexcept = default;

  Ref<BeforeTextInsertedEvent> BeforeTextInsertedEvent::Create(DOMStringView text) noexcept
  {
    auto event = CreateRef<BeforeTextInsertedEvent>(text);
    event->_isTrusted = true;
    return event;
  }
}
