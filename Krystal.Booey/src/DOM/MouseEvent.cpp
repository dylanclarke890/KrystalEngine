#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Platform/PlatformMouseEvent.hpp"
#include <cmath>

namespace krys::boo::dom
{
  namespace
  {
    KRYS_NODISCARD double FiniteOrZero(double value) noexcept
    {
      return std::isfinite(value) ? value : 0;
    }

    KRYS_NODISCARD int16 ButtonOrLeft(MouseButton button) noexcept
    {
      return static_cast<int16>(button == MouseButton::None ? MouseButton::Left : button);
    }

    KRYS_NODISCARD RefPtr<Node> AsNode(RefPtr<EventTarget> target) noexcept
    {
      return ShareRefPtr(DynamicDowncast<Node>(target.get()));
    }
  }

  bool IsAnyClick(DOMStringAtom eventType) noexcept
  {
    return eventType == EventNames::Click || eventType == EventNames::AuxClick;
  }

  bool IsAnyClick(const Event &event) noexcept
  {
    return IsAnyClick(event.Type());
  }

  MouseEvent::MouseEvent(DOMStringAtom type, const MouseEventInit &initializer) noexcept
      : MouseRelatedEvent(type, initializer),
        _button(initializer.Button == static_cast<int16>(MouseButton::None)
                  ? static_cast<int16>(MouseButton::Left)
                  : initializer.Button),
        _buttons(initializer.Buttons),
        _buttonDown(initializer.Button != static_cast<int16>(MouseButton::None)),
        _relatedTarget(initializer.RelatedTarget)
  {
    InitCoordinates({FiniteOrZero(initializer.ClientX), FiniteOrZero(initializer.ClientY)});
  }

  MouseEvent::MouseEvent(DOMStringAtom type, const UIEventInit &eventInitDict, LocalFrameView *view,
                         const DoublePoint &screenLocation, const DoublePoint &windowLocation,
                         double movementX, double movementY, EnumSet<Modifier> modifiers, MouseButton button,
                         uint16 buttons, EventTarget *relatedTarget, Simulated isSimulated) noexcept
      : MouseRelatedEvent(type, eventInitDict, view, screenLocation, windowLocation, movementX, movementY,
                          modifiers, isSimulated),
        _button(ButtonOrLeft(button)), _buttons(buttons), _buttonDown(button != MouseButton::None),
        _relatedTarget(ShareRefPtr(relatedTarget))
  {
    _isTrusted = true;
  }

  Ref<MouseEvent> MouseEvent::Create(DOMStringAtom eventType, LocalFrameView *view,
                                     const PlatformMouseEvent &event, int32 detail,
                                     Node *relatedTarget) noexcept
  {
    bool isMouseEnterOrLeave = eventType == EventNames::MouseEnter || eventType == EventNames::MouseLeave;

    UIEventInit init;
    init.Bubbles = !isMouseEnterOrLeave;
    init.Cancelable = !isMouseEnterOrLeave;
    init.Composed = !isMouseEnterOrLeave;
    init.Detail = detail;

    Ref<MouseEvent> mouseEvent =
      AdoptRef(*new MouseEvent(eventType, init, view, event.GlobalPosition(), event.Position(),
                               event.MovementDelta().X(), event.MovementDelta().Y(), event.Modifiers(),
                               event.Button(), event.Buttons(), relatedTarget, Simulated::No));
    mouseEvent->_timeStamp = static_cast<DOMHighResTimeStamp>(event.Timestamp().count());
    return mouseEvent;
  }

  Ref<MouseEvent> MouseEvent::Create(DOMStringAtom eventType, bool canBubble, bool isCancelable,
                                     bool isComposed, DOMHighResTimeStamp timestamp, LocalFrameView *view,
                                     int32 detail, double screenX, double screenY, double clientX,
                                     double clientY, EnumSet<Modifier> modifiers, MouseButton button,
                                     uint16 buttons, EventTarget *relatedTarget) noexcept
  {
    UIEventInit init;
    init.Bubbles = canBubble;
    init.Cancelable = isCancelable;
    init.Composed = isComposed;
    init.Detail = detail;

    Ref<MouseEvent> mouseEvent =
      AdoptRef(*new MouseEvent(eventType, init, view, {FiniteOrZero(screenX), FiniteOrZero(screenY)}, {}, 0,
                               0, modifiers, button, buttons, relatedTarget, Simulated::No));
    mouseEvent->InitCoordinates({FiniteOrZero(clientX), FiniteOrZero(clientY)});
    mouseEvent->_timeStamp = timestamp;
    return mouseEvent;
  }

  Ref<MouseEvent> MouseEvent::CreateSimulated(DOMStringAtom eventType, Event *underlyingEvent,
                                              Element &target, bool isTrusted) noexcept
  {
    UIEventInit init;
    init.Bubbles = true;
    init.Cancelable = true;
    init.Composed = true;

    EnumSet<Modifier> modifiers;
    for (auto *event = underlyingEvent; event; event = event->UnderlyingEvent())
    {
      if (event->IsMouseEvent() || event->IsKeyboardEvent())
      {
        modifiers = static_cast<UIEventWithKeyState *>(event)->ModifierKeys();
        break;
      }
    }

    auto *frame = target.NodeDocument().Frame();
    Ref<MouseEvent> mouseEvent =
      AdoptRef(*new MouseEvent(eventType, init, frame ? frame->View() : nullptr, {}, {}, 0, 0, modifiers,
                               MouseButton::Left, 0, nullptr, Simulated::Yes));
    mouseEvent->_isTrusted = isTrusted;
    if (underlyingEvent)
    {
      mouseEvent->_timeStamp = underlyingEvent->TimeStamp();
    }

    mouseEvent->SetUnderlyingEvent(underlyingEvent);
    if (auto *underlyingMouseEvent = DynamicDowncast<MouseEvent>(underlyingEvent))
    {
      mouseEvent->SetScreenLocation(underlyingMouseEvent->ScreenLocation());
      mouseEvent->InitCoordinates({underlyingMouseEvent->ClientX(), underlyingMouseEvent->ClientY()});
    }
    else if (isTrusted)
    {
      // Without an underlying event, only the user agent's events have coordinates; script's
      // (element.click()) are at 0, as in other browsers.
      // FIXME: The target's screen rect (ChromeClient::rootViewToScreen), once the host maps to the screen.
      auto center = target.BoundingClientRect().Center();
      mouseEvent->SetScreenLocation({center.X(), center.Y()});
      mouseEvent->InitCoordinates({center.X(), center.Y()});
    }
    return mouseEvent;
  }

  void MouseEvent::InitMouseEvent(DOMStringAtom type, bool canBubble, bool cancelable, int32 detail,
                                  double screenX, double screenY, double clientX, double clientY,
                                  bool ctrlKey, bool altKey, bool shiftKey, bool metaKey, int16 button,
                                  EventTarget *relatedTarget) noexcept
  {
    if (_dispatched)
    {
      return;
    }

    InitUIEvent(type, canBubble, cancelable, detail);

    SetScreenLocation({FiniteOrZero(screenX), FiniteOrZero(screenY)});
    SetModifierKeys(ctrlKey, altKey, shiftKey, metaKey);
    _button =
      button == static_cast<int16>(MouseButton::None) ? static_cast<int16>(MouseButton::Left) : button;
    _buttonDown = button != static_cast<int16>(MouseButton::None);
    _relatedTarget = ShareRefPtr(relatedTarget);

    InitCoordinates({FiniteOrZero(clientX), FiniteOrZero(clientY)});

    SetIsSimulated(false);
  }

  bool MouseEvent::CanTriggerActivationBehavior(const Event &event) noexcept
  {
    if (!IsAnyClick(event))
    {
      return false;
    }
    auto *mouseEvent = event.IsMouseEvent() ? static_cast<const MouseEvent *>(&event) : nullptr;
    return !mouseEvent || mouseEvent->ButtonType() != MouseButton::Right;
  }

  uint32 MouseEvent::Which() const noexcept
  {
    // For the DOM, the return values for left, middle and right mouse buttons are 0, 1, 2, respectively.
    // For the Netscape "which" property, the return values for left, middle and right mouse buttons are 1, 2,
    // 3, respectively. So we must add 1.
    if (!_buttonDown)
    {
      return 0;
    }
    return static_cast<uint32>(_button + 1);
  }

  RefPtr<Node> MouseEvent::ToElement() const noexcept
  {
    // MSIE extension - "the object toward which the user is moving the mouse pointer"
    if (Type() == EventNames::MouseOut || Type() == EventNames::MouseLeave)
    {
      return AsNode(RelatedTarget());
    }
    return AsNode(Target());
  }

  RefPtr<Node> MouseEvent::FromElement() const noexcept
  {
    // MSIE extension - "object from which activation or the mouse pointer is exiting during the event" (huh?)
    if (Type() == EventNames::MouseOut || Type() == EventNames::MouseLeave)
    {
      return AsNode(Target());
    }
    return AsNode(RelatedTarget());
  }
}
