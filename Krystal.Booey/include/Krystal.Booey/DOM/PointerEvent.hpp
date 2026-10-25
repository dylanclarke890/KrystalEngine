#pragma once

#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Dicts/PointerEventInit.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/DOM/PointerEventTypeNames.hpp"
#include "Krystal.Booey/Platform/PointerID.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <numbers>

namespace krys::boo
{
  class PlatformTouchEvent;
  class PlatformTouchPoint;
}

namespace krys::boo::dom
{
  /// @brief An event from a pointer (a mouse, a pen or a touch): `pointerdown`, `pointerup`, `pointermove`,
  /// `pointercancel`, `pointerover`, `pointerout`, `pointerenter`, `pointerleave`, `gotpointercapture` and
  /// `lostpointercapture`; and `click`, `auxclick` and `contextmenu`.
  /// @see https://w3c.github.io/pointerevents/#pointerevent-interface
  class PointerEvent : public MouseEvent
  {
    friend class EventFactory;

    using dictionary_type = PointerEventInit;

  public:
    enum class IsPrimary : bool
    {
      No,
      Yes
    };

  private:
    struct PointerEventTilt
    {
      int32 TiltX;
      int32 TiltY;
    };

    struct PointerEventAngle
    {
      double AltitudeAngle;
      double AzimuthAngle;
    };

    PointerID _pointerId {MousePointerID};
    double _width {1};
    double _height {1};
    float _pressure {0};
    float _tangentialPressure {0};
    int32 _tiltX {0};
    int32 _tiltY {0};
    int32 _twist {0};
    double _altitudeAngle {std::numbers::pi / 2};
    double _azimuthAngle {0};
    DOMString _pointerType {MousePointerEventType()};
    bool _isPrimary {false};
    bool _fractionalCoordinatesAllowed {true};
    List<Ref<PointerEvent>> _coalescedEvents;
    List<Ref<PointerEvent>> _predictedEvents;

  public:
    /// @see https://w3c.github.io/pointerevents/#dom-pointerevent-constructor
    PointerEvent(DOMStringAtom type, const PointerEventInit &initializer = {}) noexcept;

    ~PointerEvent() noexcept override;

    /// @brief A trusted `gotpointercapture` or `lostpointercapture` event for @p pointerId.
    KRYS_NODISCARD static Ref<PointerEvent> CreateForPointerCapture(DOMStringAtom type, PointerID pointerId,
                                                                    bool isPrimary,
                                                                    const DOMString &pointerType) noexcept;

    /// @brief The pointer event type a mouse event type maps to, or null if none: `pointerdown` for
    /// `mousedown`, say, and `click` for `click`.
    KRYS_NODISCARD static DOMStringAtom TypeFromMouseEventType(DOMStringAtom mouseEventType) noexcept;

    /// @brief The pointer event @p mouseEvent maps to, or null if it maps to none.
    /// @param button The button that changed, or `PointerHasNotChanged`.
    KRYS_NODISCARD static RefPtr<PointerEvent> Create(MouseButton button, const MouseEvent &mouseEvent,
                                                      PointerID pointerId,
                                                      const DOMString &pointerType) noexcept;
    KRYS_NODISCARD static Ref<PointerEvent> Create(DOMStringAtom type, MouseButton button,
                                                   const MouseEvent &mouseEvent, PointerID pointerId,
                                                   const DOMString &pointerType) noexcept;
    KRYS_NODISCARD static Ref<PointerEvent> Create(DOMStringAtom type, MouseButton button,
                                                   const MouseEvent &mouseEvent, PointerID pointerId,
                                                   const DOMString &pointerType, bool canBubble,
                                                   bool isCancelable) noexcept;
    /// @brief A trusted event with no position, as `pointercancel` after a pointer is cancelled.
    KRYS_NODISCARD static Ref<PointerEvent> Create(DOMStringAtom type, PointerID pointerId,
                                                   const DOMString &pointerType,
                                                   IsPrimary isPrimary = IsPrimary::No) noexcept;
    /// @brief The trusted event for the touch at @p touchIndex in @p event: `pointerdown` for a touch
    /// pressed, `pointermove` for one moved or stationary, `pointerup` for one released and `pointercancel`
    /// for one cancelled.
    /// @param view The frame view of the document it's dispatched in.
    KRYS_NODISCARD static Ref<PointerEvent> Create(const PlatformTouchEvent &event,
                                                   const List<Ref<PointerEvent>> &coalescedEvents,
                                                   const List<Ref<PointerEvent>> &predictedEvents,
                                                   uint32 touchIndex, bool isPrimary, LocalFrameView *view,
                                                   const DoublePoint &touchDelta = {}) noexcept;
    KRYS_NODISCARD static Ref<PointerEvent> Create(const PlatformTouchEvent &event,
                                                   const List<Ref<PointerEvent>> &coalescedEvents,
                                                   const List<Ref<PointerEvent>> &predictedEvents,
                                                   bool canBubble, bool isCancelable, uint32 touchIndex,
                                                   bool isPrimary, LocalFrameView *view,
                                                   const DoublePoint &touchDelta = {}) noexcept;
    /// @brief As above, of @p type.
    KRYS_NODISCARD static Ref<PointerEvent> Create(DOMStringAtom type, const PlatformTouchEvent &event,
                                                   const List<Ref<PointerEvent>> &coalescedEvents,
                                                   const List<Ref<PointerEvent>> &predictedEvents,
                                                   uint32 touchIndex, bool isPrimary, LocalFrameView *view,
                                                   const DoublePoint &touchDelta = {}) noexcept;

    /// @brief The pointer id of @p point's touch.
    KRYS_NODISCARD static PointerID PointerIdForTouch(const PlatformTouchPoint &point) noexcept;

    /// @see https://w3c.github.io/pointerevents/#dom-pointerevent-pointerid
    KRYS_NODISCARD PointerID PointerId() const noexcept
    {
      return _pointerId;
    }

    /// @see https://w3c.github.io/pointerevents/#dom-pointerevent-width
    KRYS_NODISCARD double Width() const noexcept
    {
      return _width;
    }

    /// @see https://w3c.github.io/pointerevents/#dom-pointerevent-height
    KRYS_NODISCARD double Height() const noexcept
    {
      return _height;
    }

    /// @see https://w3c.github.io/pointerevents/#dom-pointerevent-pressure
    KRYS_NODISCARD float Pressure() const noexcept
    {
      return _pressure;
    }

    /// @see https://w3c.github.io/pointerevents/#dom-pointerevent-tangentialpressure
    KRYS_NODISCARD float TangentialPressure() const noexcept
    {
      return _tangentialPressure;
    }

    /// @see https://w3c.github.io/pointerevents/#dom-pointerevent-tiltx
    KRYS_NODISCARD int32 TiltX() const noexcept
    {
      return _tiltX;
    }

    /// @see https://w3c.github.io/pointerevents/#dom-pointerevent-tilty
    KRYS_NODISCARD int32 TiltY() const noexcept
    {
      return _tiltY;
    }

    /// @see https://w3c.github.io/pointerevents/#dom-pointerevent-twist
    KRYS_NODISCARD int32 Twist() const noexcept
    {
      return _twist;
    }

    /// @see https://w3c.github.io/pointerevents/#dom-pointerevent-altitudeangle
    KRYS_NODISCARD double AltitudeAngle() const noexcept
    {
      return _altitudeAngle;
    }

    /// @see https://w3c.github.io/pointerevents/#dom-pointerevent-azimuthangle
    KRYS_NODISCARD double AzimuthAngle() const noexcept
    {
      return _azimuthAngle;
    }

    /// @see https://w3c.github.io/pointerevents/#dom-pointerevent-pointertype
    KRYS_NODISCARD const DOMString &PointerType() const noexcept
    {
      return _pointerType;
    }

    /// @see https://w3c.github.io/pointerevents/#dom-pointerevent-isprimary
    KRYS_NODISCARD bool GetIsPrimary() const noexcept
    {
      return _isPrimary;
    }

    KRYS_NODISCARD bool FractionalCoordinatesAllowed() const noexcept
    {
      return _fractionalCoordinatesAllowed;
    }

    KRYS_NODISCARD double OffsetX() const noexcept override;
    KRYS_NODISCARD double OffsetY() const noexcept override;

    KRYS_NODISCARD double ScreenX() const noexcept override
    {
      return AdjustedCoordinateForType(ScreenLocation().X());
    }
    KRYS_NODISCARD double ScreenY() const noexcept override
    {
      return AdjustedCoordinateForType(ScreenLocation().Y());
    }
    KRYS_NODISCARD double ClientX() const noexcept override
    {
      return AdjustedCoordinateForType(ClientLocation().X());
    }
    KRYS_NODISCARD double ClientY() const noexcept override
    {
      return AdjustedCoordinateForType(ClientLocation().Y());
    }
    KRYS_NODISCARD double PageX() const noexcept override
    {
      return AdjustedCoordinateForType(PageLocation().X());
    }
    KRYS_NODISCARD double PageY() const noexcept override
    {
      return AdjustedCoordinateForType(PageLocation().Y());
    }

    /// @see https://w3c.github.io/pointerevents/#dom-pointerevent-getcoalescedevents
    KRYS_NODISCARD List<Ref<PointerEvent>> GetCoalescedEvents() const noexcept;

    /// @see https://w3c.github.io/pointerevents/#dom-pointerevent-getpredictedevents
    KRYS_NODISCARD List<Ref<PointerEvent>> GetPredictedEvents() const noexcept;

    KRYS_NODISCARD bool IsPointerEvent() const noexcept final
    {
      return true;
    }

    // https://w3c.github.io/pointerevents/#attributes-and-default-actions
    // Many user agents expose non-standard attributes fromElement and toElement in MouseEvents to
    // support legacy content. In those user agents, the values of those (inherited) attributes in
    // PointerEvents must be null to encourage the use of the standardized alternates (i.e. target
    // and relatedTarget).
    KRYS_NODISCARD RefPtr<Node> ToElement() const noexcept final
    {
      return nullptr;
    }
    KRYS_NODISCARD RefPtr<Node> FromElement() const noexcept final
    {
      return nullptr;
    }

    /// @brief Whether a pointer event of @p type always says which button changed: `pointerdown`,
    /// `pointerup`, `click`, `auxclick` and `contextmenu`.
    KRYS_NODISCARD static bool TypeRequiresResolvedButton(DOMStringAtom type) noexcept;
    KRYS_NODISCARD static MouseButton ButtonForType(DOMStringAtom type) noexcept
    {
      return !TypeRequiresResolvedButton(type) ? MouseButton::PointerHasNotChanged : MouseButton::Left;
    }

  protected:
    KRYS_NODISCARD static bool TypeCanBubble(DOMStringAtom type) noexcept
    {
      return !TypeIsEnterOrLeave(type);
    }
    KRYS_NODISCARD static bool TypeIsCancelable(DOMStringAtom type) noexcept
    {
      return !TypeIsEnterOrLeave(type);
    }
    KRYS_NODISCARD static bool TypeIsComposed(DOMStringAtom type) noexcept
    {
      return !TypeIsEnterOrLeave(type);
    }

    PointerEvent(DOMStringAtom type, MouseButton button, const MouseEvent &mouseEvent, PointerID pointerId,
                 const DOMString &pointerType, bool canBubble, bool isCancelable, bool isComposed) noexcept;

    void ReceivedTarget() noexcept override;

  private:
    KRYS_NODISCARD static bool TypeIsEnterOrLeave(DOMStringAtom type) noexcept
    {
      return type == EventNames::PointerEnter || type == EventNames::PointerLeave;
    }

    KRYS_NODISCARD static uint16 ButtonsForType(DOMStringAtom type) noexcept
    {
      // We have contact with the touch surface for most events except when we've released the touch or
      // canceled it.
      return (type == EventNames::PointerUp || type == EventNames::PointerOut
              || type == EventNames::PointerLeave || type == EventNames::PointerCancel)
               ? 0
               : 1;
    }

    KRYS_NODISCARD static float PressureForPressureInsensitiveInputDevices(uint16 buttons) noexcept
    {
      // https://www.w3.org/TR/pointerevents/#dfn-active-buttons-state
      bool isInActiveButtonsState = buttons;
      // https://www.w3.org/TR/pointerevents/#dom-pointerevent-pressure
      return isInActiveButtonsState ? 0.5f : 0;
    }

    KRYS_NODISCARD static PointerEventAngle AngleFromTilt(int32 tiltX, int32 tiltY) noexcept;
    KRYS_NODISCARD static PointerEventTilt TiltFromAngle(double altitudeAngle, double azimuthAngle) noexcept;

    KRYS_NODISCARD static bool FractionalCoordinatesAllowedForType(DOMStringAtom type) noexcept;
    KRYS_NODISCARD double AdjustedCoordinateForType(double coordinate) const noexcept;

    PointerEvent(DOMStringAtom type, PointerID pointerId, const DOMString &pointerType,
                 IsPrimary isPrimary) noexcept;
    PointerEvent(DOMStringAtom type, const PlatformTouchEvent &event,
                 const List<Ref<PointerEvent>> &coalescedEvents,
                 const List<Ref<PointerEvent>> &predictedEvents, bool canBubble, bool isCancelable,
                 uint32 touchIndex, bool isPrimary, LocalFrameView *view,
                 const DoublePoint &touchDelta) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::PointerEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsPointerEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
