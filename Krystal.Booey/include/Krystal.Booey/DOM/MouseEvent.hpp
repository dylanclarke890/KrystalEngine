#pragma once

#include "Krystal.Booey/DOM/Dicts/MouseEventInit.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Booey/DOM/MouseRelatedEvent.hpp"
#include "Krystal.Booey/Page/MouseEventTypes.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class PlatformMouseEvent;
}

namespace krys::boo::dom
{
  class Element;
  class Node;

  /// @brief Whether @p eventType is `click` or `auxclick`.
  KRYS_NODISCARD bool IsAnyClick(DOMStringAtom eventType) noexcept;
  KRYS_NODISCARD bool IsAnyClick(const Event &event) noexcept;

  /// @brief An event from a pointing device: `mousedown`, `mouseup`, `mousemove`, `mouseover`, `mouseout`,
  /// `mouseenter`, `mouseleave`, `click`, `auxclick` and `dblclick`; `wheel` is a `WheelEvent`.
  /// @see https://w3c.github.io/uievents/#interface-mouseevent
  // FIXME: Force, synthetic clicks, and coalesced and predicted events come with pointer events.
  class MouseEvent : public MouseRelatedEvent
  {
    friend class EventFactory;

    using dictionary_type = MouseEventInit;

  private:
    int16 _button {static_cast<int16>(MouseButton::Left)};
    uint16 _buttons {0};
    bool _buttonDown {false};
    RefPtr<EventTarget> _relatedTarget;

  public:
    /// @see https://w3c.github.io/uievents/#dom-mouseevent-mouseevent
    MouseEvent(DOMStringAtom type, const MouseEventInit &initializer = {}) noexcept;

    /// @brief A trusted event of type @p eventType from @p event, which happened in @p view.
    KRYS_NODISCARD static Ref<MouseEvent> Create(DOMStringAtom eventType, LocalFrameView *view,
                                                 const PlatformMouseEvent &event, int32 detail,
                                                 Node *relatedTarget) noexcept;

    /// @brief A trusted event from its parts, with its client location rather than its window location.
    KRYS_NODISCARD static Ref<MouseEvent> Create(DOMStringAtom eventType, bool canBubble, bool isCancelable,
                                                 bool isComposed, DOMHighResTimeStamp timestamp,
                                                 LocalFrameView *view, int32 detail, double screenX,
                                                 double screenY, double clientX, double clientY,
                                                 EnumSet<Modifier> modifiers, MouseButton button,
                                                 uint16 buttons, EventTarget *relatedTarget) noexcept;

    /// @brief A simulated press, release or click of the left button at @p target, standing in for
    /// @p underlyingEvent (a key press, say), with its position and modifiers.
    /// @param isTrusted Whether the user agent simulates it, rather than script.
    KRYS_NODISCARD static Ref<MouseEvent> CreateSimulated(DOMStringAtom eventType, Event *underlyingEvent,
                                                          Element &target, bool isTrusted) noexcept;

    /// @note Legacy.
    /// @see https://w3c.github.io/uievents/#dom-mouseevent-initmouseevent
    void InitMouseEvent(DOMStringAtom type, bool canBubble, bool cancelable, int32 detail, double screenX,
                        double screenY, double clientX, double clientY, bool ctrlKey, bool altKey,
                        bool shiftKey, bool metaKey, int16 button, EventTarget *relatedTarget) noexcept;

    /// @see https://w3c.github.io/uievents/#dom-mouseevent-button
    KRYS_NODISCARD int16 Button() const noexcept
    {
      return _button;
    }

    KRYS_NODISCARD MouseButton ButtonType() const noexcept
    {
      return ButtonFromShort(_button);
    }

    /// @see https://w3c.github.io/uievents/#dom-mouseevent-buttons
    KRYS_NODISCARD uint16 Buttons() const noexcept
    {
      return _buttons;
    }

    KRYS_NODISCARD bool ButtonDown() const noexcept
    {
      return _buttonDown;
    }

    /// @see https://w3c.github.io/uievents/#dom-mouseevent-relatedtarget
    KRYS_NODISCARD RefPtr<EventTarget> RelatedTarget() const noexcept override
    {
      return _relatedTarget;
    }

    /// @note Legacy: the node the pointer moved to.
    KRYS_NODISCARD RefPtr<Node> ToElement() const noexcept;
    /// @note Legacy: the node the pointer moved from.
    KRYS_NODISCARD RefPtr<Node> FromElement() const noexcept;

    /// @note Legacy: the button, counted from one, or zero if none was pressed.
    KRYS_NODISCARD uint32 Which() const noexcept;

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

    MouseEvent(DOMStringAtom type, const UIEventInit &eventInitDict, LocalFrameView *view,
               const DoublePoint &screenLocation, const DoublePoint &windowLocation, double movementX,
               double movementY, EnumSet<Modifier> modifiers, MouseButton button, uint16 buttons,
               EventTarget *relatedTarget, Simulated isSimulated) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::MouseEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsMouseEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
