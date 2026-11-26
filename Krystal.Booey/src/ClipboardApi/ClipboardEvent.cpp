#include "Krystal.Booey/ClipboardApi/ClipboardEvent.hpp"
#include "Krystal.Booey/HTML/DragAndDrop/DataTransfer.hpp"

namespace krys::boo
{
  ClipboardEvent::ClipboardEvent(DOMStringAtom type, const ClipboardEventInit &eventInitDict) noexcept
      : Event(krys::move(type), eventInitDict), _clipboardData(eventInitDict.ClipboardData)
  {
  }

  ClipboardEvent::~ClipboardEvent() noexcept = default;

  Ref<ClipboardEvent> ClipboardEvent::Create(const DOMStringAtom &type,
                                             Ref<DataTransfer> &&dataTransfer) noexcept
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
