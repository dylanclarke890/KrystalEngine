#include "Krystal.Booey/HTML/Events/PageTransitionEvent.hpp"

namespace krys::boo::html
{
  PageTransitionEvent::PageTransitionEvent(dom::DOMStringAtom type,
                                           const PageTransitionEventInit &eventInitDict) noexcept
      : Event(type, eventInitDict), _persisted(eventInitDict.Persisted)
  {
  }

  // https://html.spec.whatwg.org/#fire-a-page-transition-event
  Ref<PageTransitionEvent> PageTransitionEvent::Create(dom::DOMStringAtom type, bool persisted) noexcept
  {
    // ... with the persisted attribute initialized to persisted, the cancelable attribute initialized to
    // true, the bubbles attribute initialized to true, and legacy target override flag set.
    PageTransitionEventInit init;
    init.Persisted = persisted;
    init.Cancelable = true;
    init.Bubbles = true;
    auto event = CreateRef<PageTransitionEvent>(type, init);
    event->_isTrusted = true;
    return event;
  }
}
