#include "Krystal.Booey/Events/Pointer/MouseEvent.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/Node.hpp"
#include "Krystal.Booey/Input/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Page/Frame.hpp"
#include "Krystal.Booey/Page/FrameView.hpp"
#include <cmath>

namespace krys::boo
{
  namespace
  {
    /// @brief A coordinate a page gave, with infinities and NaN as 0.
    KRYS_NODISCARD double FiniteOrZero(double value) noexcept
    {
      return std::isfinite(value) ? value : 0;
    }

    /// @brief The `button` value of @p button. With no button involved, `button` reads as the left (0) and
    /// only `ButtonDown()` tells the difference.
    KRYS_NODISCARD int16 ButtonValue(int16 button) noexcept
    {
      return button == static_cast<int16>(MouseButton::None) ? static_cast<int16>(MouseButton::Left) : button;
    }
  }

  bool pointer_events::IsAnyClick(const DOMStringAtom &eventType) noexcept
  {
    return eventType == event_names::Click || eventType == event_names::AuxClick;
  }

  bool pointer_events::IsAnyClick(const Event &event) noexcept
  {
    return pointer_events::IsAnyClick(event.Type());
  }

  MouseEvent::MouseEvent(DOMStringAtom type, const MouseEventInit &initializer) noexcept
      : MouseRelatedEvent(krys::move(type), initializer), _button(ButtonValue(initializer.Button)),
        _buttons(initializer.Buttons),
        _buttonDown(initializer.Button != static_cast<int16>(MouseButton::None)),
        _relatedTarget(initializer.RelatedTarget)
  {
    InitCoordinates({FiniteOrZero(initializer.ClientX), FiniteOrZero(initializer.ClientY)});
  }

  MouseEvent::MouseEvent(DOMStringAtom type, const UIEventInit &eventInitDict, boo::FrameView *view,
                         const PointD &screenLocation, const PointD &windowLocation, double movementX,
                         double movementY, EnumSet<Modifier> modifiers, MouseButton button, uint16 buttons,
                         EventTarget *relatedTarget, double force, SyntheticClickType syntheticClickType,
                         const List<Ref<MouseEvent>> &coalescedEvents,
                         const List<Ref<MouseEvent>> &predictedEvents, Simulated isSimulated) noexcept
      : MouseRelatedEvent(krys::move(type), eventInitDict, view, screenLocation, windowLocation, movementX,
                          movementY, modifiers, isSimulated),
        _button(ButtonValue(static_cast<int16>(button))), _buttons(buttons),
        _syntheticClickType(syntheticClickType), _buttonDown(button != MouseButton::None),
        _relatedTarget(ShareRefPtr(relatedTarget)), _force(force), _coalescedEvents(coalescedEvents),
        _predictedEvents(predictedEvents)
  {
    _isTrusted = true;
  }

  Ref<MouseEvent> MouseEvent::Create(const DOMStringAtom &eventType, boo::FrameView *view,
                                     const PlatformMouseEvent &event,
                                     const List<Ref<MouseEvent>> &coalescedEvents,
                                     const List<Ref<MouseEvent>> &predictedEvents, int32 detail,
                                     Node *relatedTarget) noexcept
  {
    // mouseenter and mouseleave go only to the element entered or left; the other types bubble, can be
    // canceled and leave shadow trees.
    bool bubbles = eventType != event_names::MouseEnter && eventType != event_names::MouseLeave;
    UIEventInit init;
    init.Bubbles = bubbles;
    init.Cancelable = bubbles;
    init.Composed = bubbles;
    init.Detail = detail;

    Ref<MouseEvent> mouseEvent = AdoptRef(*new MouseEvent(
      eventType, init, view, event.GlobalPosition(), event.Position(), event.MovementDelta().X(),
      event.MovementDelta().Y(), event.Modifiers(), event.Button(), event.Buttons(), relatedTarget,
      event.Force(), event.GetSyntheticClickType(), coalescedEvents, predictedEvents, Simulated::No));
    mouseEvent->_createTime = event.Timestamp();
    mouseEvent->_penOrientation = event.GetPenOrientation();
    return mouseEvent;
  }

  Ref<MouseEvent> MouseEvent::Create(DOMStringAtom eventType, bool canBubble, bool isCancelable,
                                     bool isComposed, MonotonicTime::duration timestamp, boo::FrameView *view,
                                     int32 detail, double screenX, double screenY, double clientX,
                                     double clientY, EnumSet<Modifier> modifiers, MouseButton button,
                                     uint16 buttons, SyntheticClickType syntheticClickType,
                                     EventTarget *relatedTarget) noexcept
  {
    UIEventInit init;
    init.Bubbles = canBubble;
    init.Cancelable = isCancelable;
    init.Composed = isComposed;
    init.Detail = detail;

    Ref<MouseEvent> mouseEvent = AdoptRef(*new MouseEvent(
      krys::move(eventType), init, view, {FiniteOrZero(screenX), FiniteOrZero(screenY)}, {}, 0, 0, modifiers,
      button, buttons, relatedTarget, 0, syntheticClickType, {}, {}, Simulated::No));
    mouseEvent->InitCoordinates({FiniteOrZero(clientX), FiniteOrZero(clientY)});
    mouseEvent->_createTime = timestamp;
    return mouseEvent;
  }

  Ref<MouseEvent> MouseEvent::CreateSimulated(DOMStringAtom eventType, Event *underlyingEvent,
                                              Element &target, bool isTrusted) noexcept
  {
    UIEventInit init;
    init.Bubbles = true;
    init.Cancelable = true;
    init.Composed = true;

    // The modifiers are those of the key press or click it stands in for.
    EnumSet<Modifier> modifiers;
    if (auto *eventWithKeyState = ui_events::FindEventWithKeyState(underlyingEvent))
    {
      modifiers = eventWithKeyState->ModifierKeys();
    }

    auto *frame = target.NodeDocument().Frame();
    Ref<MouseEvent> mouseEvent = AdoptRef(
      *new MouseEvent(krys::move(eventType), init, frame ? frame->View() : nullptr, {}, {}, 0, 0, modifiers,
                      MouseButton::Left, 0, nullptr, 0, SyntheticClickType::NoTap, {}, {}, Simulated::Yes));
    mouseEvent->_isTrusted = isTrusted;
    if (underlyingEvent)
    {
      mouseEvent->_createTime = underlyingEvent->TimeStamp();
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
      auto center = target.BoundingClientRect().Center();
      // The client rect lays the target out, which its screen rect needs.
      auto screenCenter = target.ScreenRect().Center();
      mouseEvent->SetScreenLocation(
        {static_cast<double>(screenCenter.X()), static_cast<double>(screenCenter.Y())});
      mouseEvent->InitCoordinates({center.X(), center.Y()});
    }
    return mouseEvent;
  }

  void MouseEvent::InitMouseEvent(DOMStringAtom type, bool canBubble, bool cancelable, Window *view,
                                  int32 detail, double screenX, double screenY, double clientX,
                                  double clientY, bool ctrlKey, bool altKey, bool shiftKey, bool metaKey,
                                  int16 button, EventTarget *relatedTarget) noexcept
  {
    // As initUIEvent: nothing changes while the event is being dispatched.
    if (_dispatched)
    {
      return;
    }

    InitUIEvent(krys::move(type), canBubble, cancelable, view, detail);
    // The coordinates are converted with the new view's frame view.
    UpdateFrameView();

    SetScreenLocation({FiniteOrZero(screenX), FiniteOrZero(screenY)});
    SetModifierKeys(ctrlKey, altKey, shiftKey, metaKey);
    _button = ButtonValue(button);
    _buttonDown = button != static_cast<int16>(MouseButton::None);
    _relatedTarget = ShareRefPtr(relatedTarget);

    InitCoordinates({FiniteOrZero(clientX), FiniteOrZero(clientY)});
    // An event the page initialized has the coordinates it was given, even if it was simulated before.
    SetIsSimulated(false);
  }

  bool MouseEvent::CanTriggerActivationBehavior(const Event &event) noexcept
  {
    if (!pointer_events::IsAnyClick(event))
    {
      return false;
    }
    auto *mouseEvent = DynamicDowncast<MouseEvent>(event);
    return !mouseEvent || mouseEvent->ButtonType() != MouseButton::Right;
  }

  uint32 MouseEvent::Which() const noexcept
  {
    // button + 1, so the left button is 1; 0 when no button is involved.
    if (!_buttonDown)
    {
      return 0;
    }
    return static_cast<uint32>(_button + 1);
  }
}
