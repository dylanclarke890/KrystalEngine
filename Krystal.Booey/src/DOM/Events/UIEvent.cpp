#include "Krystal.Booey/DOM/Events/UIEvent.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"

namespace krys::boo::dom
{
  UIEvent::UIEvent(DOMStringAtom type, const UIEventInit &eventInitDict) noexcept
      : Event(type, eventInitDict), _view(ShareRefPtr(eventInitDict.View)), _detail(eventInitDict.Detail)
  {
  }

  UIEvent::~UIEvent() noexcept = default;

  LocalDOMWindow *UIEvent::View() const noexcept
  {
    return _view.get();
  }

  void UIEvent::InitUIEvent(DOMStringAtom type, bool bubbles, bool cancelable, LocalDOMWindow *view,
                            int32 detail) noexcept
  {
    if (_dispatched)
    {
      return;
    }

    InitEvent(type, bubbles, cancelable);
    _view = ShareRefPtr(view);
    _detail = detail;
  }

  void UIEvent::SetView(LocalDOMWindow *view) noexcept
  {
    _view = ShareRefPtr(view);
  }
}
