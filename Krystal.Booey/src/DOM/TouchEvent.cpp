#include "Krystal.Booey/DOM/TouchEvent.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"

namespace krys::boo::dom
{
  namespace
  {
    KRYS_NODISCARD MouseEventInit ToMouseEventInit(const TouchEventInit &initializer) noexcept
    {
      MouseEventInit init;
      static_cast<EventModifierInit &>(init) = initializer;
      return init;
    }

    KRYS_NODISCARD UIEventInit TrustedInit(DOMStringAtom type) noexcept
    {
      // https://w3c.github.io/touch-events/#list-of-touchevent-types
      UIEventInit init;
      init.Bubbles = true;
      init.Cancelable = type != EventNames::TouchCancel;
      init.Composed = true;
      return init;
    }
  }

  TouchEvent::TouchEvent(DOMStringAtom type, const TouchEventInit &initializer) noexcept
      : MouseRelatedEvent(type, ToMouseEventInit(initializer)),
        _touches(TouchList::Create(initializer.Touches)),
        _targetTouches(TouchList::Create(initializer.TargetTouches)),
        _changedTouches(TouchList::Create(initializer.ChangedTouches))
  {
  }

  TouchEvent::TouchEvent(TouchList *touches, TouchList *targetTouches, TouchList *changedTouches,
                         DOMStringAtom type, LocalFrameView *view, const DoublePoint &globalLocation,
                         EnumSet<Modifier> modifiers) noexcept
      : MouseRelatedEvent(type, TrustedInit(type), view, globalLocation, {}, 0, 0, modifiers, Simulated::No),
        _touches(ShareRefPtr(touches)), _targetTouches(ShareRefPtr(targetTouches)),
        _changedTouches(ShareRefPtr(changedTouches))
  {
    _isTrusted = true;
  }

  TouchEvent::~TouchEvent() noexcept = default;

  Ref<TouchEvent> TouchEvent::Create(TouchList *touches, TouchList *targetTouches, TouchList *changedTouches,
                                     DOMStringAtom type, LocalFrameView *view,
                                     const DoublePoint &globalLocation, EnumSet<Modifier> modifiers) noexcept
  {
    return AdoptRef(
      *new TouchEvent(touches, targetTouches, changedTouches, type, view, globalLocation, modifiers));
  }
}
