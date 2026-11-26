#include "Krystal.Booey/Events/UI/UIEvent.hpp"
#include "Krystal.Booey/HTML/Window.hpp"

namespace krys::boo
{
  UIEvent::UIEvent(DOMStringAtom type, const UIEventInit &eventInitDict) noexcept
      : Event(krys::move(type), eventInitDict), _view(ShareRefPtr(eventInitDict.View)),
        _detail(eventInitDict.Detail)
  {
  }

  UIEvent::~UIEvent() noexcept = default;

  Window *UIEvent::View() const noexcept
  {
    return _view.get();
  }

  void UIEvent::InitUIEvent(DOMStringAtom type, bool bubbles, bool cancelable, Window *view,
                            int32 detail) noexcept
  {
    if (_dispatched)
    {
      return;
    }

    InitEvent(krys::move(type), bubbles, cancelable);
    _view = ShareRefPtr(view);
    _detail = detail;
  }

  void UIEvent::SetView(Window *view) noexcept
  {
    _view = ShareRefPtr(view);
  }
}
