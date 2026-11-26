#pragma once

#include "Krystal.Booey/DOM/Events/EventTarget.hpp"
#include "Krystal.Booey/Events/Pointer/MouseRelatedEvent.hpp"
#include "Krystal.Booey/Events/UI/UIEvent.hpp"
#include "Krystal.Booey/Input/MouseEventTypes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::pointer_events
{
  /// @brief Whether @p eventType is `click` or `auxclick`.
  KRYS_NODISCARD bool IsAnyClick(const DOMStringAtom &eventType) noexcept;

  KRYS_NODISCARD bool IsAnyClick(const Event &event) noexcept;
}

namespace krys::boo
{
  class Element;
  class Node;
  class PlatformMouseEvent;

  /// @see https://w3c.github.io/pointerevents/#dom-mouseeventinit
  struct MouseEventInit : EventModifierInit
  {
    double ScreenX {0};
    double ScreenY {0};
    /// @see https://w3c.github.io/pointerlock/#extensions-to-the-mouseeventinit-dictionary
    double MovementX {0};
    double MovementY {0};
    double ClientX {0};
    double ClientY {0};
    int16 Button {0};
    uint16 Buttons {0};
    RefPtr<EventTarget> RelatedTarget;
  };

  /// @brief An event from a pointing device: `mousedown`, `mouseup`, `mousemove`, `mouseover`, `mouseout`,
  /// `mouseenter`, `mouseleave` and `dblclick`; `click`, `auxclick` and `contextmenu` are `PointerEvent`s,
  /// and `wheel` is a `WheelEvent`.
  /// @see https://w3c.github.io/pointerevents/#idl-def-mouseevent
  class MouseEvent : public MouseRelatedEvent
  {
  private:
    int16 _button {static_cast<int16>(MouseButton::Left)};
    uint16 _buttons {0};
    SyntheticClickType _syntheticClickType {SyntheticClickType::NoTap};
    bool _buttonDown {false};
    RefPtr<EventTarget> _relatedTarget;
    double _force {0};
    PenOrientation _penOrientation;
    List<Ref<MouseEvent>> _coalescedEvents;
    List<Ref<MouseEvent>> _predictedEvents;

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = MouseEventInit;

    /// @see https://w3c.github.io/pointerevents/#dom-mouseevent-constructor
    MouseEvent(DOMStringAtom type, const MouseEventInit &initializer = {}) noexcept;

    /// @brief A trusted event of type @p eventType from @p event, which happened in @p view.
    /// @param coalescedEvents The events for the moves @p event stands for.
    /// @param predictedEvents The events for the moves predicted after @p event.
    KRYS_NODISCARD static Ref<MouseEvent> Create(const DOMStringAtom &eventType, boo::FrameView *view,
                                                 const PlatformMouseEvent &event,
                                                 const List<Ref<MouseEvent>> &coalescedEvents,
                                                 const List<Ref<MouseEvent>> &predictedEvents, int32 detail,
                                                 Node *relatedTarget) noexcept;

    /// @brief A trusted event from its parts, with its client location rather than its window location.
    KRYS_NODISCARD static Ref<MouseEvent>
      Create(DOMStringAtom eventType, bool canBubble, bool isCancelable, bool isComposed,
             MonotonicTime::duration timestamp, boo::FrameView *view, int32 detail, double screenX,
             double screenY, double clientX, double clientY, EnumSet<Modifier> modifiers, MouseButton button,
             uint16 buttons, SyntheticClickType syntheticClickType, EventTarget *relatedTarget) noexcept;

    /// @brief A simulated press, release or click of the left button at @p target, standing in for
    /// @p underlyingEvent (a key press, say), with its position and modifiers.
    /// @param isTrusted Whether the user agent simulates it, rather than script.
    KRYS_NODISCARD static Ref<MouseEvent> CreateSimulated(DOMStringAtom eventType, Event *underlyingEvent,
                                                          Element &target, bool isTrusted) noexcept;

    /// @note Legacy.
    /// @see https://w3c.github.io/pointerevents/#dom-mouseevent-initmouseevent
    void InitMouseEvent(DOMStringAtom type, bool canBubble, bool cancelable, Window *view, int32 detail,
                        double screenX, double screenY, double clientX, double clientY, bool ctrlKey,
                        bool altKey, bool shiftKey, bool metaKey, int16 button,
                        EventTarget *relatedTarget) noexcept;

    /// @see https://w3c.github.io/pointerevents/#dom-mouseevent-button
    KRYS_NODISCARD int16 Button() const noexcept
    {
      return _button;
    }

    /// @brief `Button()` as a `MouseButton`.
    KRYS_NODISCARD MouseButton ButtonType() const noexcept
    {
      return ButtonFromShort(_button);
    }

    /// @see https://w3c.github.io/pointerevents/#dom-mouseevent-buttons
    KRYS_NODISCARD uint16 Buttons() const noexcept
    {
      return _buttons;
    }

    /// @brief For a click the host made from a tap, whether it was a one or two finger tap; `NoTap`
    /// otherwise.
    KRYS_NODISCARD SyntheticClickType GetSyntheticClickType() const noexcept
    {
      return _syntheticClickType;
    }

    /// @brief Whether a button is involved: false for a move with no button pressed, whose `Button()` still
    /// reads 0.
    KRYS_NODISCARD bool ButtonDown() const noexcept
    {
      return _buttonDown;
    }

    /// @brief The pressure, for a pressure sensitive device.
    KRYS_NODISCARD double Force() const noexcept
    {
      return _force;
    }

    void SetForce(double force) noexcept
    {
      _force = force;
    }

    /// @brief How the pen is held, for a pen's events, which its pointer events report.
    KRYS_NODISCARD const PenOrientation &GetPenOrientation() const noexcept
    {
      return _penOrientation;
    }

    /// @see https://w3c.github.io/pointerevents/#dom-mouseevent-relatedtarget
    KRYS_NODISCARD RefPtr<EventTarget> RelatedTarget() const noexcept override
    {
      return _relatedTarget;
    }

    /// @note Legacy: the button, counted from one, or zero if none was pressed.
    KRYS_NODISCARD uint32 Which() const noexcept;

    /// @brief The moves this event stands for, and those the host predicts after it, which a pointer event
    /// made from it reports.
    /// @see https://w3c.github.io/pointerevents/#dom-pointerevent-getcoalescedevents
    KRYS_NODISCARD const List<Ref<MouseEvent>> &CoalescedEvents() const noexcept
    {
      return _coalescedEvents;
    }

    KRYS_NODISCARD const List<Ref<MouseEvent>> &PredictedEvents() const noexcept
    {
      return _predictedEvents;
    }

    /// @brief Whether @p event is a click that may run activation behavior (not one from the right button).
    KRYS_NODISCARD static bool CanTriggerActivationBehavior(const Event &event) noexcept;

    KRYS_NODISCARD bool IsMouseEvent() const noexcept final
    {
      return true;
    }

  protected:
    void RelatedTarget(EventTarget *relatedTarget) noexcept override
    {
      _relatedTarget = ShareRefPtr(relatedTarget);
    }

    /// @brief A trusted event; the `Create` functions and derived events build on it.
    MouseEvent(DOMStringAtom type, const UIEventInit &eventInitDict, boo::FrameView *view,
               const PointD &screenLocation, const PointD &windowLocation, double movementX, double movementY,
               EnumSet<Modifier> modifiers, MouseButton button, uint16 buttons, EventTarget *relatedTarget,
               double force, SyntheticClickType syntheticClickType,
               const List<Ref<MouseEvent>> &coalescedEvents, const List<Ref<MouseEvent>> &predictedEvents,
               Simulated isSimulated) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::MouseEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::Event &event) noexcept
  {
    return event.IsMouseEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
