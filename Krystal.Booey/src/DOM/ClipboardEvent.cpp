#include "Krystal.Booey/DOM/ClipboardEvent.hpp"
#include "Krystal.Booey/DOM/DataTransfer.hpp"

namespace krys::boo::dom
{
  ClipboardEvent::ClipboardEvent(DOMStringAtom type, const ClipboardEventInit &eventInitDict) noexcept
      : Event(type, eventInitDict), _clipboardData(eventInitDict.ClipboardData)
  {
  }

  ClipboardEvent::~ClipboardEvent() noexcept = default;

  Ref<ClipboardEvent> ClipboardEvent::Create(DOMStringAtom type, Ref<DataTransfer> &&dataTransfer) noexcept
  {
    ClipboardEventInit init;
    init.Bubbles = true;
    init.Cancelable = true;
    init.Composed = true;
    init.ClipboardData = krys::move(dataTransfer);
    auto event = CreateRef<ClipboardEvent>(type, init);
    event->_isTrusted = true;
    return event;
  }
}
