#include "Krystal.Booey/Page/PointerCaptureController.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/PointerEvent.hpp"
#include "Krystal.Booey/HTML/Enums/TaskSource.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/PointerLockController.hpp"
#include "Krystal.Booey/Platform/PlatformTouchEvent.hpp"
#include "Krystal.Booey/Rendering/HitTestRequest.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include <algorithm>

namespace krys::boo
{
  PointerCaptureController::PointerCaptureController(Page &page) noexcept
      : _page(WeakPtr<Page>::WithRef(page))
  {
    Reset();
  }

  auto PointerCaptureController::CapturingDataFor(PointerID pointerId) const noexcept -> RefPtr<CapturingData>
  {
    auto it = _activePointerIdsToCapturingData.find(pointerId);
    if (it == _activePointerIdsToCapturingData.end())
    {
      return nullptr;
    }
    return it->second;
  }

  dom::Element *PointerCaptureController::PointerCaptureElement(dom::Document *document,
                                                                PointerID pointerId) const noexcept
  {
    if (auto capturingData = CapturingDataFor(pointerId))
    {
      if (capturingData->TargetOverride && &capturingData->TargetOverride->NodeDocument() == document)
      {
        return capturingData->TargetOverride.get();
      }
    }
    return nullptr;
  }

  dom::ExceptionOr<void> PointerCaptureController::SetPointerCapture(dom::Element *capturingTarget,
                                                                     PointerID pointerId) noexcept
  {
    // https://w3c.github.io/pointerevents/#setting-pointer-capture

    // 1. If the pointerId provided as the method's argument does not match any of the active pointers, then
    // throw a DOMException with the name NotFoundError.
    auto capturingData = CapturingDataFor(pointerId);
    if (!capturingData)
    {
      return dom::Exception {dom::ExceptionCode::NotFoundError};
    }

    // 2. If the Element on which this method is invoked is not connected, throw an exception with the name
    // InvalidStateError.
    if (!capturingTarget->IsConnected())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError};
    }

    // 3. If this method is invoked while the document has a locked element, throw an exception with the name
    // InvalidStateError.
    if (auto *page = capturingTarget->NodeDocument().GetPage())
    {
      if (page->GetPointerLockController().IsLocked())
      {
        return dom::Exception {dom::ExceptionCode::InvalidStateError};
      }
    }

    // 4. If the pointer is not in the active buttons state or the element's node document is not the active
    // document of the pointer, then terminate these steps.
    if (!capturingData->PointerIsPressed
        || &capturingTarget->NodeDocument() != capturingData->ActiveDocument.get())
    {
      return {};
    }

    // 5. For the specified pointerId, set the pending pointer capture target override to the Element on which
    // this method was invoked.
    capturingData->PendingTargetOverride = ShareRefPtr(capturingTarget);

    UpdateHaveAnyCapturingElement();
    return {};
  }

  dom::ExceptionOr<void> PointerCaptureController::ReleasePointerCapture(dom::Element *capturingTarget,
                                                                         PointerID pointerId) noexcept
  {
    // https://w3c.github.io/pointerevents/#releasing-pointer-capture

    // Pointer capture is released on an element explicitly by calling the
    // element.releasePointerCapture(pointerId) method. When this method is called, a user agent MUST run the
    // following steps:

    // 1. If the pointerId provided as the method's argument does not match any of the active pointers and
    // these steps are not being invoked as a result of the implicit release of pointer capture, then throw a
    // DOMException with the name NotFoundError.
    auto capturingData = CapturingDataFor(pointerId);
    if (!capturingData)
    {
      return dom::Exception {dom::ExceptionCode::NotFoundError};
    }

    // 2. If hasPointerCapture is false for the Element with the specified pointerId, then terminate these
    // steps.
    if (!HasPointerCapture(capturingTarget, pointerId))
    {
      return {};
    }

    // 3. For the specified pointerId, clear the pending pointer capture target override, if set.
    capturingData->PendingTargetOverride = nullptr;

    // FIXME: This leaves value.targetOverride set: webkit.org/b/221342.

    UpdateHaveAnyCapturingElement();
    return {};
  }

  bool PointerCaptureController::HasPointerCapture(dom::Element *capturingTarget,
                                                   PointerID pointerId) const noexcept
  {
    // https://w3c.github.io/pointerevents/#dom-element-haspointercapture

    // Indicates whether the element on which this method is invoked has pointer capture for the pointer
    // identified by the argument pointerId. In particular, returns true if the pending pointer capture target
    // override for pointerId is set to the element on which this method is invoked, and false otherwise.

    if (!_haveAnyCapturingElement)
    {
      return false;
    }

    auto capturingData = CapturingDataFor(pointerId);
    return capturingData && capturingData->PendingTargetOverride.get() == capturingTarget;
  }

  void PointerCaptureController::PointerLockWasApplied() noexcept
  {
    // https://w3c.github.io/pointerevents/#implicit-release-of-pointer-capture

    // When a pointer lock is successfully applied on an element, a user agent MUST run the steps as if the
    // releasePointerCapture() method has been called if any element is set to be captured or pending to be
    // captured.
    for (auto &[pointerId, capturingData] : _activePointerIdsToCapturingData)
    {
      capturingData->PendingTargetOverride = nullptr;
      capturingData->TargetOverride = nullptr;
    }

    UpdateHaveAnyCapturingElement();
  }

  void PointerCaptureController::ElementWasRemovedSlow(dom::Element &element) noexcept
  {
    krys_debug_assert(_haveAnyCapturingElement);

    for (auto &[pointerId, capturingData] : _activePointerIdsToCapturingData)
    {
      if (capturingData->PendingTargetOverride.get() == &element
          || capturingData->TargetOverride.get() == &element)
      {
        // https://w3c.github.io/pointerevents/#implicit-release-of-pointer-capture
        // When the pointer capture target override is no longer connected, the pending pointer capture target
        // override and pointer capture target override nodes SHOULD be cleared and also a PointerEvent named
        // lostpointercapture corresponding to the captured pointer SHOULD be fired at the document.
        auto id = static_cast<PointerID>(pointerId);
        auto pointerType = capturingData->PointerType;
        (void)ReleasePointerCapture(&element, id);
        // FIXME: Spec doesn't specify which task source to use.
        element.NodeDocument().QueueTaskToDispatchEvent(
          html::TaskSource::UserInteraction,
          dom::PointerEvent::Create(dom::DOMStringAtom(EventNames::LostPointerCapture), id, pointerType));
        return;
      }
    }
  }

  void PointerCaptureController::Reset() noexcept
  {
    _activePointerIdsToCapturingData.clear();
    _haveAnyCapturingElement = false;

    _activePointerIdsToCapturingData.emplace(MousePointerID,
                                             CapturingData::Create(dom::MousePointerEventType()));
  }

  void PointerCaptureController::UpdateHaveAnyCapturingElement() noexcept
  {
    _haveAnyCapturingElement = std::ranges::any_of(_activePointerIdsToCapturingData, [&](const auto &entry)
                                                   { return entry.second->HasAnyElement(); });
  }

  void PointerCaptureController::TouchWithIdentifierWasRemoved(PointerID pointerId) noexcept
  {
    _activePointerIdsToCapturingData.erase(pointerId);
    UpdateHaveAnyCapturingElement();
  }

  bool PointerCaptureController::HasCancelledPointerEventForIdentifier(PointerID pointerId) const noexcept
  {
    auto capturingData = CapturingDataFor(pointerId);
    return capturingData && capturingData->CurrentState == CapturingData::State::Cancelled;
  }

  bool PointerCaptureController::PreventsCompatibilityMouseEventsForIdentifier(
    PointerID pointerId) const noexcept
  {
    auto capturingData = CapturingDataFor(pointerId);
    return capturingData && capturingData->PreventsCompatibilityMouseEvents;
  }

  void PointerCaptureController::ClearUnmatchedMouseDown(PointerID pointerId) noexcept
  {
    if (auto capturingData = CapturingDataFor(pointerId))
    {
      capturingData->PointerIsPressed = false;
    }
  }

  RefPtr<dom::PointerEvent> PointerCaptureController::PointerEventForMouseEvent(
    const dom::MouseEvent &mouseEvent, PointerID pointerId, const dom::DOMString &pointerType) noexcept
  {
    // If we already have known touches then we cannot dispatch a mouse event,
    // for instance in the case of a long press to initiate a system drag.
    for (auto &[id, capturingData] : _activePointerIdsToCapturingData)
    {
      if (capturingData->PointerType == dom::TouchPointerEventType()
          && capturingData->CurrentState == CapturingData::State::Ready)
      {
        return nullptr;
      }
    }

    auto type = mouseEvent.Type();

    auto capturingData = CapturingDataFor(pointerId);
    bool pointerIsPressed = capturingData ? capturingData->PointerIsPressed : false;

    MouseButton newButton = mouseEvent.ButtonType();
    MouseButton previousMouseButton =
      capturingData ? capturingData->PreviousMouseButton : MouseButton::PointerHasNotChanged;
    MouseButton button = [&]
    {
      auto pointerEventType = dom::PointerEvent::TypeFromMouseEventType(type);
      if (!dom::PointerEvent::TypeRequiresResolvedButton(pointerEventType))
      {
        if (newButton == previousMouseButton || !pointerIsPressed)
        {
          return MouseButton::PointerHasNotChanged;
        }
      }
      return newButton;
    }();

    // https://w3c.github.io/pointerevents/#chorded-button-interactions
    // Some pointer devices, such as mouse or pen, support multiple buttons. In the Mouse Event model, each
    // button press produces a mousedown and mouseup event. To better abstract this hardware difference and
    // simplify cross-device input authoring, Pointer Events do not fire overlapping pointerdown and pointerup
    // events for chorded button presses (depressing an additional button while another button on the pointer
    // device is already depressed).
    if (type == EventNames::MouseDown || type == EventNames::MouseUp)
    {
      // We're already active and getting another mousedown, this means that we should dispatch
      // a pointermove event and let the button state show the newly depressed button.
      if (type == EventNames::MouseDown && pointerIsPressed)
      {
        return dom::PointerEvent::Create(dom::DOMStringAtom(EventNames::PointerMove), button, mouseEvent,
                                         pointerId, pointerType);
      }

      // We're active and the mouseup still has some pressed button, this means we should dispatch
      // a pointermove event.
      if (type == EventNames::MouseUp && pointerIsPressed && mouseEvent.Buttons() > 0)
      {
        return dom::PointerEvent::Create(dom::DOMStringAtom(EventNames::PointerMove), button, mouseEvent,
                                         pointerId, pointerType);
      }
    }

    auto pointerEvent = dom::PointerEvent::Create(button, mouseEvent, pointerId, pointerType);
    if (capturingData)
    {
      capturingData->PreviousMouseButton = newButton;
    }
    else if (pointerEvent)
    {
      EnsureCapturingDataForPointerEvent(*pointerEvent)->PreviousMouseButton = newButton;
    }

    return pointerEvent;
  }

  void PointerCaptureController::DispatchOverOrOutEvent(dom::DOMStringAtom type, dom::EventTarget *target,
                                                        const PlatformTouchEvent &event, uint32 index,
                                                        bool isPrimary, LocalFrameView *view,
                                                        const DoublePoint &touchDelta) noexcept
  {
    DispatchEvent(*dom::PointerEvent::Create(type, event, {}, {}, index, isPrimary, view, touchDelta),
                  target);
  }

  void PointerCaptureController::DispatchEnterOrLeaveEvent(dom::DOMStringAtom type,
                                                           dom::Element &targetElement,
                                                           const PlatformTouchEvent &event, uint32 index,
                                                           bool isPrimary, LocalFrameView *view,
                                                           const DoublePoint &touchDelta) noexcept
  {
    // FIXME: Only dispatch to the elements with a listener for the event, or all of them when an ancestor has
    // a capturing one (they don't bubble), once targets can say which listeners they have.
    List<Ref<dom::Element>> targetChain;
    for (dom::Element *element = &targetElement; element; element = element->ParentElementInComposedTree())
    {
      targetChain.push_back(ShareRef(*element));
    }

    if (type == EventNames::PointerEnter)
    {
      for (auto it = targetChain.rbegin(); it != targetChain.rend(); ++it)
      {
        DispatchEvent(*dom::PointerEvent::Create(type, event, {}, {}, index, isPrimary, view, touchDelta),
                      it->get());
      }
    }
    else
    {
      for (auto &element : targetChain)
      {
        DispatchEvent(*dom::PointerEvent::Create(type, event, {}, {}, index, isPrimary, view, touchDelta),
                      element.get());
      }
    }
  }

  void PointerCaptureController::DispatchEventForTouchAtIndex(dom::EventTarget &target,
                                                              const PlatformTouchEvent &platformTouchEvent,
                                                              uint32 index, bool isPrimary,
                                                              LocalFrameView *view,
                                                              const DoublePoint &touchDelta) noexcept
  {
    RefPtr<Page> page = ShareRefPtr(_page.get());
    if (!page)
    {
      return;
    }

    auto *targetNode = DynamicDowncast<dom::Node>(&target);
    auto *targetElement = targetNode ? DynamicDowncast<dom::Element>(targetNode) : nullptr;
    krys_assert(targetElement);

    auto mapToPointerEvents = [&](const List<PlatformTouchEvent> &events) -> List<Ref<dom::PointerEvent>>
    {
      if (index)
      {
        return {dom::PointerEvent::Create(platformTouchEvent, {}, {}, false, false, index, isPrimary, view,
                                          touchDelta)};
      }

      List<Ref<dom::PointerEvent>> result;
      for (auto &event : events)
      {
        result.push_back(
          dom::PointerEvent::Create(event, {}, {}, false, false, index, isPrimary, view, touchDelta));
      }
      return result;
    };

    auto coalescedEvents = mapToPointerEvents(platformTouchEvent.CoalescedEvents());
    auto predictedEvents = mapToPointerEvents(platformTouchEvent.PredictedEvents());

    auto pointerEvent = dom::PointerEvent::Create(platformTouchEvent, coalescedEvents, predictedEvents, index,
                                                  isPrimary, view, touchDelta);

    // A pointer cancelled because its touch now pans or zooms the page fires nothing more.
    // https://w3c.github.io/pointerevents/#the-pointercancel-event
    if (pointerEvent->Type() != EventNames::PointerDown
        && HasCancelledPointerEventForIdentifier(pointerEvent->PointerId()))
    {
      return;
    }

    Ref<CapturingData> capturingData = EnsureCapturingDataForPointerEvent(*pointerEvent);

    // Check if the target changed, which would require dispatching boundary events.
    RefPtr<dom::Element> previousTarget = capturingData->PreviousTarget;
    RefPtr<dom::Element> currentTarget = ShareRefPtr(targetElement);

    capturingData->PreviousTarget = currentTarget;

    if (pointerEvent->Type() == EventNames::PointerMove && previousTarget != currentTarget)
    {
      // FIXME: Only dispatch pointerenter and pointerleave to the elements with a listener for them, or all
      // of them when an ancestor has a capturing one (they don't bubble), once targets can say which
      // listeners they have.
      List<Ref<dom::Element>> leftElementsChain;
      for (dom::Element *element = previousTarget.get(); element;
           element = element->ParentElementInComposedTree())
      {
        leftElementsChain.push_back(ShareRef(*element));
      }
      List<Ref<dom::Element>> enteredElementsChain;
      for (dom::Element *element = currentTarget.get(); element;
           element = element->ParentElementInComposedTree())
      {
        enteredElementsChain.push_back(ShareRef(*element));
      }

      if (!leftElementsChain.empty() && !enteredElementsChain.empty()
          && leftElementsChain.back().get() == enteredElementsChain.back().get())
      {
        size_t minHeight = std::min(leftElementsChain.size(), enteredElementsChain.size());
        size_t i;
        for (i = 0; i < minHeight; ++i)
        {
          if (leftElementsChain[leftElementsChain.size() - i - 1].get()
              != enteredElementsChain[enteredElementsChain.size() - i - 1].get())
          {
            break;
          }
        }
        leftElementsChain.erase(leftElementsChain.end() - static_cast<ptrdiff_t>(i), leftElementsChain.end());
        enteredElementsChain.erase(enteredElementsChain.end() - static_cast<ptrdiff_t>(i),
                                   enteredElementsChain.end());
      }

      if (previousTarget)
      {
        DispatchOverOrOutEvent(dom::DOMStringAtom(EventNames::PointerOut), previousTarget.get(),
                               platformTouchEvent, index, isPrimary, view, touchDelta);
      }

      for (auto &chain : leftElementsChain)
      {
        DispatchEvent(*dom::PointerEvent::Create(dom::DOMStringAtom(EventNames::PointerLeave),
                                                 platformTouchEvent, coalescedEvents, predictedEvents, index,
                                                 isPrimary, view, touchDelta),
                      chain.get());
      }

      if (currentTarget)
      {
        DispatchOverOrOutEvent(dom::DOMStringAtom(EventNames::PointerOver), currentTarget.get(),
                               platformTouchEvent, index, isPrimary, view, touchDelta);
      }

      for (auto it = enteredElementsChain.rbegin(); it != enteredElementsChain.rend(); ++it)
      {
        DispatchEvent(*dom::PointerEvent::Create(dom::DOMStringAtom(EventNames::PointerEnter),
                                                 platformTouchEvent, coalescedEvents, predictedEvents, index,
                                                 isPrimary, view, touchDelta),
                      it->get());
      }
    }

    if (pointerEvent->Type() == EventNames::PointerDown)
    {
      // https://w3c.github.io/pointerevents/#the-pointerdown-event
      // For input devices that do not support hover, a user agent MUST also fire a pointer event named
      // pointerover followed by a pointer event named pointerenter prior to dispatching the pointerdown
      // event.
      DispatchOverOrOutEvent(dom::DOMStringAtom(EventNames::PointerOver), currentTarget.get(),
                             platformTouchEvent, index, isPrimary, view, touchDelta);
      DispatchEnterOrLeaveEvent(dom::DOMStringAtom(EventNames::PointerEnter), *currentTarget,
                                platformTouchEvent, index, isPrimary, view, touchDelta);
    }

    if (pointerEvent->Type() == EventNames::PointerCancel)
    {
      auto location = platformTouchEvent.TouchPoints()[index].Pos();
      IntPoint documentPoint =
        view ? view->WindowToContents(FlooredIntPoint(location)) : FlooredIntPoint(location);
      CancelPointer(pointerEvent->PointerId(), documentPoint, pointerEvent.get());
      return;
    }

    DispatchEvent(*pointerEvent, &target);

    if (pointerEvent->Type() != EventNames::PointerUp)
    {
      return;
    }

    // https://w3c.github.io/pointerevents/#the-pointerup-event
    // For input devices that do not support hover, a user agent MUST also fire a pointer event named
    // pointerout followed by a pointer event named pointerleave after dispatching the pointerup event.
    DispatchOverOrOutEvent(dom::DOMStringAtom(EventNames::PointerOut), currentTarget.get(),
                           platformTouchEvent, index, isPrimary, view, touchDelta);
    DispatchEnterOrLeaveEvent(dom::DOMStringAtom(EventNames::PointerLeave), *currentTarget,
                              platformTouchEvent, index, isPrimary, view, touchDelta);

    capturingData->CurrentState = CapturingData::State::Finished;
    capturingData->PreviousTarget = nullptr;
  }

  void PointerCaptureController::DispatchEvent(dom::PointerEvent &event, dom::EventTarget *target) noexcept
  {
    if (!target || event.Target())
    {
      return;
    }

    // https://w3c.github.io/pointerevents/#firing-events-using-the-pointerevent-interface
    // If the event is not gotpointercapture or lostpointercapture, run Process Pending Pointer Capture steps
    // for this PointerEvent. We only need to do this for non-mouse type since for mouse events this method
    // will be called in Document::PrepareMouseEvent().
    if (event.PointerType() != dom::MousePointerEventType())
    {
      ProcessPendingPointerCapture(event.PointerId());
    }

    PointerEventWillBeDispatched(event, target);
    (void)dom::EventDispatcher::DispatchToTarget(event, target);
    PointerEventWasDispatched(event);
  }

  void PointerCaptureController::PointerEventWillBeDispatched(const dom::PointerEvent &event,
                                                              dom::EventTarget *target) noexcept
  {
    auto *node = DynamicDowncast<dom::Node>(target);
    auto *element = node ? DynamicDowncast<dom::Element>(node) : nullptr;
    if (!element)
    {
      return;
    }

    bool isPointerdown = event.Type() == EventNames::PointerDown;
    bool isPointerup = event.Type() == EventNames::PointerUp;
    bool isPointermove = event.Type() == EventNames::PointerMove;

    if (!isPointerdown && !isPointerup && !isPointermove)
    {
      return;
    }

    // Let targetDocument be target's node document.
    // If the event is pointerdown, pointermove, or pointerup set active document for the event's pointerId to
    // targetDocument.
    auto capturingData = EnsureCapturingDataForPointerEvent(event);
    capturingData->ActiveDocument = WeakPtr<dom::Document>::WithRef(element->NodeDocument());

    if (isPointermove)
    {
      return;
    }

    capturingData->PointerIsPressed = isPointerdown;

    if (event.PointerType() == dom::TouchPointerEventType() && isPointerdown)
    {
      // https://w3c.github.io/pointerevents/#implicit-pointer-capture

      // Some input devices (such as touchscreens) implement a "direct manipulation" metaphor where a pointer
      // is intended to act primarily on the UI element it became active upon (providing a physical illusion
      // of direct contact, instead of indirect contact via a cursor that conceptually floats above the UI).
      // Such devices are identified by the InputDeviceCapabilities.pointerMovementScrolls property and should
      // have "implicit pointer capture" behavior as follows.

      // Direct manipulation devices should behave exactly as if setPointerCapture was called on the target
      // element just before the invocation of any pointerdown listeners. The hasPointerCapture API may be
      // used (eg. within any pointerdown listener) to determine whether this has occurred. If
      // releasePointerCapture is not called for the pointer before the next pointer event is fired, then a
      // gotpointercapture event will be dispatched to the target (as normal) indicating that capture is
      // active.
      (void)SetPointerCapture(element, event.PointerId());
    }
    element->NodeDocument().HandleLightDismiss(event, *element);
  }

  auto PointerCaptureController::EnsureCapturingDataForPointerEvent(const dom::PointerEvent &event) noexcept
    -> Ref<CapturingData>
  {
    auto it = _activePointerIdsToCapturingData.find(event.PointerId());
    if (it == _activePointerIdsToCapturingData.end())
    {
      it = _activePointerIdsToCapturingData
             .emplace(event.PointerId(), CapturingData::Create(event.PointerType()))
             .first;
    }
    return it->second;
  }

  void PointerCaptureController::PointerEventWasDispatched(const dom::PointerEvent &event) noexcept
  {
    auto capturingData = CapturingDataFor(event.PointerId());
    if (!capturingData)
    {
      return;
    }

    capturingData->IsPrimary = event.GetIsPrimary();

    // Immediately after firing the pointerup or pointercancel events, a user agent MUST clear the pending
    // pointer capture target override for the pointerId of the pointerup or pointercancel event that was just
    // dispatched, and then run Process Pending Pointer Capture steps to fire lostpointercapture if necessary.
    // https://w3c.github.io/pointerevents/#implicit-release-of-pointer-capture
    if (event.Type() == EventNames::PointerUp)
    {
      capturingData->PendingTargetOverride = nullptr;
      ProcessPendingPointerCapture(event.PointerId());
    }

    // If a mouse pointer has moved while it isn't pressed, make sure we reset the
    // preventsCompatibilityMouseEvents flag since we could otherwise prevent compatibility mouse events while
    // those are only supposed to be prevented while the pointer is pressed.
    if (event.Type() == EventNames::PointerMove && capturingData->PointerType == dom::MousePointerEventType()
        && !capturingData->PointerIsPressed)
    {
      capturingData->PreventsCompatibilityMouseEvents = false;
    }

    // If the pointer event dispatched was pointerdown and the event was canceled, then set the PREVENT MOUSE
    // EVENT flag for this pointerType.
    // https://www.w3.org/TR/pointerevents/#mapping-for-devices-that-support-hover
    if (event.Type() == EventNames::PointerDown)
    {
      capturingData->PreventsCompatibilityMouseEvents = event.DefaultPrevented();
    }
  }

  void PointerCaptureController::CancelPointer(PointerID pointerId, const IntPoint &documentPoint,
                                               dom::PointerEvent *existingCancelEvent) noexcept
  {
    // https://w3c.github.io/pointerevents/#the-pointercancel-event

    // A user agent MUST fire a pointer event named pointercancel in the following circumstances:
    //
    // The user agent has determined that a pointer is unlikely to continue to produce events (for example,
    // because of a hardware event). After having fired the pointerdown event, if the pointer is subsequently
    // used to manipulate the page viewport (e.g. panning or zooming). Immediately before drag operation
    // starts [HTML], for the pointer that caused the drag operation. After firing the pointercancel event, a
    // user agent MUST also fire a pointer event named pointerout followed by firing a pointer event named
    // pointerleave.

    // https://w3c.github.io/pointerevents/#implicit-release-of-pointer-capture

    // Immediately after firing the pointerup or pointercancel events, a user agent MUST clear the pending
    // pointer capture target override for the pointerId of the pointerup or pointercancel event that was just
    // dispatched, and then run Process Pending Pointer Capture steps to fire lostpointercapture if necessary.
    // After running Process Pending Pointer Capture steps, if the pointer supports hover, user agent MUST
    // also send corresponding boundary events necessary to reflect the current position of the pointer with
    // no capture.

    auto capturingData = CapturingDataFor(pointerId);
    if (!capturingData)
    {
      return;
    }

    if (capturingData->CurrentState == CapturingData::State::Cancelled)
    {
      return;
    }

    RefPtr<Page> page = ShareRefPtr(_page.get());
    if (!page)
    {
      return;
    }

    capturingData->PendingTargetOverride = nullptr;
    capturingData->CurrentState = CapturingData::State::Cancelled;
    capturingData->PreviousTarget = nullptr;

    auto target = [&]() -> RefPtr<dom::Element>
    {
      if (capturingData->TargetOverride)
      {
        return capturingData->TargetOverride;
      }
      constexpr EnumSet<HitTestRequest::Type> hitType {
        HitTestRequest::Type::ReadOnly, HitTestRequest::Type::Active,
        HitTestRequest::Type::DisallowUserAgentShadowContent, HitTestRequest::Type::AllowChildFrameContent};
      return ShareRefPtr(page->MainFrame()
                           .GetEventHandler()
                           .HitTestResultAtPoint(LayoutPoint(documentPoint), hitType)
                           .InnerNonSharedElement());
    }();

    if (!target)
    {
      return;
    }

    // After firing the pointercancel event, a user agent MUST also fire a pointer event named pointerout
    // followed by firing a pointer event named pointerleave.
    auto isPrimary =
      capturingData->IsPrimary ? dom::PointerEvent::IsPrimary::Yes : dom::PointerEvent::IsPrimary::No;

    if (existingCancelEvent)
    {
      (void)dom::EventDispatcher::DispatchToTarget(*existingCancelEvent, target.get());
    }
    else
    {
      auto cancelEvent = dom::PointerEvent::Create(dom::DOMStringAtom(EventNames::PointerCancel), pointerId,
                                                   capturingData->PointerType, isPrimary);
      (void)dom::EventDispatcher::DispatchToTarget(*cancelEvent, target.get());
    }

    (void)dom::EventDispatcher::DispatchToTarget(
      *dom::PointerEvent::Create(dom::DOMStringAtom(EventNames::PointerOut), pointerId,
                                 capturingData->PointerType, isPrimary),
      target.get());
    (void)dom::EventDispatcher::DispatchToTarget(
      *dom::PointerEvent::Create(dom::DOMStringAtom(EventNames::PointerLeave), pointerId,
                                 capturingData->PointerType, isPrimary),
      target.get());
    ProcessPendingPointerCapture(pointerId);
  }

  void PointerCaptureController::ProcessPendingPointerCapture(PointerID pointerId) noexcept
  {
    auto capturingData = CapturingDataFor(pointerId);
    if (!capturingData)
    {
      return;
    }

    if (_processingPendingPointerCapture)
    {
      return;
    }

    _processingPendingPointerCapture = true;

    // Cache the pending target override since it could be modified during the dispatch of events in this
    // function.
    auto pendingTargetOverride = capturingData->PendingTargetOverride;

    // https://w3c.github.io/pointerevents/#process-pending-pointer-capture
    // 1. If the pointer capture target override for this pointer is set and is not equal to the pending
    // pointer capture target override, then fire a pointer event named lostpointercapture at the pointer
    // capture target override node.
    if (auto targetOverride = capturingData->TargetOverride;
        targetOverride && targetOverride != pendingTargetOverride)
    {
      if (targetOverride->IsConnected())
      {
        (void)dom::EventDispatcher::DispatchToTarget(
          *dom::PointerEvent::CreateForPointerCapture(dom::DOMStringAtom(EventNames::LostPointerCapture),
                                                      pointerId, capturingData->IsPrimary,
                                                      capturingData->PointerType),
          targetOverride.get());
      }
      if (capturingData->PointerType == dom::MousePointerEventType())
      {
        if (auto *frame = targetOverride->NodeDocument().Frame())
        {
          frame->GetEventHandler().PointerCaptureElementDidChange(nullptr);
        }
      }
    }

    // 2. If the pending pointer capture target override for this pointer is set and is not equal to the
    // pointer capture target override, then fire a pointer event named gotpointercapture at the pending
    // pointer capture target override.
    if (capturingData->PendingTargetOverride && capturingData->TargetOverride != pendingTargetOverride)
    {
      if (capturingData->PointerType == dom::MousePointerEventType())
      {
        if (auto *frame = pendingTargetOverride->NodeDocument().Frame())
        {
          frame->GetEventHandler().PointerCaptureElementDidChange(pendingTargetOverride.get());
        }
      }
      (void)dom::EventDispatcher::DispatchToTarget(
        *dom::PointerEvent::CreateForPointerCapture(dom::DOMStringAtom(EventNames::GotPointerCapture),
                                                    pointerId, capturingData->IsPrimary,
                                                    capturingData->PointerType),
        pendingTargetOverride.get());
    }

    // 3. Set the pointer capture target override to the pending pointer capture target override, if set.
    // Otherwise, clear the pointer capture target override.
    capturingData->TargetOverride = pendingTargetOverride;

    _processingPendingPointerCapture = false;
  }
}
