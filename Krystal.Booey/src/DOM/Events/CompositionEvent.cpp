#include "Krystal.Booey/DOM/Events/CompositionEvent.hpp"

namespace krys::boo::dom
{
  CompositionEvent::CompositionEvent(DOMStringAtom type, const CompositionEventInit &initializer) noexcept
      : UIEvent(type, initializer), _data(initializer.Data)
  {
  }

  CompositionEvent::~CompositionEvent() noexcept = default;

  Ref<CompositionEvent> CompositionEvent::Create(DOMStringAtom type, LocalDOMWindow *view,
                                                 DOMStringView data) noexcept
  {
    CompositionEventInit init;
    init.Bubbles = true;
    init.Cancelable = true;
    init.Composed = true;
    init.Data = DOMString(data);
    auto event = CreateRef<CompositionEvent>(type, init);
    event->SetView(view);
    event->_isTrusted = true;
    return event;
  }

  void CompositionEvent::InitCompositionEvent(DOMStringAtom type, bool canBubble, bool cancelable,
                                              LocalDOMWindow *view, DOMStringView data) noexcept
  {
    if (IsBeingDispatched())
    {
      return;
    }

    InitUIEvent(type, canBubble, cancelable, view, 0);

    _data = DOMString(data);
  }
}
