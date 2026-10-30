#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Page/MouseEventTypes.hpp"
#include "Krystal.Booey/Platform/PointerID.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Document;
  class Element;
  class EventTarget;
  class MouseEvent;
  class PointerEvent;
}

namespace krys::boo
{
  class DoublePoint;
  class LocalFrameView;
  class PlatformTouchEvent;
}

namespace krys::boo
{
  class IntPoint;
  class Page;

  /// @brief A page's pointers: which element captures each one (`setPointerCapture()`), the
  /// `gotpointercapture` and `lostpointercapture` events as capture changes, whether a cancelled
  /// `pointerdown` stops the mouse's compatibility events, and the pointer events a mouse's events map to.
  /// @see https://w3c.github.io/pointerevents/#pointer-capture
  class PointerCaptureController
  {
    KRYS_NON_COPY_MOVABLE(PointerCaptureController);

    struct CapturingData : public RefCounted<CapturingData>
    {
      enum class State : uint8
      {
        Ready,
        Finished,
        Cancelled,
      };

      WeakPtr<dom::Document> ActiveDocument;
      RefPtr<dom::Element> PendingTargetOverride;
      RefPtr<dom::Element> TargetOverride;
      RefPtr<dom::Element> PreviousTarget;
      dom::DOMString PointerType;
      State CurrentState {State::Ready};
      bool IsPrimary {false};
      bool PreventsCompatibilityMouseEvents {false};
      bool PointerIsPressed {false};
      MouseButton PreviousMouseButton {MouseButton::PointerHasNotChanged};

      explicit CapturingData(const dom::DOMString &pointerType) noexcept : PointerType(pointerType)
      {
      }

      KRYS_NODISCARD static Ref<CapturingData> Create(const dom::DOMString &pointerType) noexcept
      {
        return AdoptRef(*new CapturingData(pointerType));
      }

      KRYS_NODISCARD bool HasAnyElement() const noexcept
      {
        return PendingTargetOverride || TargetOverride || PreviousTarget;
      }
    };

  private:
    WeakPtr<Page> _page;
    // While PointerID is defined as int32_t, we use int64_t here so that we may use a value outside of the
    // int32_t range to have safe empty and removed values, allowing any int32_t to be provided through the
    // API for lookup in this hashmap.
    HashMap<int64, Ref<CapturingData>> _activePointerIdsToCapturingData;
    bool _processingPendingPointerCapture {false};
    bool _haveAnyCapturingElement {false};

  public:
    explicit PointerCaptureController(Page &page) noexcept;

    /// @brief The element capturing @p pointerId, if it's in @p document.
    KRYS_NODISCARD dom::Element *PointerCaptureElement(dom::Document *document,
                                                       PointerID pointerId) const noexcept;
    /// @see https://w3c.github.io/pointerevents/#setting-pointer-capture
    dom::ExceptionOr<void> SetPointerCapture(dom::Element *capturingTarget, PointerID pointerId) noexcept;
    /// @see https://w3c.github.io/pointerevents/#releasing-pointer-capture
    dom::ExceptionOr<void> ReleasePointerCapture(dom::Element *capturingTarget, PointerID pointerId) noexcept;
    /// @see https://w3c.github.io/pointerevents/#dom-element-haspointercapture
    KRYS_NODISCARD bool HasPointerCapture(dom::Element *capturingTarget, PointerID pointerId) const noexcept;
    /// @brief Forgets every pointer but the mouse, as when the page's main document changes.
    void Reset() noexcept;

    /// @brief Releases every capture, as a pointer lock does.
    void PointerLockWasApplied() noexcept;
    void ElementWasRemoved(dom::Element &element) noexcept
    {
      if (_haveAnyCapturingElement)
      {
        ElementWasRemovedSlow(element);
      }
    }

    /// @brief The pointer event to dispatch before @p mouseEvent, or null if none.
    KRYS_NODISCARD RefPtr<dom::PointerEvent>
      PointerEventForMouseEvent(const dom::MouseEvent &mouseEvent, PointerID pointerId,
                                const dom::DOMString &pointerType) noexcept;

    /// @brief Dispatches the pointer events for the touch at @p index in @p event at @p target: `pointerover`
    /// and `pointerenter` before a `pointerdown`, the boundary events if a move changed the target, and
    /// `pointerout` and `pointerleave` after a `pointerup`.
    /// @param view The frame view of @p target's document.
    void DispatchEventForTouchAtIndex(dom::EventTarget &target, const PlatformTouchEvent &event, uint32 index,
                                      bool isPrimary, LocalFrameView *view,
                                      const DoublePoint &touchDelta) noexcept;

    void TouchWithIdentifierWasRemoved(PointerID pointerId) noexcept;
    KRYS_NODISCARD bool HasCancelledPointerEventForIdentifier(PointerID pointerId) const noexcept;
    KRYS_NODISCARD bool PreventsCompatibilityMouseEventsForIdentifier(PointerID pointerId) const noexcept;
    /// @brief Dispatches @p event at @p target, keeping the pointer's state.
    void DispatchEvent(dom::PointerEvent &event, dom::EventTarget *target) noexcept;
    /// @brief Fires `pointercancel`, `pointerout` and `pointerleave` for @p pointerId, at its capturing
    /// element or else the element at @p documentPoint, and releases its capture.
    /// @see https://w3c.github.io/pointerevents/#the-pointercancel-event
    void CancelPointer(PointerID pointerId, const IntPoint &documentPoint,
                       dom::PointerEvent *existingCancelEvent = nullptr) noexcept;
    /// @see https://w3c.github.io/pointerevents/#process-pending-pointer-capture
    void ProcessPendingPointerCapture(PointerID pointerId) noexcept;
    // Used for mouse presses that trigger contextmenu, causing
    // the matching release to be suppressed.
    void ClearUnmatchedMouseDown(PointerID pointerId) noexcept;

  private:
    KRYS_NODISCARD RefPtr<CapturingData> CapturingDataFor(PointerID pointerId) const noexcept;
    Ref<CapturingData> EnsureCapturingDataForPointerEvent(const dom::PointerEvent &event) noexcept;
    void PointerEventWillBeDispatched(const dom::PointerEvent &event, dom::EventTarget *target) noexcept;
    void PointerEventWasDispatched(const dom::PointerEvent &event) noexcept;

    void DispatchOverOrOutEvent(dom::DOMStringAtom type, dom::EventTarget *target,
                                const PlatformTouchEvent &event, uint32 index, bool isPrimary,
                                LocalFrameView *view, const DoublePoint &touchDelta) noexcept;
    void DispatchEnterOrLeaveEvent(dom::DOMStringAtom type, dom::Element &targetElement,
                                   const PlatformTouchEvent &event, uint32 index, bool isPrimary,
                                   LocalFrameView *view, const DoublePoint &touchDelta) noexcept;

    void UpdateHaveAnyCapturingElement() noexcept;
    void ElementWasRemovedSlow(dom::Element &element) noexcept;
  };
}
