#include "Krystal.Booey/DOM/CompositionEvent.hpp"

namespace krys::boo::dom
{
  CompositionEvent::CompositionEvent(DOMStringAtom type, const CompositionEventInit &initializer) noexcept
      : UIEvent(type, initializer), _data(initializer.Data)
  {
  }

  CompositionEvent::~CompositionEvent() noexcept = default;

  Ref<CompositionEvent> CompositionEvent::Create(DOMStringAtom type, DOMStringView data) noexcept
  {
    CompositionEventInit init;
    init.Bubbles = true;
    init.Cancelable = true;
    init.Composed = true;
    init.Data = DOMString(data);
    auto event = CreateRef<CompositionEvent>(type, init);
    event->_isTrusted = true;
    return event;
  }

  void CompositionEvent::InitCompositionEvent(DOMStringAtom type, bool canBubble, bool cancelable,
                                              DOMStringView data) noexcept
  {
    if (IsBeingDispatched())
    {
      return;
    }

    InitUIEvent(type, canBubble, cancelable, 0);

    _data = DOMString(data);
  }
}
