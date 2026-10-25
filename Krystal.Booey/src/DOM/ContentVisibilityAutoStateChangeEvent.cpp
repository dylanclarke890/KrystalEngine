#include "Krystal.Booey/DOM/ContentVisibilityAutoStateChangeEvent.hpp"

namespace krys::boo::dom
{
  ContentVisibilityAutoStateChangeEvent::ContentVisibilityAutoStateChangeEvent(
    DOMStringAtom type, const ContentVisibilityAutoStateChangeEventInit &eventInitDict) noexcept
      : Event(type, eventInitDict), _skipped(eventInitDict.Skipped)
  {
  }

  Ref<ContentVisibilityAutoStateChangeEvent> ContentVisibilityAutoStateChangeEvent::Create(
    DOMStringAtom type, const ContentVisibilityAutoStateChangeEventInit &init, bool isTrusted) noexcept
  {
    auto event = CreateRef<ContentVisibilityAutoStateChangeEvent>(type, init);
    event->_isTrusted = isTrusted;
    return event;
  }
}
