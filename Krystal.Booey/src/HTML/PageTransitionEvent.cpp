#include "Krystal.Booey/HTML/PageTransitionEvent.hpp"

namespace krys::boo::html
{
  PageTransitionEvent::PageTransitionEvent(dom::DOMStringAtom type,
                                           const PageTransitionEventInit &eventInitDict) noexcept
      : Event(type, eventInitDict), _persisted(eventInitDict.Persisted)
  {
  }

  Ref<PageTransitionEvent> PageTransitionEvent::Create(dom::DOMStringAtom type, bool persisted) noexcept
  {
    PageTransitionEventInit init;
    init.Persisted = persisted;
    auto event = CreateRef<PageTransitionEvent>(type, init);
    event->_isTrusted = true;
    return event;
  }
}
