#include "Krystal.Web/Css/Containment/ContentVisibilityAutoStateChangeEvent.hpp"

namespace krys::web
{
  ContentVisibilityAutoStateChangeEvent::ContentVisibilityAutoStateChangeEvent(
    DOMStringAtom type, const ContentVisibilityAutoStateChangeEventInit &eventInitDict) noexcept
      : Event(krys::move(type), eventInitDict), _skipped(eventInitDict.Skipped)
  {
  }

  Ref<ContentVisibilityAutoStateChangeEvent> ContentVisibilityAutoStateChangeEvent::Create(
    const DOMStringAtom &type, const ContentVisibilityAutoStateChangeEventInit &init, bool isTrusted) noexcept
  {
    auto event = CreateRef<ContentVisibilityAutoStateChangeEvent>(type, init);
    event->_isTrusted = isTrusted;
    return event;
  }
}
