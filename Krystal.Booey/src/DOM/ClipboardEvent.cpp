#include "Krystal.Booey/DOM/ClipboardEvent.hpp"

namespace krys::boo::dom
{
  ClipboardEvent::ClipboardEvent(DOMStringAtom type, const ClipboardEventInit &eventInitDict) noexcept
      : Event(type, eventInitDict)
  {
  }

  Ref<ClipboardEvent> ClipboardEvent::Create(DOMStringAtom type) noexcept
  {
    ClipboardEventInit init;
    init.Bubbles = true;
    init.Cancelable = true;
    init.Composed = true;
    auto event = CreateRef<ClipboardEvent>(type, init);
    event->_isTrusted = true;
    return event;
  }
}
