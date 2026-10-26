#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/DataTransfer.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DragEvent.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/PointerEvent.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/DOM/TextEvent.hpp"
#include "Krystal.Booey/DOM/TouchEvent.hpp"
#include "Krystal.Booey/DOM/WheelEvent.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/EditingBehavior.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/Editing/VisibleSelection.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcherManager.hpp"
#include "Krystal.Booey/HTML/HTMLAnchorElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Page/AutoscrollController.hpp"
#include "Krystal.Booey/Page/DragController.hpp"
#include "Krystal.Booey/Page/DragState.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/MouseEventWithHitTestResults.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/PointerCaptureController.hpp"
#include "Krystal.Booey/Page/PointerLockController.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/Pasteboard.hpp"
#include "Krystal.Booey/Platform/Scrollbar.hpp"
#include "Krystal.Booey/Platform/StaticPasteboard.hpp"
#include "Krystal.Booey/Rendering/HitTestRequest.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/HitTestSource.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/RenderListBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Booey/Style/Values/UI/Cursor.hpp"
#include <algorithm>

namespace krys::boo
{
  namespace
  {
    // Match key code of composition keydown event on windows.
    // IE sends VK_PROCESSKEY which has value 229;
    constexpr int32 CompositionEventKeyCode = 229;

    /// @brief The element key events go to in @p document: its focused element, or its body.
    // FIXME: A fullscreen video and a plugin document's plugin, once there are any.
    KRYS_NODISCARD dom::Element *EventTargetElementForDocument(dom::Document *document) noexcept
    {
      if (!document)
      {
        return nullptr;
      }
      dom::Element *element = document->FocusedElement();
      if (!element && Is<dom::HTMLDocument>(*document))
      {
        element = document->BodyOrFrameset();
      }
      return element;
    }

    KRYS_NODISCARD DoublePoint DocumentPointForWindowPoint(LocalFrame &frame,
                                                           const DoublePoint &windowPoint) noexcept
    {
      auto *view = frame.View();
      if (!view)
      {
        // FIXME: Is it really OK to use the wrong coordinates here when view is 0?
        // Historically the code would just crash; this is clearly no worse than that.
        return windowPoint;
      }
      return view->WindowToContents(windowPoint);
    }

    /// @brief The nearest inclusive ancestor of @p a and @p b in the composed tree, or null if they're in
    /// different trees.
    KRYS_NODISCARD dom::Node *CommonInclusiveAncestorInComposedTree(dom::Node &a, dom::Node &b) noexcept
    {
      for (dom::Node *ancestorOfA = &a; ancestorOfA; ancestorOfA = ancestorOfA->ParentInComposedTree())
      {
        for (dom::Node *ancestorOfB = &b; ancestorOfB; ancestorOfB = ancestorOfB->ParentInComposedTree())
        {
          if (ancestorOfA == ancestorOfB)
          {
            return ancestorOfA;
          }
        }
      }
      return nullptr;
    }

    KRYS_NODISCARD RefPtr<dom::Node> TargetNodeForClickEvent(dom::Node *mousePressNode,
                                                             dom::Node *mouseReleaseNode) noexcept
    {
      if (!mousePressNode || !mouseReleaseNode)
      {
        return nullptr;
      }

      if (mousePressNode == mouseReleaseNode)
      {
        return ShareRefPtr(mouseReleaseNode);
      }

      // If mousePressNode and mouseReleaseNode differ, we should fire the event at their common ancestor if
      // there is one.
      if (&mousePressNode->NodeDocument() == &mouseReleaseNode->NodeDocument())
      {
        if (auto *commonAncestor = CommonInclusiveAncestorInComposedTree(*mousePressNode, *mouseReleaseNode))
        {
          return ShareRefPtr(commonAncestor);
        }
      }

      auto *mouseReleaseShadowHost = dom::TreeQueries::ShadowHost(*mouseReleaseNode);
      if (mouseReleaseShadowHost && mouseReleaseShadowHost == dom::TreeQueries::ShadowHost(*mousePressNode))
      {
        // We want to dispatch the click to the shadow tree host element to give listeners the illusion that
        // the shadow tree is a single element. For example, we want to give the illusion that <input
        // type="range"> is a single element even though it is a composition of multiple shadom tree elements.
        return ShareRefPtr(mouseReleaseShadowHost);
      }

      return nullptr;
    }

    KRYS_NODISCARD bool ShouldRefetchEventTarget(const MouseEventWithHitTestResults &mouseEvent) noexcept
    {
      auto *targetNode = mouseEvent.TargetNode();
      krys_debug_assert(targetNode);
      if (!targetNode->ParentNode())
      {
        return true;
      }
      auto *shadowRoot = DynamicDowncast<dom::ShadowRoot>(*targetNode);
      return shadowRoot && shadowRoot->Host() && Is<html::HTMLInputElement>(*shadowRoot->Host());
    }

    KRYS_NODISCARD dom::FocusDirection FocusDirectionForKey(const dom::DOMString &keyIdentifier) noexcept
    {
      if (keyIdentifier == u8"Down")
      {
        return dom::FocusDirection::Down;
      }
      if (keyIdentifier == u8"Up")
      {
        return dom::FocusDirection::Up;
      }
      if (keyIdentifier == u8"Left")
      {
        return dom::FocusDirection::Left;
      }
      if (keyIdentifier == u8"Right")
      {
        return dom::FocusDirection::Right;
      }
      return dom::FocusDirection::None;
    }

    KRYS_NODISCARD Scrollbar *ScrollbarForMouseEvent(const MouseEventWithHitTestResults &mouseEvent,
                                                     LocalFrameView *view) noexcept
    {
      if (view)
      {
        if (auto *scrollbar = view->ScrollbarAtPoint(FlooredIntPoint(mouseEvent.Event().Position())))
        {
          return scrollbar;
        }
      }
      return mouseEvent.GetScrollbar();
    }

    KRYS_NODISCARD RefPtr<dom::Element> GetElementOrAncestorElementForNode(dom::Node *targetNode) noexcept
    {
      while (targetNode)
      {
        if (auto *asElement = DynamicDowncast<dom::Element>(targetNode))
        {
          return ShareRefPtr(asElement);
        }
        targetNode = targetNode->ParentInComposedTree();
      }
      return nullptr;
    }
  }

  EventHandler::EventHandler(LocalFrame &frame) noexcept
      : _frame(frame), _autoscrollController(CreateUnique<AutoscrollController>()),
        _fakeMouseMoveEventTimer(*this, &EventHandler::FakeMouseMoveEventTimerFired)
  {
  }

  EventHandler::~EventHandler() noexcept = default;

  void EventHandler::ResizeLayerDestroyed() noexcept
  {
    krys_debug_assert(_resizeLayer);
    _resizeLayer = nullptr;
  }

  void EventHandler::SetCapturingMouseEventsElement(RefPtr<dom::Element> &&element) noexcept
  {
    _capturingMouseEventsElement = krys::move(element);
  }

  void EventHandler::PointerCaptureElementDidChange(dom::Element *element) noexcept
  {
    if (_capturingMouseEventsElement.get() == element)
    {
      return;
    }

    SetCapturingMouseEventsElement(ShareRefPtr(element));

    // Click events should be aware of pointer capture, too.
    if (element && _clickNode)
    {
      _clickCaptureElement = ShareRefPtr(element);
    }

    // Since `UpdateMouseEventTargetNode` will dispatch boundary events, call it only when
    // pointer capture is received. When capture is released, the PointerCaptureController
    // already handles the appropriate boundary events (pointerout and pointerleave from
    // the captured element).
    if (element)
    {
      UpdateMouseEventTargetNode(dom::DOMStringAtom(EventNames::GotPointerCapture), element,
                                 _lastPlatformMouseEvent, FireMouseOverOut::Yes);
    }
  }

  HitTestResult EventHandler::HitTestResultAtPoint(const LayoutPoint &point,
                                                   EnumSet<HitTestRequest::Type> hitType) const noexcept
  {
    Ref frame = ShareRef(_frame);

    // FIXME: Hit testing in the root frame (the frames above can obscure this one), once there are subframes.

    auto result = HitTestResult {point};
    RefPtr<dom::Document> document = ShareRefPtr(frame->Document());
    if (!document)
    {
      return result;
    }

    // We should always start hit testing a clean tree.
    // FIXME: The subframes' too (updateLayoutAndStyleIfNeededRecursive), once there are subframes.
    (void)document->UpdateLayout();

    HitTestRequest request(hitType);
    document->HitTest(request, result);
    if (!request.ReadOnly())
    {
      RefPtr<dom::Element> targetElement = ShareRefPtr(result.TargetElement());
      document->UpdateHoverActiveState(request, targetElement.get());
    }

    // FIXME: Image overlays (disallowsUserAgentShadowContentExceptForImageOverlays), once there are any.
    if (request.DisallowsUserAgentShadowContent())
    {
      result.SetToNonUserAgentShadowAncestor();
    }

    return result;
  }

  void EventHandler::Clear() noexcept
  {
    _resizeLayer = nullptr;
    _capturingMouseEventsElement = nullptr;
    ClearElementUnderMouse();
    _lastElementUnderMouse = nullptr;
    _ancestorsOfLastElementUnderMouse.clear();
    _mouseMoveTargetOverride = nullptr;
    _lastScrollbarUnderMouse = nullptr;
    _clickCount = 0;
    _clickNode = nullptr;
    _clickCaptureElement = nullptr;
    _lastKnownMousePosition = null;
    _lastKnownMouseGlobalPosition = null;
    _mousePressNode = nullptr;
    _mousePressed = false;
    _mouseDownMayStartSelect = false;
    _mouseDownWasSingleClickInSelection = false;
    _selectionInitiationState = HaveNotStartedSelection;
    _mouseDownMayStartAutoscroll = false;
    _dragStartSelection = null;
    _dragTarget = nullptr;
    _shouldOnlyFireDragOverEvent = false;
    _capturesDragging = false;
    _maxMouseMovedDuration = {};
    _originatingTouchPointTargets.clear();
    _originatingTouchPointDocument = nullptr;
    _originatingTouchPointTargetKey = 0;
    _touchPressed = false;
    _didStartDrag = false;
    CancelFakeMouseMoveEvent();
  }

  void EventHandler::NodeWillBeRemoved(dom::Node &nodeToBeRemoved) noexcept
  {
    auto contains = [&](dom::Node *node)
    {
      return node && dom::TreeQueries::IsShadowIncludingInclusiveAncestor(nodeToBeRemoved, *node);
    };

    if (contains(_clickNode.get()))
    {
      _clickNode = nullptr;
    }

    if (contains(_elementUnderMouse.get()))
    {
      if (auto *elementBeingRemoved = DynamicDowncast<dom::Element>(nodeToBeRemoved))
      {
        _mouseMoveTargetOverride = ShareRefPtr(elementBeingRemoved->ParentElementInComposedTree());
      }
    }

    if (contains(_lastElementUnderMouse.get()))
    {
      _lastElementUnderMouse = nullptr;
    }

    if (contains(_clickCaptureElement.get()))
    {
      _clickCaptureElement = nullptr;
    }

    // Ancestors are held strongly, so the removed ones are dropped here.
    std::erase_if(_ancestorsOfLastElementUnderMouse,
                  [&](const RefPtr<dom::Element> &element) { return contains(element.get()); });
  }

  void EventHandler::SetLastKnownMousePosition(DoublePoint position, DoublePoint globalPosition) noexcept
  {
    _lastKnownMousePosition = position;
    _lastKnownMouseGlobalPosition = globalPosition;
  }

  bool EventHandler::HandleMousePressEvent(const PlatformMouseEvent &platformMouseEvent) noexcept
  {
    Ref frame = ShareRef(_frame);
    RefPtr<LocalFrameView> protectedView = ShareRefPtr(frame->View());

    // FIXME: Page overlays and synthetic touch events.

    // A press is an activation-triggering input event: the window is notified before the event is dispatched.
    // https://html.spec.whatwg.org/#activation-triggering-input-event
    if (auto *window = frame->Window())
    {
      window->NotifyActivated();
    }

    if (auto *page = frame->GetPage())
    {
      if (auto &pointerLockController = page->GetPointerLockController(); pointerLockController.IsLocked())
      {
        pointerLockController.DispatchLockedMouseEvent(platformMouseEvent,
                                                       dom::DOMStringAtom(EventNames::MouseDown));
        return true;
      }
    }

    _mousePressed = true;
    _capturesDragging = true;
    SetLastKnownMousePosition(platformMouseEvent.Position(), platformMouseEvent.GlobalPosition());
    _mouseDownTimestamp = platformMouseEvent.Timestamp();
    _mouseDownMayStartDrag = false;
    _mouseDownMayStartSelect = false;
    _mouseDownMayStartAutoscroll = false;
    if (auto *view = frame->View())
    {
      _mouseDownContentsPosition =
        LayoutPoint(view->WindowToContents(FlooredIntPoint(platformMouseEvent.Position())));
    }
    else
    {
      InvalidateClick();
      return false;
    }

    // Save the document point we generate in case the window coordinate is invalidated by what happens
    // when we dispatch the event.
    DoublePoint documentPoint = DocumentPointForWindowPoint(*frame, platformMouseEvent.Position());
    auto *document = frame->Document();
    if (!document)
    {
      InvalidateClick();
      return false;
    }
    MouseEventWithHitTestResults mouseEvent = document->PrepareMouseEvent(
      HitTestRequest(EnumSet<HitTestRequest::Type> {HitTestRequest::Type::Active,
                                                    HitTestRequest::Type::DisallowUserAgentShadowContent}),
      documentPoint, platformMouseEvent);

    if (!mouseEvent.TargetNode())
    {
      InvalidateClick();
      return false;
    }

    _mousePressNode = ShareRefPtr(mouseEvent.TargetNode());
    document->SetSequentialFocusNavigationStartingPoint(mouseEvent.TargetNode());

    RefPtr<Scrollbar> scrollbar = ShareRefPtr(ScrollbarForMouseEvent(mouseEvent, frame->View()));
    UpdateLastScrollbarUnderMouse(scrollbar.get(), SetOrClearLastScrollbar::Set);
    bool passedToScrollbar = scrollbar && PassMousePressEventToScrollbar(mouseEvent, scrollbar.get());

    // FIXME: Subframes and pan scrolling.

    _clickCount = platformMouseEvent.ClickCount();
    _clickNode = ShareRefPtr(mouseEvent.TargetNode());

    auto *layer = _clickNode && _clickNode->Renderer() ? _clickNode->Renderer()->EnclosingLayer() : nullptr;
    auto localPoint = RoundedIntPoint(mouseEvent.GetHitTestResult().LocalPoint());
    if (layer && layer->IsPointInResizeControl(localPoint))
    {
      layer->SetInResizeMode(true);
      _resizeLayer = WeakPtr<RenderLayer>::WithRef(*layer);
      _offsetFromResizeCorner = LayoutSize(layer->OffsetFromResizeCorner(localPoint));
      RefPtr<dom::Node> resizedNode = ShareRefPtr(mouseEvent.TargetNode());
      (void)DispatchMouseEvent(dom::DOMStringAtom(EventNames::MouseDown), resizedNode.get(), _clickCount,
                               platformMouseEvent, FireMouseOverOut::Yes);
      return true;
    }

    _frame.Document()->Selection().SetCaretBlinkingSuspended(true);

    RefPtr<dom::Node> targetNode = ShareRefPtr(mouseEvent.TargetNode());
    bool swallowEvent = !DispatchMouseEvent(dom::DOMStringAtom(EventNames::MouseDown), targetNode.get(),
                                            _clickCount, platformMouseEvent, FireMouseOverOut::Yes);
    if (!swallowEvent || mouseEvent.GetScrollbar())
    {
      _capturesDragging = true;
    }
    else
    {
      _capturesDragging = _capturesDragging ? CapturesDragging::InabilityReason::Unknown
                                            : _capturesDragging.GetInabilityReason();
    }

    // If the hit testing originally determined the event was in a scrollbar, refetch the
    // MouseEventWithHitTestResults in case the scrollbar widget was destroyed when the mouse event was
    // handled.
    if (mouseEvent.GetScrollbar())
    {
      const bool wasLastScrollBar = mouseEvent.GetScrollbar() == _lastScrollbarUnderMouse.get();
      mouseEvent = document->PrepareMouseEvent(HitTestRequest(), documentPoint, platformMouseEvent);
      if (wasLastScrollBar && mouseEvent.GetScrollbar() != _lastScrollbarUnderMouse.get())
      {
        _lastScrollbarUnderMouse = nullptr;
      }
    }

    if (!swallowEvent && ShouldRefetchEventTarget(mouseEvent))
    {
      mouseEvent = document->PrepareMouseEvent(HitTestRequest(), documentPoint, platformMouseEvent);
    }

    if (!swallowEvent)
    {
      if (passedToScrollbar)
      {
        swallowEvent = true;
      }
      else
      {
        swallowEvent = HandleMousePressEvent(mouseEvent);
      }
    }
    return swallowEvent;
  }

  bool EventHandler::MouseMoved(const PlatformMouseEvent &event) noexcept
  {
    Ref frame = ShareRef(_frame);
    RefPtr<LocalFrameView> protectedView = ShareRefPtr(frame->View());

    // The longest a move took to handle.
    auto start = MonotonicTime::Now();

    // FIXME: Page overlays, and telling the chrome the mouse moved over an element.
    HitTestResult hitTestResult;
    bool result = HandleMouseMoveEvent(event, &hitTestResult);

    _maxMouseMovedDuration = std::max(_maxMouseMovedDuration, MonotonicTime::Now() - start);
    return result;
  }

  bool EventHandler::HandleMouseMoveEvent(const PlatformMouseEvent &platformMouseEvent,
                                          HitTestResult *hitTestResult) noexcept
  {
    Ref frame = ShareRef(_frame);
    RefPtr<LocalFrameView> protectedView = ShareRefPtr(frame->View());

    // FIXME: Synthetic touch events, hover timers, fake mouse moves, SVG panning and frame set resizing.

    if (auto *page = frame->GetPage())
    {
      if (auto &pointerLockController = page->GetPointerLockController(); pointerLockController.IsLocked())
      {
        pointerLockController.DispatchLockedMouseEvent(platformMouseEvent,
                                                       dom::DOMStringAtom(EventNames::MouseMove));
        return true;
      }
    }

    SetLastKnownMousePosition(platformMouseEvent.Position(), platformMouseEvent.GlobalPosition());

    // Send events right to a scrollbar if the mouse is pressed.
    if (RefPtr<Scrollbar> lastScrollbarUnderMouse = _lastScrollbarUnderMouse;
        lastScrollbarUnderMouse && _mousePressed)
    {
      return lastScrollbarUnderMouse->MouseMoved(platformMouseEvent);
    }

    EnumSet<HitTestRequest::Type> hitType {HitTestRequest::Type::Move,
                                           HitTestRequest::Type::DisallowUserAgentShadowContent,
                                           HitTestRequest::Type::AllowFrameScrollbars};
    if (_mousePressed)
    {
      hitType.Add(HitTestRequest::Type::Active);
    }

    // Treat any mouse move events as readonly if the user is currently touching the screen.
    if (_touchPressed)
    {
      hitType.Add(HitTestRequest::Type::Active);
      hitType.Add(HitTestRequest::Type::ReadOnly);
    }

    MouseEventWithHitTestResults mouseEvent = PrepareMouseEvent(HitTestRequest(hitType), platformMouseEvent);
    if (hitTestResult)
    {
      *hitTestResult = mouseEvent.GetHitTestResult();
    }

    if (auto *resizeLayer = _resizeLayer.get(); resizeLayer && resizeLayer->InResizeMode())
    {
      resizeLayer->Resize(platformMouseEvent, _offsetFromResizeCorner);

      if (resizeLayer->Renderer().Style().ShouldPlaceVerticalScrollbarOnLeft())
      {
        auto absolutePoint = frame->View()->WindowToContents(FlooredIntPoint(platformMouseEvent.Position()));
        auto localPoint = RoundedIntPoint(resizeLayer->AbsoluteToContents(LayoutPoint(absolutePoint)));
        _offsetFromResizeCorner.SetWidth(LayoutUnit(resizeLayer->OffsetFromResizeCorner(localPoint).Width()));
      }
    }
    else
    {
      RefPtr<Scrollbar> scrollbar = ShareRefPtr(mouseEvent.GetScrollbar());
      UpdateLastScrollbarUnderMouse(scrollbar.get(), _mousePressed ? SetOrClearLastScrollbar::Clear
                                                                   : SetOrClearLastScrollbar::Set);

      // Handle hover effects on platforms that support visual feedback on scrollbar hovering.
      if (!_mousePressed && scrollbar)
      {
        (void)scrollbar->MouseMoved(platformMouseEvent);
      }
    }

    // FIXME: Subframes and remote frames.

    if (auto *view = frame->View())
    {
      UpdateCursor(*view, mouseEvent.GetHitTestResult(), platformMouseEvent.ShiftKey());
    }

    RefPtr<dom::Node> targetNode = ShareRefPtr(mouseEvent.TargetNode());
    bool swallowEvent = !DispatchMouseEvent(dom::DOMStringAtom(EventNames::MouseMove), targetNode.get(), 0,
                                            platformMouseEvent, FireMouseOverOut::Yes);

    if (!swallowEvent
        || (!_capturesDragging
            && _capturesDragging.GetInabilityReason()
                 == CapturesDragging::InabilityReason::MouseMoveIsCancelled))
    {
      swallowEvent = HandleMouseDraggedEvent(mouseEvent);
    }

    return swallowEvent;
  }

  void EventHandler::InvalidateClick() noexcept
  {
    _clickCount = 0;
    _clickNode = nullptr;
    _clickCaptureElement = nullptr;
  }

  bool EventHandler::SwallowAnyClickEvent(
    const PlatformMouseEvent &platformMouseEvent, const MouseEventWithHitTestResults &mouseEvent,
    IgnoreAncestorNodesForClickEvent ignoreAncestorNodesForClickEvent) noexcept
  {
    if (!_clickCount)
    {
      return false;
    }

    RefPtr<dom::Node> nodeToClick;
    if (ignoreAncestorNodesForClickEvent == IgnoreAncestorNodesForClickEvent::Yes)
    {
      if (mouseEvent.TargetNode() == _clickNode.get())
      {
        nodeToClick = _clickNode;
      }
    }
    else
    {
      RefPtr<dom::Node> clickNode = _clickNode;
      nodeToClick = TargetNodeForClickEvent(clickNode.get(), mouseEvent.TargetNode());
    }

    if (!nodeToClick && !_clickCaptureElement)
    {
      return false;
    }

    // The auxclick event should only be fired for the non-primary pointer buttons.
    // In the case of right button, the auxclick event is dispatched after any contextmenu event.
    //
    // The click event should only be fired for the primary pointer button.
    bool isPrimaryPointerButton = platformMouseEvent.Button() == MouseButton::Left;
    dom::DOMStringAtom eventName(isPrimaryPointerButton ? EventNames::Click : EventNames::AuxClick);

    bool swallowed = false;
    RefPtr<dom::Element> clickCaptureElement = std::exchange(_clickCaptureElement, nullptr);
    if (clickCaptureElement)
    {
      UpdateMouseEventTargetNode(eventName, nodeToClick.get(), platformMouseEvent, FireMouseOverOut::Yes);
      swallowed =
        !DispatchAnyClickEvent(eventName, clickCaptureElement.get(), _clickCount, platformMouseEvent);
    }
    else if (nodeToClick)
    {
      swallowed = !DispatchMouseEvent(eventName, nodeToClick.get(), _clickCount, platformMouseEvent,
                                      FireMouseOverOut::No);
    }

    // FIXME: Telling the chrome client about the click (didDispatchClickEvent), once it wants to know.
    return swallowed;
  }

  bool EventHandler::DispatchAnyClickEvent(dom::DOMStringAtom eventType, dom::Node *clickNode,
                                           int32 clickCount,
                                           const PlatformMouseEvent &platformMouseEvent) noexcept
  {
    if (clickNode)
    {
      if (RefPtr<dom::Element> node = ShareRefPtr(DynamicDowncast<dom::Element>(clickNode)))
      {
        return node->DispatchMouseEvent(platformMouseEvent, eventType, clickCount).Dispatched
               == dom::Element::EventIsDispatched::Yes;
      }
    }
    return false;
  }

  bool EventHandler::HandleMouseReleaseEvent(const PlatformMouseEvent &platformMouseEvent) noexcept
  {
    Ref frame = ShareRef(_frame);
    RefPtr<LocalFrameView> protectedView = ShareRefPtr(frame->View());

    frame->Document()->Selection().SetCaretBlinkingSuspended(false);

    if (auto *page = frame->GetPage())
    {
      if (auto &pointerLockController = page->GetPointerLockController(); pointerLockController.IsLocked())
      {
        pointerLockController.DispatchLockedMouseEvent(platformMouseEvent,
                                                       dom::DOMStringAtom(EventNames::MouseUp));
        return true;
      }
    }

    // FIXME: Page overlays, synthetic touch events, user gestures and pan scrolling.

    _mousePressed = false;
    SetLastKnownMousePosition(platformMouseEvent.Position(), platformMouseEvent.GlobalPosition());

    // FIXME: SVG panning, frame set resizing and immediate actions.

    if (RefPtr<Scrollbar> lastScrollbarUnderMouse = _lastScrollbarUnderMouse)
    {
      InvalidateClick();
      (void)lastScrollbarUnderMouse->MouseUp(platformMouseEvent);
      RefPtr<dom::Element> lastElementUnderMouse = _lastElementUnderMouse;
      return !DispatchMouseEvent(dom::DOMStringAtom(EventNames::MouseUp), lastElementUnderMouse.get(),
                                 _clickCount, platformMouseEvent, FireMouseOverOut::No);
    }

    MouseEventWithHitTestResults mouseEvent = PrepareMouseEvent(
      HitTestRequest(EnumSet<HitTestRequest::Type> {HitTestRequest::Type::Release,
                                                    HitTestRequest::Type::DisallowUserAgentShadowContent}),
      platformMouseEvent);

    // FIXME: Mouse capture, subframes and remote frames.

    RefPtr<dom::Node> targetNode = ShareRefPtr(mouseEvent.TargetNode());
    bool swallowMouseUpEvent = !DispatchMouseEvent(dom::DOMStringAtom(EventNames::MouseUp), targetNode.get(),
                                                   _clickCount, platformMouseEvent, FireMouseOverOut::Yes);

    bool swallowClickEvent =
      SwallowAnyClickEvent(platformMouseEvent, mouseEvent, IgnoreAncestorNodesForClickEvent::No);

    if (auto *resizeLayer = _resizeLayer.get())
    {
      resizeLayer->SetInResizeMode(false);
      _resizeLayer = nullptr;
    }

    bool swallowMouseReleaseEvent = false;
    if (!swallowMouseUpEvent)
    {
      swallowMouseReleaseEvent = HandleMouseReleaseEvent(mouseEvent);
    }

    InvalidateClick();

    return swallowMouseUpEvent || swallowClickEvent || swallowMouseReleaseEvent;
  }

  MouseEventWithHitTestResults EventHandler::PrepareMouseEvent(const HitTestRequest &request,
                                                               const PlatformMouseEvent &mouseEvent) noexcept
  {
    _lastPlatformMouseEvent = mouseEvent;
    Ref frame = ShareRef(_frame);
    krys_assert(frame->Document());
    return frame->Document()->PrepareMouseEvent(
      request, DocumentPointForWindowPoint(*frame, mouseEvent.Position()), mouseEvent);
  }

  void EventHandler::UpdateMouseEventTargetNode(dom::DOMStringAtom eventType, dom::Node *targetNode,
                                                const PlatformMouseEvent &platformMouseEvent,
                                                FireMouseOverOut fireMouseOverOut) noexcept
  {
    (void)eventType;
    Ref frame = ShareRef(_frame);

    // If we're capturing, we always go right to that element.
    if (_capturingMouseEventsElement)
    {
      _elementUnderMouse = _capturingMouseEventsElement;
    }
    else
    {
      // If the target node is a non-element, dispatch on the parent.
      _elementUnderMouse = GetElementOrAncestorElementForNode(targetNode);
    }

    krys_debug_assert(!_elementUnderMouse || &_elementUnderMouse->NodeDocument() == frame->Document());
    krys_debug_assert(!_lastElementUnderMouse
                      || &_lastElementUnderMouse->NodeDocument() == frame->Document());

    // Fire mouseout/mouseover if the mouse has shifted to a different node.
    if (fireMouseOverOut == FireMouseOverOut::Yes)
    {
      // FIXME: Telling scrollable areas about the mouse (notifyScrollableAreasOfMouseEvents).

      if (_lastElementUnderMouse && &_lastElementUnderMouse->NodeDocument() != frame->Document())
      {
        _lastElementUnderMouse = nullptr;
      }

      if (_lastElementUnderMouse != _elementUnderMouse)
      {
        // FIXME: Only dispatch mouseenter and mouseleave when an ancestor has a capturing listener or the
        // element has a listener for them (they don't bubble), once targets can say which listeners they
        // have; this optimization avoids O(n^2) capturing event-handler checks.
        List<Ref<dom::Element>> leftElementsChain;
        for (dom::Element *element = _lastElementUnderMouse.get(); element;
             element = element->ParentElementInComposedTree())
        {
          leftElementsChain.push_back(ShareRef(*element));
        }
        List<RefPtr<dom::Element>> elementsUnderMouse;
        for (dom::Element *element = _elementUnderMouse.get(); element;
             element = element->ParentElementInComposedTree())
        {
          elementsUnderMouse.push_back(ShareRefPtr(element));
        }

        List<RefPtr<dom::Element>> enteredElementsChain = elementsUnderMouse;
        if (!leftElementsChain.empty() && !enteredElementsChain.empty()
            && leftElementsChain.back().get() == enteredElementsChain.back().get())
        {
          size_t minHeight = std::min(leftElementsChain.size(), enteredElementsChain.size());
          size_t i;
          for (i = 0; i < minHeight; ++i)
          {
            auto &enteredElement = enteredElementsChain[enteredElementsChain.size() - i - 1];
            if (leftElementsChain[leftElementsChain.size() - i - 1].get() != enteredElement.get())
            {
              break;
            }
          }
          leftElementsChain.erase(leftElementsChain.end() - static_cast<ptrdiff_t>(i),
                                  leftElementsChain.end());
          enteredElementsChain.erase(enteredElementsChain.end() - static_cast<ptrdiff_t>(i),
                                     enteredElementsChain.end());
        }

        if (RefPtr<dom::Element> lastElementUnderMouse = _lastElementUnderMouse)
        {
          (void)lastElementUnderMouse->DispatchMouseEvent(
            platformMouseEvent, dom::DOMStringAtom(EventNames::MouseOut), 0, _elementUnderMouse.get());
        }

        for (auto &chain : leftElementsChain)
        {
          (void)chain->DispatchMouseEvent(platformMouseEvent, dom::DOMStringAtom(EventNames::MouseLeave), 0,
                                          _elementUnderMouse.get());
        }

        if (RefPtr<dom::Element> elementUnderMouse = _elementUnderMouse)
        {
          (void)elementUnderMouse->DispatchMouseEvent(
            platformMouseEvent, dom::DOMStringAtom(EventNames::MouseOver), 0, _lastElementUnderMouse.get());
        }

        for (auto it = enteredElementsChain.rbegin(); it != enteredElementsChain.rend(); ++it)
        {
          auto &chain = *it;
          if (!chain)
          {
            continue;
          }

          if (!IsElementAnAncestorOfLastElementUnderMouse(chain.get()))
          {
            (void)chain->DispatchMouseEvent(platformMouseEvent, dom::DOMStringAtom(EventNames::MouseEnter), 0,
                                            _lastElementUnderMouse.get());
          }
        }

        if (!elementsUnderMouse.empty())
        {
          elementsUnderMouse.erase(elementsUnderMouse.begin());
        }
        _ancestorsOfLastElementUnderMouse = krys::move(elementsUnderMouse);
      }

      // Event handling may have moved the element to a different document.
      if (_elementUnderMouse && &_elementUnderMouse->NodeDocument() != frame->Document())
      {
        ClearElementUnderMouse();
      }

      _lastElementUnderMouse = _elementUnderMouse;
    }
  }

  void EventHandler::ClearElementUnderMouse() noexcept
  {
    _elementUnderMouse = nullptr;
  }

  bool EventHandler::IsElementAnAncestorOfLastElementUnderMouse(dom::Element *element) const noexcept
  {
    if (!element)
    {
      return false;
    }

    return std::ranges::any_of(_ancestorsOfLastElementUnderMouse, [&](const RefPtr<dom::Element> &ancestor)
                               { return ancestor.get() == element; });
  }

  bool EventHandler::DispatchMouseEvent(dom::DOMStringAtom eventType, dom::Node *targetNode, int32 clickCount,
                                        const PlatformMouseEvent &platformMouseEvent,
                                        FireMouseOverOut fireMouseOverOut) noexcept
  {
    _mouseMoveTargetOverride = nullptr;

    // FIXME: The window's last user click event (updateLastUserClickEvent), once there's a Window.
    UpdateMouseEventTargetNode(eventType, targetNode, platformMouseEvent, fireMouseOverOut);

    bool isMouseDownEvent = eventType == EventNames::MouseDown;

    RefPtr<dom::Element> elementUnderMouse = _elementUnderMouse;
    if (eventType == EventNames::MouseMove && _mouseMoveTargetOverride)
    {
      // If _mouseMoveTargetOverride is set, targetNode must have been disconnected during the execution of
      // this method. One situation this may occur is an event listener for a boundary event immediately
      // deleting the target node. In this scenario, the `mousemove` event that follows should be sent to the
      // parent node instead.
      // FIXME: Is there a more elegant way to handle this case? Should we do the same for other events
      // besides `mousemove`?
      elementUnderMouse = _mouseMoveTargetOverride;
    }

    if (elementUnderMouse)
    {
      auto result = elementUnderMouse->DispatchMouseEvent(platformMouseEvent, eventType, clickCount);
      _mouseMoveTargetOverride = nullptr;
      _capturesDragging = CapturesDragging::InabilityReason::Unknown;
      if (result.DefaultPrevented == dom::Element::EventIsDefaultPrevented::Yes)
      {
        if (isMouseDownEvent)
        {
          _capturesDragging = CapturesDragging::InabilityReason::MousePressIsCancelled;
        }
        else if (eventType == EventNames::MouseMove)
        {
          _capturesDragging = CapturesDragging::InabilityReason::MouseMoveIsCancelled;
        }
      }
      if (result.Dispatched == dom::Element::EventIsDispatched::No)
      {
        return false;
      }
    }

    if (!isMouseDownEvent)
    {
      return true;
    }

    // If clicking on a frame scrollbar, do not make any change to which element is focused.
    if (auto *view = _frame.View();
        view && view->ScrollbarAtPoint(FlooredIntPoint(platformMouseEvent.Position())))
    {
      return true;
    }

    // Walk up the tree from the element under the mouse to an element to focus, or focus the viewport if
    // there's none.
    // FIXME: A press inside a selected range in the focused element doesn't move the focus, once there's a
    // selection. (Form controls are mouse focusable here, as on GTK and WPE, so the latest focus trigger
    // needs no workaround for those that aren't.)
    RefPtr<dom::Element> element;
    if (RefPtr<dom::Element> pressedElement = _elementUnderMouse)
    {
      element = ShareRefPtr(html::FocusAlgorithms::ClickFocusableInclusiveAncestor(*pressedElement));
    }

    // Only change the focus when clicking scrollbars if it can be transferred to a mouse focusable node.
    if (!element && IsInsideScrollbar(FlooredIntPoint(platformMouseEvent.Position())))
    {
      return false;
    }

    dom::FocusOptions options {.Trigger = dom::FocusTrigger::Click};
    if (element)
    {
      html::FocusAlgorithms::FocusingSteps(*element, nullptr, options);
    }
    else if (RefPtr<dom::Document> document = ShareRefPtr(_frame.Document()))
    {
      html::FocusAlgorithms::FocusingSteps(*document, nullptr, options);
    }
    // FIXME: Selecting the contents of a shadow host's focus delegate (findTargetAndUpdateFocusAppearance).
    return true;
  }

  void EventHandler::ScheduleScrollEvent() noexcept
  {
    SetFrameWasScrolledByUser();
    if (!_frame.View())
    {
      return;
    }
    if (auto *document = _frame.Document())
    {
      document->AddPendingScrollEventTarget(*document, dom::ScrollEventType::Scroll);
    }
  }

  bool EventHandler::KeyEvent(const PlatformKeyboardEvent &keyEvent) noexcept
  {
    // FIXME: Not treating an unhandled key event as user interaction (userDidInteractWithPage, the last
    // handled user gesture), once there are user gestures.
    return InternalKeyEvent(keyEvent);
  }

  bool EventHandler::InternalKeyEvent(const PlatformKeyboardEvent &initialKeyEvent) noexcept
  {
    using Type = PlatformEvent::Type;

    if (initialKeyEvent.GetType() == Type::KeyDown && initialKeyEvent.Key() == u8"Escape")
    {
      if (auto *page = _frame.GetPage(); page && page->GetPointerLockController().GetElement())
      {
        page->GetPointerLockController().RequestPointerUnlockAndForceCursorVisible();
      }
    }

    // FIXME: Escape hiding validation messages and leaving fullscreen; the caps lock state of text fields;
    // pan scrolling.

    // Check for cases where we are too early for events -- possible unmatched key up from pressing return in
    // the location bar.
    RefPtr<dom::Element> element = ShareRefPtr(EventTargetElementForDocument(_frame.Document()));
    if (!element)
    {
      return false;
    }

    // A key press other than Escape is an activation-triggering input event: the window is notified before
    // the event is dispatched. https://html.spec.whatwg.org/#activation-triggering-input-event
    if (initialKeyEvent.GetType() != Type::KeyUp && initialKeyEvent.GetType() != Type::Char
        && initialKeyEvent.Key() != u8"Escape")
    {
      if (auto *window = _frame.Window())
      {
        window->NotifyActivated();
      }
    }

    // FIXME: Typing gestures and access keys (handleAccessKey).

    // FIXME: it would be fair to let an input method handle KeyUp events before DOM dispatch.
    if (initialKeyEvent.GetType() == Type::KeyUp || initialKeyEvent.GetType() == Type::Char)
    {
      return !element->DispatchKeyEvent(initialKeyEvent);
    }

    PlatformKeyboardEvent keyDownEvent = initialKeyEvent;
    if (keyDownEvent.GetType() != Type::RawKeyDown)
    {
      keyDownEvent.DisambiguateKeyDownEvent(Type::RawKeyDown);
    }
    Ref<dom::KeyboardEvent> keydown = dom::KeyboardEvent::Create(keyDownEvent, _frame.Window());

    auto setHasFocusVisibleIfNeeded = [&](dom::Element &element)
    {
      // If the user interacts with the page via the keyboard, the currently focused element should match
      // :focus-visible. Just typing a modifier key is not considered user interaction with the page, but
      // Shift + a (or Caps Lock + a) is considered an interaction.
      bool userHasInteractedViaKeyword =
        keydown->ModifierKeys().IsEmpty()
        || ((keydown->ShiftKey() || keydown->CapsLockKey()) && !initialKeyEvent.Text().empty());

      if (element.Focused() && userHasInteractedViaKeyword)
      {
        element.SetHasFocusVisible(true);
      }
    };
    setHasFocusVisibleIfNeeded(*element);

    if (initialKeyEvent.GetType() == Type::RawKeyDown)
    {
      (void)dom::EventDispatcher::DispatchToTarget(*keydown, element.get());
      // FIXME: Also when the focused frame changed, once there are subframes.
      return keydown->DefaultPrevented() || keydown->DefaultHandled()
             || DefaultKeyboardEventHandler(*keydown);
    }

    // Run input method in advance of DOM event handling. This may result in the IM
    // modifying the page prior the keydown event, but this behaviour is necessary
    // in order to match IE:
    // 1. preventing default handling of keydown and keypress events has no effect on IM input;
    // 2. if an input method handles the event, its keyCode is set to 229 in keydown event.
    _frame.Document()->GetEditor().HandleInputMethodKeydown(*keydown);

    bool handledByInputMethod = keydown->DefaultHandled();

    if (handledByInputMethod)
    {
      keyDownEvent.SetWindowsVirtualKeyCode(CompositionEventKeyCode);
      keydown = dom::KeyboardEvent::Create(keyDownEvent, _frame.Window());
      keydown->SetTarget(element);
      keydown->SetIsDefaultEventHandlerIgnored();
    }

    // FIXME: Accessibility stopping the propagation (accessibilityPreventsEventPropagation), with
    // accessibility.
    (void)dom::EventDispatcher::DispatchToTarget(*keydown, element.get());
    if (handledByInputMethod)
    {
      _frame.Document()->GetEditor().DidDispatchInputMethodKeydown(*keydown);
      return true;
    }

    // FIXME: Also when the focused frame changed, once there are subframes; the backward compatibility mode,
    // which dispatches a cancelled keypress anyway.
    bool keydownResult =
      keydown->DefaultPrevented() || keydown->DefaultHandled() || DefaultKeyboardEventHandler(*keydown);
    if (keydownResult)
    {
      return keydownResult;
    }

    // Focus may have changed during keydown handling, so refetch element.
    element = ShareRefPtr(EventTargetElementForDocument(_frame.Document()));
    if (!element)
    {
      return false;
    }
    setHasFocusVisibleIfNeeded(*element);

    PlatformKeyboardEvent keyPressEvent = initialKeyEvent;
    keyPressEvent.DisambiguateKeyDownEvent(Type::Char);
    if (keyPressEvent.Text().empty())
    {
      return keydownResult;
    }
    Ref<dom::KeyboardEvent> keypress = dom::KeyboardEvent::Create(keyPressEvent, _frame.Window());
    if (keypress->IsComposing())
    {
      keypress->SetTarget(element);
      (void)_frame.Document()->GetEditor().HandleKeyboardEvent(*keypress);
      return keydownResult;
    }
    (void)dom::EventDispatcher::DispatchToTarget(*keypress, element.get());

    return keydownResult || keypress->DefaultPrevented() || keypress->DefaultHandled()
           || DefaultKeyboardEventHandler(*keypress);
  }

  bool EventHandler::HandleTextInputEvent(utf8_stringview text, dom::Event *underlyingEvent,
                                          dom::TextEventInputType inputType) noexcept
  {
    // Platforms should differentiate real commands like selectAll from text input in disguise (like
    // insertNewline), and avoid dispatching text input events from keydown default handlers.
    krys_debug_assert(!underlyingEvent || !underlyingEvent->IsKeyboardEvent()
                      || underlyingEvent->Type() == EventNames::KeyPress);

    RefPtr<dom::EventTarget> target;
    if (underlyingEvent)
    {
      target = underlyingEvent->Target();
    }
    else
    {
      target = ShareRefPtr(EventTargetElementForDocument(_frame.Document()));
    }
    if (!target)
    {
      return false;
    }

    auto event = dom::TextEvent::Create(text, inputType);
    event->SetUnderlyingEvent(underlyingEvent);

    (void)dom::EventDispatcher::DispatchToTarget(*event, target.get());
    if (event->DefaultPrevented())
    {
      return false;
    }
    if (event->DefaultHandled())
    {
      return true;
    }
    return _frame.Document()->GetEditor().HandleTextEvent(*event);
  }

  bool EventHandler::DefaultKeyboardEventHandler(dom::KeyboardEvent &event) noexcept
  {
    // FIXME: keyup stops animated keyboard scrolls, once there are any.

    if (event.Type() == EventNames::KeyDown)
    {
      if (_frame.Document()->GetEditor().HandleKeyboardEvent(event))
      {
        return true;
      }

      if (event.Key() == u8"Escape")
      {
        if (auto *window = _frame.Document()->Window())
        {
          // Dialogs and popovers close through their close watchers.
          (void)window->GetCloseWatcherManager().EscapeKeyHandler(event);
        }
        return false;
      }
      // FIXME: Backspace (history navigation).
      if (event.KeyIdentifier() == u8"U+0009")
      {
        return DefaultTabEventHandler(event);
      }
      if (event.KeyIdentifier() == u8"PageDown" || event.KeyIdentifier() == u8"PageUp")
      {
        return DefaultPageUpDownEventHandler(event);
      }
      if (event.KeyIdentifier() == u8"Home" || event.KeyIdentifier() == u8"End")
      {
        return DefaultHomeEndEventHandler(event);
      }
      dom::FocusDirection direction = FocusDirectionForKey(event.KeyIdentifier());
      if (direction != dom::FocusDirection::None)
      {
        return DefaultArrowEventHandler(direction, event);
      }
      return false;
    }
    if (event.Type() == EventNames::KeyPress)
    {
      if (_frame.Document()->GetEditor().HandleKeyboardEvent(event))
      {
        return true;
      }
      if (event.CharCode() == ' ')
      {
        return DefaultSpaceEventHandler(event);
      }
    }
    return false;
  }

  bool EventHandler::DefaultKeyboardScrollEventHandler(dom::KeyboardEvent &, ScrollLogicalDirection direction,
                                                       ScrollGranularity granularity) noexcept
  {
    // FIXME: Animated keyboard scrolling (keyboardScrollRecursively), once there are animations.
    return LogicalScrollRecursively(direction, granularity);
  }

  bool EventHandler::DefaultPageUpDownEventHandler(dom::KeyboardEvent &event) noexcept
  {
    krys_debug_assert(event.Type() == EventNames::KeyDown);

    if (event.CtrlKey() || event.MetaKey() || event.AltKey() || event.ShiftKey())
    {
      return false;
    }

    ScrollLogicalDirection direction = event.KeyIdentifier() == u8"PageUp"
                                         ? ScrollLogicalDirection::ScrollBlockDirectionBackward
                                         : ScrollLogicalDirection::ScrollBlockDirectionForward;
    return DefaultKeyboardScrollEventHandler(event, direction, ScrollGranularity::Page);
  }

  bool EventHandler::DefaultHomeEndEventHandler(dom::KeyboardEvent &event) noexcept
  {
    krys_debug_assert(event.Type() == EventNames::KeyDown);

    if (event.CtrlKey() || event.MetaKey() || event.AltKey() || event.ShiftKey())
    {
      return false;
    }

    ScrollLogicalDirection direction = event.KeyIdentifier() == u8"Home"
                                         ? ScrollLogicalDirection::ScrollBlockDirectionBackward
                                         : ScrollLogicalDirection::ScrollBlockDirectionForward;
    return DefaultKeyboardScrollEventHandler(event, direction, ScrollGranularity::Document);
  }

  bool EventHandler::DefaultSpaceEventHandler(dom::KeyboardEvent &event) noexcept
  {
    krys_debug_assert(event.Type() == EventNames::KeyPress);

    if (event.CtrlKey() || event.MetaKey() || event.AltKey())
    {
      return false;
    }

    ScrollLogicalDirection direction = event.ShiftKey() ? ScrollLogicalDirection::ScrollBlockDirectionBackward
                                                        : ScrollLogicalDirection::ScrollBlockDirectionForward;
    if (LogicalScrollOverflow(direction, ScrollGranularity::Page))
    {
      return true;
    }

    RefPtr<LocalFrameView> view = ShareRefPtr(_frame.View());
    if (!view)
    {
      return false;
    }

    // FIXME: Animated keyboard scrolling (keyboardScroll), once there are animations.
    return view->LogicalScroll(direction, ScrollGranularity::Page);
  }

  bool EventHandler::DefaultArrowEventHandler(dom::FocusDirection focusDirection,
                                              dom::KeyboardEvent &event) noexcept
  {
    krys_debug_assert(event.Type() == EventNames::KeyDown);

    if (!_frame.Document()->GetSettings().SpatialNavigationEnabled)
    {
      ScrollLogicalDirection direction = ScrollLogicalDirection::ScrollBlockDirectionForward;
      switch (focusDirection)
      {
        case dom::FocusDirection::Down:
          direction = ScrollLogicalDirection::ScrollBlockDirectionForward;
          break;
        case dom::FocusDirection::Right:
          direction = ScrollLogicalDirection::ScrollInlineDirectionForward;
          break;
        case dom::FocusDirection::Up: direction = ScrollLogicalDirection::ScrollBlockDirectionBackward; break;
        case dom::FocusDirection::Left:
          direction = ScrollLogicalDirection::ScrollInlineDirectionBackward;
          break;
        case dom::FocusDirection::None:
        case dom::FocusDirection::Backward:
        case dom::FocusDirection::Forward:  krys_assert(false); break;
      }
      return DefaultKeyboardScrollEventHandler(event, direction, ScrollGranularity::Line);
    }

    if (event.CtrlKey() || event.MetaKey() || event.ShiftKey())
    {
      return false;
    }

    // Arrows and other possible directional navigation keys can be used in design
    // mode editing.
    // FIXME: Design mode, once documents have it.

    return html::FocusAlgorithms::DirectionalNavigation(*_frame.Document(), focusDirection);
  }

  bool EventHandler::ScrollOverflow(ScrollDirection direction, ScrollGranularity granularity,
                                    dom::Node *startingNode) noexcept
  {
    RefPtr<dom::Node> node = ShareRefPtr(startingNode);

    if (!node)
    {
      node = ShareRefPtr(_frame.Document()->FocusedElement());
    }

    if (!node)
    {
      node = _mousePressNode;
    }

    if (node)
    {
      // FIXME: List boxes, which scroll themselves, once there are list boxes.
      auto *r = node->Renderer();
      if (r && r->EnclosingBox().Scroll(direction, granularity))
      {
        SetFrameWasScrolledByUser();
        return true;
      }
    }

    return false;
  }

  bool EventHandler::LogicalScrollOverflow(ScrollLogicalDirection direction, ScrollGranularity granularity,
                                           dom::Node *startingNode) noexcept
  {
    RefPtr<dom::Node> node = ShareRefPtr(startingNode);

    if (!node)
    {
      node = ShareRefPtr(_frame.Document()->FocusedElement());
    }

    if (!node)
    {
      node = _mousePressNode;
    }

    if (node)
    {
      auto *r = node->Renderer();
      if (r && r->EnclosingBox().LogicalScroll(direction, granularity))
      {
        SetFrameWasScrolledByUser();
        return true;
      }
    }

    return false;
  }

  bool EventHandler::ScrollRecursively(ScrollDirection direction, ScrollGranularity granularity,
                                       dom::Node *startingNode) noexcept
  {
    // The layout needs to be up to date to determine if we can scroll. We may be
    // here because of an onLoad event, in which case the final layout hasn't been performed yet.
    Ref frame = ShareRef(_frame);
    (void)frame->Document()->UpdateLayout();
    if (ScrollOverflow(direction, granularity, startingNode))
    {
      return true;
    }

    RefPtr<LocalFrameView> view = ShareRefPtr(frame->View());
    return view && view->Scroll(direction, granularity);
  }

  bool EventHandler::LogicalScrollRecursively(ScrollLogicalDirection direction, ScrollGranularity granularity,
                                              dom::Node *startingNode) noexcept
  {
    Ref frame = ShareRef(_frame);

    // The layout needs to be up to date to determine if we can scroll. We may be
    // here because of an onLoad event, in which case the final layout hasn't been performed yet.
    (void)frame->Document()->UpdateLayout();
    if (LogicalScrollOverflow(direction, granularity, startingNode))
    {
      return true;
    }

    RefPtr<LocalFrameView> view = ShareRefPtr(frame->View());
    return view && view->LogicalScroll(direction, granularity);
  }

  bool EventHandler::DefaultTabEventHandler(dom::KeyboardEvent &event) noexcept
  {
    krys_debug_assert(event.Type() == EventNames::KeyDown);

    // We should only advance focus on tabs if no special modifier keys are held down.
    if (event.CtrlKey() || event.MetaKey())
    {
      return false;
    }

    auto *document = _frame.Document();
    if (!document)
    {
      return false;
    }

    // FIXME: Tabs can be used in design mode editing, once there's editing; whether the page lets Tab cycle
    // through elements (tabKeyCyclesThroughElements), once the host can say.
    return html::FocusAlgorithms::SequentialNavigation(
      *document, event.ShiftKey() ? dom::FocusDirection::Backward : dom::FocusDirection::Forward);
  }

  void EventHandler::ScheduleCursorUpdate() noexcept
  {
    if (_hasScheduledCursorUpdate)
    {
      return;
    }

    auto *page = _frame.GetPage();
    if (!page)
    {
      return;
    }

    // FIXME: Whether the chrome sets cursors (supportsSettingCursor), once there's a chrome client.
    _hasScheduledCursorUpdate = true;
    page->ScheduleRenderingUpdate({RenderingUpdateStep::CursorUpdate});
  }

  void EventHandler::UpdateCursorIfNeeded() noexcept
  {
    if (std::exchange(_hasScheduledCursorUpdate, false))
    {
      UpdateCursor();
    }
  }

  void EventHandler::DispatchFakeMouseMoveEventSoon() noexcept
  {
    if (_mousePressed)
    {
      return;
    }

    if (!_lastKnownMousePosition)
    {
      return;
    }

    // FIXME: Whether the chrome wants fake mouse moves (shouldDispatchFakeMouseMoveEvents), once there's a
    // chrome client.

    // If the content has ever taken longer than fakeMouseMoveShortInterval we
    // reschedule the timer and use a longer time. This will cause the content
    // to receive these moves only after the user is done scrolling, reducing
    // pauses during the scroll.
    constexpr auto fakeMouseMoveDurationThreshold = std::chrono::milliseconds(10);
    constexpr auto fakeMouseMoveShortInterval = std::chrono::milliseconds(100);
    constexpr auto fakeMouseMoveLongInterval = std::chrono::milliseconds(250);
    if (_fakeMouseMoveEventTimer.IsActive())
    {
      _fakeMouseMoveEventTimer.Stop();
    }
    _fakeMouseMoveEventTimer.StartOneShot(_maxMouseMovedDuration > fakeMouseMoveDurationThreshold
                                            ? fakeMouseMoveLongInterval
                                            : fakeMouseMoveShortInterval);
  }

  void EventHandler::DispatchFakeMouseMoveEventSoonInQuad(const FloatQuad &quad) noexcept
  {
    auto *view = _frame.View();
    if (!view || !_lastKnownMousePosition)
    {
      return;
    }

    if (!quad.ContainsPoint(FloatPoint(view->WindowToContents(*_lastKnownMousePosition))))
    {
      return;
    }

    DispatchFakeMouseMoveEventSoon();
  }

  void EventHandler::CancelFakeMouseMoveEvent() noexcept
  {
    _fakeMouseMoveEventTimer.Stop();
  }

  void EventHandler::FakeMouseMoveEventTimerFired() noexcept
  {
    krys_debug_assert(!_mousePressed);

    Ref frame = ShareRef(_frame);
    if (!frame->View())
    {
      return;
    }

    // FIXME: Not while the page is hidden or its focus controller inactive, once there are those; the current
    // state of the modifier keys rather than the last mouse event's, once the host reports it.
    PlatformMouseEvent fakeMouseMoveEvent(*_lastKnownMousePosition,
                                          _lastKnownMouseGlobalPosition.value_or(*_lastKnownMousePosition),
                                          MouseButton::None, PlatformEvent::Type::MouseMoved, 0,
                                          _lastPlatformMouseEvent.Modifiers(), MonotonicTime::Now());
    (void)MouseMoved(fakeMouseMoveEvent);
  }

  void EventHandler::UpdateCursor() noexcept
  {
    if (!_lastKnownMousePosition)
    {
      return;
    }

    auto *view = _frame.View();
    if (!view)
    {
      return;
    }

    auto *document = _frame.Document();
    if (!document)
    {
      return;
    }

    // FIXME: The current modifier keys (getCurrentModifierState), once the host reports them.
    bool shiftKey = _lastPlatformMouseEvent.ShiftKey();

    HitTestResult result {LayoutPoint(view->WindowToContents(*_lastKnownMousePosition))};
    document->HitTest(HitTestRequest(EnumSet<HitTestRequest::Type> {
                        HitTestRequest::Type::ReadOnly, HitTestRequest::Type::AllowFrameScrollbars}),
                      result);

    UpdateCursor(*view, result, shiftKey);
  }

  void EventHandler::UpdateCursor(LocalFrameView &view, const HitTestResult &result, bool shiftKey) noexcept
  {
    // FIXME: Telling the view, and through it the chrome, to set the cursor; until then the host reads
    // CurrentMouseCursor().
    (void)view;
    if (auto optionalCursor = SelectCursor(result, shiftKey))
    {
      _currentMouseCursor = *optionalCursor;
    }
  }

  bool EventHandler::UseHandCursor(dom::Node *node, bool isOverLink, bool shiftKey) const noexcept
  {
    (void)shiftKey;
    if (!node)
    {
      return false;
    }

    // FIXME: Editable links (hasEditableStyle and the editable link behavior setting) and submit images, with
    // editing and forms.
    return isOverLink;
  }

  Maybe<Cursor> EventHandler::SelectCursor(const HitTestResult &result, bool shiftKey) const noexcept
  {
    if (auto *resizeLayer = _resizeLayer.get(); resizeLayer && resizeLayer->InResizeMode())
    {
      return null;
    }

    // FIXME: Pan scrolling and auto-hiding cursors.

    // Use always pointer cursor for scrollbars.
    if (result.GetScrollbar())
    {
      return Cursor(Cursor::Type::Pointer);
    }

    dom::Node *node = result.TargetNode();
    if (!node)
    {
      return null;
    }

    const RenderObject *renderer = node->Renderer();
    if (auto *element = DynamicDowncast<dom::Element>(node); element && result.GetPseudoElementIdentifier())
    {
      auto *pseudoElementRenderer =
        style::Styleable(*element, result.GetPseudoElementIdentifier()).Renderer();
      renderer = pseudoElementRenderer ? pseudoElementRenderer : renderer;
    }
    const RenderStyle *style = renderer ? &renderer->Style() : nullptr;
    bool horizontalText = !style || style->GetWritingMode().IsHorizontal();
    Cursor iBeam(horizontalText ? Cursor::Type::IBeam : Cursor::Type::VerticalText);

    // FIXME: Area elements (styled by their node, as they're display: none), and renderers that choose their
    // own cursor (getCursor).

    auto styleCursor = style ? style->GetCursor() : style::Cursor {null, CursorType::Auto};
    // FIXME: Cursor images, once they load.

    switch (styleCursor.Predefined)
    {
      case CursorType::Auto:
      {
        // FIXME: Image overlay text, once there are image overlays.
        bool editable = node->HasEditableStyle();

        if (UseHandCursor(node, result.IsOverLink(), shiftKey))
        {
          return Cursor(Cursor::Type::Hand);
        }

        auto *resizerRenderer = node->Renderer();

        if (resizerRenderer && resizerRenderer->IsRenderText())
        {
          resizerRenderer = resizerRenderer->Parent();
        }

        if (resizerRenderer && resizerRenderer->HasLayer())
        {
          auto &layerRenderer = Downcast<RenderLayerModelObject>(*resizerRenderer);
          if (layerRenderer.Layer()->IsPointInResizeControl(RoundedIntPoint(result.LocalPoint())))
          {
            return Cursor(layerRenderer.Style().ShouldPlaceVerticalScrollbarOnLeft()
                            ? Cursor::Type::SouthWestResize
                            : Cursor::Type::SouthEastResize);
          }
        }

        // Disabled: during a selection the cursor follows the content beneath it, as Firefox's does, rather
        // than staying an I-beam over boxes and gaps with no text.
        // During selection, use an I-beam regardless of the content beneath the cursor.
        // If a drag may be starting or we're capturing mouse events for a particular node, don't treat this
        // as a selection.
        // FIXME: Capturing mouse events, with pointer capture, if this is enabled again.
        // if (_mousePressed && MouseDownMayStartSelect() && !_mouseDownMayStartDrag
        //     && _frame.Document()->Selection().IsCaretOrRange() && renderer
        //     && renderer->Style().UsedUserSelect() != UserSelect::None)
        // {
        //   return iBeam;
        // }

        if ((editable
             || (renderer && renderer->IsRenderText() && node->CanStartSelection()
                 && renderer->Style().UsedUserSelect() != UserSelect::None))
            && !result.GetScrollbar())
        {
          return iBeam;
        }
        return Cursor(Cursor::Type::Pointer);
      }
      case CursorType::Default:      return Cursor(Cursor::Type::Pointer);
      case CursorType::None:         return Cursor(Cursor::Type::None);
      case CursorType::ContextMenu:  return Cursor(Cursor::Type::ContextMenu);
      case CursorType::Help:         return Cursor(Cursor::Type::Help);
      case CursorType::Pointer:      return Cursor(Cursor::Type::Hand);
      case CursorType::Progress:     return Cursor(Cursor::Type::Progress);
      case CursorType::Wait:         return Cursor(Cursor::Type::Wait);
      case CursorType::Cell:         return Cursor(Cursor::Type::Cell);
      case CursorType::Crosshair:    return Cursor(Cursor::Type::Cross);
      case CursorType::Text:         return Cursor(Cursor::Type::IBeam);
      case CursorType::VerticalText: return Cursor(Cursor::Type::VerticalText);
      case CursorType::Alias:        return Cursor(Cursor::Type::Alias);
      case CursorType::Copy:         return Cursor(Cursor::Type::Copy);
      case CursorType::Move:         return Cursor(Cursor::Type::Move);
      case CursorType::NoDrop:       return Cursor(Cursor::Type::NoDrop);
      case CursorType::NotAllowed:   return Cursor(Cursor::Type::NotAllowed);
      case CursorType::Grab:         return Cursor(Cursor::Type::Grab);
      case CursorType::Grabbing:     return Cursor(Cursor::Type::Grabbing);
      case CursorType::EResize:      return Cursor(Cursor::Type::EastResize);
      case CursorType::NResize:      return Cursor(Cursor::Type::NorthResize);
      case CursorType::NEResize:     return Cursor(Cursor::Type::NorthEastResize);
      case CursorType::NWResize:     return Cursor(Cursor::Type::NorthWestResize);
      case CursorType::SResize:      return Cursor(Cursor::Type::SouthResize);
      case CursorType::SEResize:     return Cursor(Cursor::Type::SouthEastResize);
      case CursorType::SWResize:     return Cursor(Cursor::Type::SouthWestResize);
      case CursorType::WResize:      return Cursor(Cursor::Type::WestResize);
      case CursorType::EWResize:     return Cursor(Cursor::Type::EastWestResize);
      case CursorType::NSResize:     return Cursor(Cursor::Type::NorthSouthResize);
      case CursorType::NESWResize:   return Cursor(Cursor::Type::NorthEastSouthWestResize);
      case CursorType::NWSEResize:   return Cursor(Cursor::Type::NorthWestSouthEastResize);
      case CursorType::ColumnResize: return Cursor(Cursor::Type::ColumnResize);
      case CursorType::RowResize:    return Cursor(Cursor::Type::RowResize);
      case CursorType::AllScroll:    return Cursor(Cursor::Type::Move);
      case CursorType::ZoomIn:       return Cursor(Cursor::Type::ZoomIn);
      case CursorType::ZoomOut:      return Cursor(Cursor::Type::ZoomOut);
    }
    return Cursor(Cursor::Type::Pointer);
  }

  bool EventHandler::PassMousePressEventToScrollbar(MouseEventWithHitTestResults &mouseEvent,
                                                    Scrollbar *scrollbar) noexcept
  {
    if (!scrollbar || !scrollbar->Enabled())
    {
      return false;
    }
    SetFrameWasScrolledByUser();
    return scrollbar->MouseDown(mouseEvent.Event());
  }

  void EventHandler::SetFrameWasScrolledByUser() noexcept
  {
    if (RefPtr<LocalFrameView> view = ShareRefPtr(_frame.View()))
    {
      view->SetLastUserScrollType(LocalFrameView::UserScrollType::Explicit);
    }
  }

  void EventHandler::UpdateLastScrollbarUnderMouse(Scrollbar *scrollbar,
                                                   SetOrClearLastScrollbar setOrClear) noexcept
  {
    if (_lastScrollbarUnderMouse.get() != scrollbar)
    {
      // Send mouse exited to the old scrollbar.
      if (RefPtr<Scrollbar> lastScrollbarUnderMouse = _lastScrollbarUnderMouse)
      {
        (void)lastScrollbarUnderMouse->MouseExited();
      }

      // Send mouse entered if we're setting a new scrollbar.
      if (scrollbar && setOrClear == SetOrClearLastScrollbar::Set)
      {
        scrollbar->MouseEntered();
        _lastScrollbarUnderMouse = ShareRefPtr(scrollbar);
      }
      else
      {
        _lastScrollbarUnderMouse = nullptr;
      }
    }
  }

  bool EventHandler::IsInsideScrollbar(const IntPoint &windowPoint) const noexcept
  {
    if (RefPtr<dom::Document> document = ShareRefPtr(_frame.Document()))
    {
      HitTestResult result {LayoutPoint(windowPoint)};
      (void)document->HitTest(
        HitTestRequest(EnumSet<HitTestRequest::Type> {HitTestRequest::Type::ReadOnly,
                                                      HitTestRequest::Type::DisallowUserAgentShadowContent}),
        result);
      return result.GetScrollbar();
    }
    return false;
  }

  bool EventHandler::HandleWheelEvent(const PlatformWheelEvent &event) noexcept
  {
    EnumSet<EventHandling> handling;
    return HandleWheelEventInternal(event, handling);
  }

  bool EventHandler::HandleWheelEventInternal(const PlatformWheelEvent &event,
                                              EnumSet<EventHandling> &handling) noexcept
  {
    Ref frame = ShareRef(_frame);
    RefPtr<dom::Document> document = ShareRefPtr(frame->Document());
    if (!document)
    {
      return false;
    }

    RefPtr<LocalFrameView> view = ShareRefPtr(frame->View());
    if (!view)
    {
      return false;
    }

    if (auto *page = frame->GetPage())
    {
      if (auto &pointerLockController = page->GetPointerLockController(); pointerLockController.IsLocked())
      {
        pointerLockController.DispatchLockedWheelEvent(event);
        return true;
      }
    }

    _isHandlingWheelEvent = true;

    // FIXME: Latching and the wheel delta filter, once there are touchpad gestures.
    SetFrameWasScrolledByUser();
    SetLastKnownMousePosition(DoublePoint(event.Position().X(), event.Position().Y()),
                              DoublePoint(event.GlobalPosition().X(), event.GlobalPosition().Y()));

    HitTestRequest request;
    HitTestResult result(LayoutPoint(view->WindowToContents(event.Position())));
    (void)document->HitTest(request, result);

    RefPtr<dom::Element> element = ShareRefPtr(result.TargetElement());

    // FIXME: Widgets and subframes (isOverWidget, passWheelEventToWidget), once there are subframes.

    if (element)
    {
      if (!element->DispatchWheelEvent(event, handling))
      {
        _isHandlingWheelEvent = false;
        // FIXME: Scroll snapping (processWheelEventForScrollSnap), once there's scroll snapping.
        return true;
      }
    }

    // Event handling may have disconnected the frame.
    view = ShareRefPtr(frame->View());
    if (!view)
    {
      _isHandlingWheelEvent = false;
      return false;
    }

    auto filteredDelta = view->DeltaForPropagation(event.Delta());
    if (view->ShouldBlockScrollPropagation(filteredDelta))
    {
      _isHandlingWheelEvent = false;
      return true;
    }

    return ProcessWheelEventForScrolling(event.CopyWithDelta(filteredDelta));
  }

  bool EventHandler::ProcessWheelEventForScrolling(const PlatformWheelEvent &event) noexcept
  {
    // We do another check on the frame view because the event handler can run JS which results in the frame
    // getting destroyed.
    RefPtr<LocalFrameView> view = ShareRefPtr(_frame.View());

    bool didHandleEvent = view ? HandleWheelEventInScrollableArea(event, *view) : false;
    _isHandlingWheelEvent = false;
    return didHandleEvent;
  }

  bool EventHandler::HandleWheelEventInAppropriateEnclosingBox(
    dom::Node *startNode, const dom::WheelEvent &wheelEvent, const FloatSize &filteredPlatformDelta) noexcept
  {
    bool shouldHandleEvent = wheelEvent.DeltaX() || wheelEvent.DeltaY();
    if (!startNode->Renderer())
    {
      return false;
    }

    if (!shouldHandleEvent)
    {
      return false;
    }

    // Script-made wheel events don't scroll (scrollViaNonPlatformEvent).
    auto &platformEvent = wheelEvent.UnderlyingPlatformEvent();
    if (!platformEvent)
    {
      return false;
    }

    return HandleWheelEventInAppropriateEnclosingBox(
      startNode, platformEvent->CopyWithDelta(filteredPlatformDelta),
      FloatSize(static_cast<float>(wheelEvent.DeltaX()), static_cast<float>(wheelEvent.DeltaY())));
  }

  bool EventHandler::HandleWheelEventInAppropriateEnclosingBox(dom::Node *startNode,
                                                               const PlatformWheelEvent &event,
                                                               FloatSize biasedDelta) noexcept
  {
    if (!startNode->Renderer())
    {
      return false;
    }

    auto scrollableAreaForBox = [](RenderBox &box) -> ScrollableArea *
    {
      if (auto *renderListBox = DynamicDowncast<RenderListBox>(box))
      {
        return renderListBox;
      }

      if (box.HasLayer())
      {
        return box.Layer()->GetScrollableArea();
      }
      return nullptr;
    };

    RenderBox *currentEnclosingBox = &startNode->Renderer()->EnclosingBox();

    while (currentEnclosingBox)
    {
      if (auto *boxScrollableArea = scrollableAreaForBox(*currentEnclosingBox))
      {
        bool scrollingWasHandled = ScrollableAreaCanHandleEvent(event, *boxScrollableArea)
                                   && HandleWheelEventInScrollableArea(event, *boxScrollableArea);
        if (scrollingWasHandled)
        {
          return true;
        }

        biasedDelta = boxScrollableArea->DeltaForPropagation(biasedDelta);
        if (boxScrollableArea->ShouldBlockScrollPropagation(biasedDelta))
        {
          return true;
        }
      }

      currentEnclosingBox = currentEnclosingBox->ContainingBlock();
      if (!currentEnclosingBox || currentEnclosingBox->IsRenderView())
      {
        return false;
      }
    }
    return false;
  }

  bool EventHandler::ScrollableAreaCanHandleEvent(const PlatformWheelEvent &wheelEvent,
                                                  ScrollableArea &scrollableArea) noexcept
  {
    auto biasedDelta = wheelEvent.Delta();

    auto verticalSide = ScrollableArea::TargetSideForScrollDelta(-biasedDelta, ScrollEventAxis::Vertical);
    if (verticalSide && !scrollableArea.IsPinnedOnSide(*verticalSide))
    {
      return true;
    }

    auto horizontalSide = ScrollableArea::TargetSideForScrollDelta(-biasedDelta, ScrollEventAxis::Horizontal);
    if (horizontalSide && !scrollableArea.IsPinnedOnSide(*horizontalSide))
    {
      return true;
    }
    // FIXME: A scroll that overscroll-behavior blocks rubber-bands (overscrollBehaviorAllowsRubberBand), once
    // there's rubber-banding.

    return false;
  }

  bool EventHandler::HandleWheelEventInScrollableArea(const PlatformWheelEvent &wheelEvent,
                                                      ScrollableArea &scrollableArea) noexcept
  {
    // FIXME: Gesture states (updateWheelGestureState), with touchpad gestures.
    return scrollableArea.HandleWheelEventForScrolling(wheelEvent);
  }

  bool EventHandler::DefaultWheelEventHandler(dom::Node *startNode, dom::WheelEvent &wheelEvent) noexcept
  {
    if (!startNode)
    {
      return false;
    }

    FloatSize filteredPlatformDelta(static_cast<float>(wheelEvent.DeltaX()),
                                    static_cast<float>(wheelEvent.DeltaY()));
    if (auto &platformEvent = wheelEvent.UnderlyingPlatformEvent())
    {
      filteredPlatformDelta = platformEvent->Delta();
    }

    // FIXME: The wheel delta filter and latching, once there are touchpad gestures.
    return HandleWheelEventInAppropriateEnclosingBox(startNode, wheelEvent, filteredPlatformDelta);
  }

  bool EventHandler::HandleTouchScroll(const PlatformWheelEvent &event) noexcept
  {
    Ref frame = ShareRef(_frame);
    RefPtr<dom::Document> document = ShareRefPtr(frame->Document());
    RefPtr<LocalFrameView> view = ShareRefPtr(frame->View());
    if (!document || !view || (!event.DeltaX() && !event.DeltaY()))
    {
      return false;
    }

    SetFrameWasScrolledByUser();

    HitTestRequest request;
    HitTestResult result(LayoutPoint(view->WindowToContents(event.Position())));
    (void)document->HitTest(request, result);

    if (RefPtr<dom::Element> element = ShareRefPtr(result.TargetElement()))
    {
      if (HandleWheelEventInAppropriateEnclosingBox(element.get(), event, event.Delta()))
      {
        return true;
      }
    }

    auto filteredDelta = view->DeltaForPropagation(event.Delta());
    if (view->ShouldBlockScrollPropagation(filteredDelta))
    {
      return true;
    }

    return HandleWheelEventInScrollableArea(event.CopyWithDelta(filteredDelta), *view);
  }

#pragma region Selection

  namespace
  {
    void SetSelectionIfNeeded(FrameSelection &selection, const VisibleSelection &newSelection) noexcept
    {
      if (selection.Selection() != newSelection && selection.ShouldChangeSelection(newSelection))
      {
        selection.SetSelection(newSelection, FrameSelection::DefaultSetSelectionOptions(UserTriggered::Yes));
      }
    }

    KRYS_NODISCARD bool DispatchSelectStart(dom::Node *node) noexcept
    {
      if (!node || !node->Renderer())
      {
        return true;
      }

      auto type = dom::DOMStringAtom(EventNames::SelectStart);
      return dom::EventDispatcher::FireEvent<dom::Event>(type, *node, [&](dom::Event &event)
                                                         { event.InitEvent(type, true, true); });
    }

    // FIXME: Image overlays (ImageOverlay::isInsideOverlay) and nodes that select on mouse down
    // (shouldSelectOnMouseDown), once there are any.
    KRYS_NODISCARD RefPtr<dom::Node> NodeToSelectOnMouseDownForNode(dom::Node &targetNode) noexcept
    {
      return dom::Position::RootUserSelectAllForNode(&targetNode);
    }

    KRYS_NODISCARD VisibleSelection ExpandSelectionToRespectSelectOnMouseDown(
      dom::Node &targetNode, const VisibleSelection &selection) noexcept
    {
      RefPtr<dom::Node> nodeToSelect = NodeToSelectOnMouseDownForNode(targetNode);
      if (!nodeToSelect)
      {
        return selection;
      }

      VisibleSelection newSelection(selection);
      newSelection.SetBase(dom::PositionBeforeNode(nodeToSelect.get())
                             .Upstream(EditingBoundaryCrossingRule::CanCrossEditingBoundary));
      newSelection.SetExtent(dom::PositionAfterNode(nodeToSelect.get())
                               .Downstream(EditingBoundaryCrossingRule::CanCrossEditingBoundary));

      return newSelection;
    }

    KRYS_NODISCARD bool ShouldAvoidExtendingSelectionOnClick(const dom::Node &targetNode,
                                                             const VisibleSelection &selection) noexcept
    {
      if (Is<dom::Text>(targetNode))
      {
        return false;
      }

      if (selection.IsContentEditable())
      {
        return false;
      }

      auto range = selection.ToNormalizedRange();
      if (!range)
      {
        return false;
      }

      if (range->Collapsed())
      {
        return false;
      }

      constexpr TextIteratorBehaviors plainTextOptions {
        TextIteratorBehavior::EmitsObjectReplacementCharacters, TextIteratorBehavior::EntersTextControls};

      if (HasAnyPlainText(*range, plainTextOptions, IgnoreCollapsedRanges::Yes))
      {
        return false;
      }

      return true;
    }

    KRYS_NODISCARD uint64 TextDistance(const dom::Position &start, const dom::Position &end) noexcept
    {
      auto range = dom::MakeSimpleRange({start, end});
      if (!range)
      {
        return 0;
      }
      return CharacterCount(*range, TextIteratorBehavior::EmitsCharactersBetweenAllVisiblePositions);
    }

    // FIXME: The editor's isSelectTrailingWhitespaceEnabled, with the editor; off, as its client's default
    // is.
    KRYS_NODISCARD AppendTrailingWhitespace
      AppendTrailingWhitespaceForEvent(const MouseEventWithHitTestResults &) noexcept
    {
      return DontAppendTrailingWhitespace;
    }
  }

  bool EventHandler::ExpandAndUpdateSelectionForMouseDownIfNeeded(dom::Node &targetNode,
                                                                  const VisibleSelection &selection,
                                                                  TextGranularity granularity) noexcept
  {
    auto expandedSelection = ExpandSelectionToRespectSelectOnMouseDown(targetNode, selection);
    if (ShouldAvoidExtendingSelectionOnClick(targetNode, expandedSelection))
    {
      return false;
    }

    return UpdateSelectionForMouseDownDispatchingSelectStart(&targetNode, expandedSelection, granularity);
  }

  bool EventHandler::UpdateSelectionForMouseDownDispatchingSelectStart(dom::Node *targetNode,
                                                                       const VisibleSelection &selection,
                                                                       TextGranularity granularity) noexcept
  {
    if (dom::Position::NodeIsUserSelectNone(targetNode))
    {
      return false;
    }

    if (!DispatchSelectStart(targetNode))
    {
      _mouseDownMayStartSelect = false;
      return false;
    }

    if (selection.IsOrphan())
    {
      _mouseDownMayStartSelect = false;
      return false;
    }

    if (selection.IsRange())
    {
      _selectionInitiationState = ExtendedSelection;
      _dragStartSelection = selection.Range();
    }
    else
    {
      granularity = TextGranularity::CharacterGranularity;
      _selectionInitiationState = PlacedCaret;
    }

    _frame.Document()->Selection().SetSelectionByMouseIfDifferent(selection, granularity);

    return true;
  }

  void EventHandler::SelectClosestWordFromHitTestResult(
    const HitTestResult &result, AppendTrailingWhitespace appendTrailingWhitespace) noexcept
  {
    RefPtr<dom::Node> targetNode = ShareRefPtr(result.TargetNode());
    VisibleSelection newSelection;

    if (targetNode && targetNode->Renderer())
    {
      VisiblePosition pos(
        targetNode->Renderer()->VisiblePositionForPoint(result.LocalPoint(), HitTestSource::User));
      if (pos.IsNotNull())
      {
        newSelection = VisibleSelection(pos);
        newSelection.ExpandUsingGranularity(TextGranularity::WordGranularity);
      }

      if (appendTrailingWhitespace == ShouldAppendTrailingWhitespace && newSelection.IsRange())
      {
        newSelection.AppendTrailingWhitespace();
      }

      ExpandAndUpdateSelectionForMouseDownIfNeeded(*targetNode, newSelection,
                                                   TextGranularity::WordGranularity);
    }
  }

  void EventHandler::SelectClosestContextualWordFromHitTestResult(
    const HitTestResult &result, AppendTrailingWhitespace appendTrailingWhitespace) noexcept
  {
    RefPtr<dom::Node> targetNode = ShareRefPtr(result.TargetNode());
    VisibleSelection newSelection;

    if (targetNode && targetNode->Renderer())
    {
      // FIXME: The system's lookup of the word under the point
      // (selectClosestWordFromHitTestResultBasedOnLookup), which only the Mac has.
      VisiblePosition pos(
        targetNode->Renderer()->VisiblePositionForPoint(result.LocalPoint(), HitTestSource::User));
      if (pos.IsNotNull())
      {
        newSelection = VisibleSelection(pos);
        newSelection.ExpandUsingGranularity(TextGranularity::WordGranularity);
      }

      if (appendTrailingWhitespace == ShouldAppendTrailingWhitespace && newSelection.IsRange())
      {
        newSelection.AppendTrailingWhitespace();
      }

      (void)UpdateSelectionForMouseDownDispatchingSelectStart(
        targetNode.get(), ExpandSelectionToRespectSelectOnMouseDown(*targetNode, newSelection),
        TextGranularity::WordGranularity);
    }
  }

  void EventHandler::SelectClosestContextualWordOrLinkFromHitTestResult(
    const HitTestResult &result, AppendTrailingWhitespace appendTrailingWhitespace) noexcept
  {
    // FIXME: In the editable case, word selection sometimes selects content that isn't underneath the mouse.
    // If the selection is non-editable, we do word selection to make it easier to use the contextual menu
    // items available for text selections. But only if we're above text.
    if (!_frame.Document()->Selection().Selection().IsContentEditable()
        && !Is<dom::Text>(result.TargetNode()))
    {
      return;
    }

    RefPtr<dom::Element> urlElement = ShareRefPtr(result.URLElement());
    if (!urlElement || !IsDraggableLink(*urlElement))
    {
      if (RefPtr<dom::Node> targetNode = ShareRefPtr(result.TargetNode());
          targetNode && IsEditableNode(*targetNode))
      {
        SelectClosestWordFromHitTestResult(result, appendTrailingWhitespace);
        return;
      }

      SelectClosestContextualWordFromHitTestResult(result, appendTrailingWhitespace);
      return;
    }

    if (RefPtr<dom::Node> targetNode = ShareRefPtr(result.TargetNode()); targetNode && targetNode->Renderer())
    {
      VisibleSelection newSelection;
      VisiblePosition pos(
        targetNode->Renderer()->VisiblePositionForPoint(result.LocalPoint(), HitTestSource::User));
      if (pos.IsNotNull() && pos.DeepEquivalent().DeprecatedNode()->IsDescendantOf(*urlElement))
      {
        newSelection = VisibleSelection::SelectionFromContentsOfNode(urlElement.get());
      }

      (void)UpdateSelectionForMouseDownDispatchingSelectStart(
        targetNode.get(), ExpandSelectionToRespectSelectOnMouseDown(*targetNode, newSelection),
        TextGranularity::WordGranularity);
    }
  }

  bool EventHandler::SendContextMenuEvent(const PlatformMouseEvent &event) noexcept
  {
    Ref<LocalFrame> frame = ShareRef(_frame);

    // Context menus should not be handled while pointer is locked.
    if (auto *page = frame->GetPage(); !page || page->GetPointerLockController().IsLocked())
    {
      return false;
    }

    RefPtr<dom::Document> doc = ShareRefPtr(frame->Document());
    RefPtr<LocalFrameView> view = ShareRefPtr(frame->View());
    if (!doc || !view)
    {
      return false;
    }

    // Caret blinking is normally un-suspended in HandleMouseReleaseEvent, but we
    // won't receive that event once the context menu is up.
    doc->Selection().SetCaretBlinkingSuspended(false);
    // Clear mouse press state to avoid initiating a drag while context menu is up.
    _mousePressed = false;
    const auto flooredEventPosition = FlooredIntPoint(event.Position());
    LayoutPoint viewportPos {view->WindowToContents(flooredEventPosition)};
    MouseEventWithHitTestResults mouseEvent = doc->PrepareMouseEvent(
      HitTestRequest(EnumSet<HitTestRequest::Type> {HitTestRequest::Type::Active,
                                                    HitTestRequest::Type::DisallowUserAgentShadowContent}),
      DoublePoint(viewportPos), event);

    // Do not show context menus when clicking on scrollbars.
    if (mouseEvent.GetScrollbar() || view->ScrollbarAtPoint(flooredEventPosition))
    {
      return false;
    }

    if (EditingBehavior(doc->GetSettings().EditingBehaviorType).ShouldSelectOnContextualMenuClick()
        && !doc->Selection().Contains(viewportPos))
    {
      _mouseDownMayStartSelect = true; // context menu events are always allowed to perform a selection
      SelectClosestContextualWordOrLinkFromHitTestResult(mouseEvent.GetHitTestResult(),
                                                         AppendTrailingWhitespaceForEvent(mouseEvent));
    }

    return !DispatchMouseEvent(dom::DOMStringAtom(EventNames::ContextMenu), mouseEvent.TargetNode(), 0, event,
                               FireMouseOverOut::No);
  }

  namespace
  {
    KRYS_NODISCARD dom::DOMStringAtom EventNameForTouchPointState(PlatformTouchPoint::State state) noexcept
    {
      switch (state)
      {
        case PlatformTouchPoint::TouchReleased:  return dom::DOMStringAtom(EventNames::TouchEnd);
        case PlatformTouchPoint::TouchCancelled: return dom::DOMStringAtom(EventNames::TouchCancel);
        case PlatformTouchPoint::TouchPressed:   return dom::DOMStringAtom(EventNames::TouchStart);
        case PlatformTouchPoint::TouchMoved:     return dom::DOMStringAtom(EventNames::TouchMove);
        // TouchStationary state is not converted to touch events, so fall through to assert.
        default:                                 krys_debug_assert_msg(false, "Not reached."); return dom::DOMStringAtom::Null();
      }
    }

    KRYS_NODISCARD HitTestResult HitTestResultInFrame(LocalFrame *frame, const LayoutPoint &point,
                                                      EnumSet<HitTestRequest::Type> hitType) noexcept
    {
      HitTestResult result(point);

      if (!frame || !frame->Document() || !frame->Document()->GetRenderView())
      {
        return result;
      }

      if (frame->View())
      {
        IntRect rect = frame->View()->VisibleContentRect();
        if (!rect.Contains(RoundedIntPoint(point)))
        {
          return result;
        }
      }
      RefPtr<dom::Document> document = ShareRefPtr(frame->Document());
      HitTestRequest request(hitType);
      (void)document->HitTest(request, result);
      return result;
    }
  }

  bool EventHandler::HandleTouchEvent(const PlatformTouchEvent &event) noexcept
  {
    Ref frame = ShareRef(_frame);
    if (!frame->View() || !frame->Document())
    {
      return false;
    }

    // The pointers of the touches that ended before are forgotten, so a later touch with the same id starts
    // afresh. They're kept until now so the host's tap after a touch ends sees whether the touch's
    // pointerdown was cancelled.
    if (auto *page = frame->GetPage())
    {
      for (auto pointerId : std::exchange(_endedTouchPointerIds, {}))
      {
        page->GetPointerCaptureController().TouchWithIdentifierWasRemoved(pointerId);
      }
    }

    // First build up the lists to use for the 'touches', 'targetTouches' and 'changedTouches' attributes
    // in the event.

    // Holds the complete set of touches on the screen and will be used as the 'touches' list in the event.
    RefPtr<dom::TouchList> touches = dom::TouchList::Create();

    // A different view on the 'touches' list above, filtered and grouped by event target. Used for the
    // 'targetTouches' list in the event.
    HashMap<dom::EventTarget *, RefPtr<dom::TouchList>> touchesByTarget;

    // Array of touches per state, used to assemble the 'changedTouches' list in the event.
    struct Touches
    {
      // The touches corresponding to the particular change state this struct instance represents.
      RefPtr<dom::TouchList> TouchList;
      // The targets involved in TouchList, in the order they were first seen.
      List<RefPtr<dom::EventTarget>> Targets;
    };
    std::array<Touches, PlatformTouchPoint::TouchStateEnd> changedTouches;

    const List<PlatformTouchPoint> &points = event.TouchPoints();

    // The end of a touch is an activation-triggering input event: the window is notified before the event
    // is dispatched.
    // https://html.spec.whatwg.org/#activation-triggering-input-event
    if (event.GetType() == PlatformEvent::Type::TouchEnd)
    {
      if (auto *window = frame->Window())
      {
        window->NotifyActivated();
      }
    }

    bool freshTouchEvents = true;
    bool allTouchReleased = true;
    for (auto &point : points)
    {
      if (point.GetState() != PlatformTouchPoint::TouchPressed)
      {
        freshTouchEvents = false;
      }
      if (point.GetState() != PlatformTouchPoint::TouchReleased
          && point.GetState() != PlatformTouchPoint::TouchCancelled)
      {
        allTouchReleased = false;
      }
    }

    for (uint32 index = 0; index < points.size(); index++)
    {
      auto &point = points[index];
      PlatformTouchPoint::State pointState = point.GetState();
      LayoutPoint pagePoint = LayoutPoint(frame->View()->WindowToContents(FlooredIntPoint(point.Pos())));

      EnumSet<HitTestRequest::Type> hitType {HitTestRequest::Type::TouchEvent};
      // The HitTestRequest types used for mouse events map quite adequately
      // to touch events. Note that in addition to meaning that the hit test
      // should affect the active state of the current node if necessary,
      // HitTestRequest::Type::Active signifies that the hit test is taking place
      // with the mouse (or finger in this case) being pressed.
      switch (pointState)
      {
        case PlatformTouchPoint::TouchPressed: hitType.Add(HitTestRequest::Type::Active); break;
        case PlatformTouchPoint::TouchMoved:
          hitType.Add(HitTestRequest::Type::Active);
          hitType.Add(HitTestRequest::Type::Move);
          hitType.Add(HitTestRequest::Type::ReadOnly);
          break;
        case PlatformTouchPoint::TouchReleased:
        case PlatformTouchPoint::TouchCancelled: hitType.Add(HitTestRequest::Type::Release); break;
        case PlatformTouchPoint::TouchStationary:
          hitType.Add(HitTestRequest::Type::Active);
          hitType.Add(HitTestRequest::Type::ReadOnly);
          break;
        default: krys_debug_assert_msg(false, "Not reached."); break;
      }

      // Increment the platform touch id by 1 to avoid storing a key of 0 in the hashmap.
      uint32 touchPointTargetKey = point.Id() + 1;
      RefPtr<dom::EventTarget> touchTarget;
      if (pointState == PlatformTouchPoint::TouchPressed)
      {
        HitTestResult result;
        if (freshTouchEvents)
        {
          EnumSet<HitTestRequest::Type> freshHitType = hitType;
          freshHitType.Add(HitTestRequest::Type::AllowChildFrameContent);
          result = HitTestResultAtPoint(pagePoint, freshHitType);
          _originatingTouchPointTargetKey = touchPointTargetKey;
        }
        else if (_originatingTouchPointDocument && _originatingTouchPointDocument->Frame())
        {
          Ref documentFrame = ShareRef(*_originatingTouchPointDocument->Frame());
          DoublePoint framePoint = documentFrame->View()->WindowToContents(point.Pos());
          result =
            HitTestResultInFrame(documentFrame.get(), LayoutPoint(RoundedIntPoint(framePoint)), hitType);
          if (!result.InnerNode())
          {
            continue;
          }
        }
        else
        {
          continue;
        }

        RefPtr<dom::Element> element = ShareRefPtr(result.TargetElement());
        if (!element)
        {
          continue;
        }

        // Record the originating touch document even if it does not have a touch listener.
        if (freshTouchEvents)
        {
          _originatingTouchPointDocument = ShareRefPtr(&element->NodeDocument());
          freshTouchEvents = false;
        }
        // FIXME: Skip documents without touch event listeners (hasTouchEventHandlers), once targets can say
        // which listeners they have.
        _originatingTouchPointTargets.insert_or_assign(touchPointTargetKey, element);
        touchTarget = element;
      }
      else if (pointState == PlatformTouchPoint::TouchReleased
               || pointState == PlatformTouchPoint::TouchCancelled)
      {
        // No need to perform a hit-test since we only need to unset :hover and :active states.
        if (allTouchReleased)
        {
          frame->Document()->UpdateHoverActiveState(HitTestRequest(hitType), nullptr);
        }
        if (touchPointTargetKey == _originatingTouchPointTargetKey)
        {
          _originatingTouchPointTargetKey = 0;
        }

        // The target should be the original target for this touch, so get it from the hashmap. As it's a
        // release or cancel we also remove it from the map.
        if (auto it = _originatingTouchPointTargets.find(touchPointTargetKey);
            it != _originatingTouchPointTargets.end())
        {
          touchTarget = krys::move(it->second);
          _originatingTouchPointTargets.erase(it);
        }
      }
      else
      {
        // No hittest is performed on move or stationary, since the target is not allowed to change anyway.
        if (auto it = _originatingTouchPointTargets.find(touchPointTargetKey);
            it != _originatingTouchPointTargets.end())
        {
          touchTarget = it->second;
        }
      }

      auto *touchTargetNode = DynamicDowncast<dom::Node>(touchTarget.get());
      if (!touchTargetNode)
      {
        continue;
      }
      Ref document = ShareRef(touchTargetNode->NodeDocument());
      RefPtr<LocalFrame> targetFrame = ShareRefPtr(document->Frame());
      if (!targetFrame || !targetFrame->View())
      {
        continue;
      }

      // FIXME: Pass the touch delta for pointermove events by remembering the position per pointer.
      if (auto *page = targetFrame->GetPage())
      {
        auto &pointerCaptureController = page->GetPointerCaptureController();
        pointerCaptureController.DispatchEventForTouchAtIndex(*touchTarget, event, index, !index,
                                                              targetFrame->View(), {0, 0});
        if (pointState == PlatformTouchPoint::TouchReleased
            || pointState == PlatformTouchPoint::TouchCancelled)
        {
          _endedTouchPointerIds.push_back(dom::PointerEvent::PointerIdForTouch(point));
        }
      }

      // pagePoint should always be relative to the target elements containing frame.
      pagePoint = LayoutPoint(targetFrame->View()->WindowToContents(FlooredIntPoint(point.Pos())));

      float scaleFactor = targetFrame->PageZoomFactor() * targetFrame->View()->FrameScaleFactor();

      DoublePoint adjustedPagePoint(pagePoint.X().ToDouble() / scaleFactor,
                                    pagePoint.Y().ToDouble() / scaleFactor);
      DoubleSize adjustedRadius = point.Radius().ScaledBy(1 / scaleFactor);

      auto touch = dom::Touch::Create(targetFrame.get(), touchTarget.get(), static_cast<int32>(point.Id()),
                                      point.ScreenPos(), adjustedPagePoint, adjustedRadius,
                                      point.RotationAngle(), point.Force(), point.Twist());

      // Ensure this target's touch list exists, even if it ends up empty, so it can always be passed to
      // TouchEvent::Create below.
      auto targetTouchesIterator = touchesByTarget.find(touchTarget.get());
      if (targetTouchesIterator == touchesByTarget.end())
      {
        targetTouchesIterator = touchesByTarget.emplace(touchTarget.get(), dom::TouchList::Create()).first;
      }

      // touches and targetTouches should only contain information about touches still on the screen, so if
      // this point is released or cancelled it will only appear in the changedTouches list.
      if (pointState != PlatformTouchPoint::TouchReleased && pointState != PlatformTouchPoint::TouchCancelled)
      {
        touches->Append(Ref<dom::Touch>(touch));
        targetTouchesIterator->second->Append(Ref<dom::Touch>(touch));
      }

      // Now build up the correct list for changedTouches.
      // Note that any touches that are in the TouchStationary state (e.g. if
      // the user had several points touched but did not move them all) should
      // never be in the changedTouches list so we do not handle them explicitly here.
      if (pointState != PlatformTouchPoint::TouchStationary)
      {
        krys_debug_assert(pointState < PlatformTouchPoint::TouchStateEnd);
        auto &changed = changedTouches[pointState];
        if (!changed.TouchList)
        {
          changed.TouchList = dom::TouchList::Create();
        }
        changed.TouchList->Append(krys::move(touch));
        if (std::ranges::find(changed.Targets, touchTarget) == changed.Targets.end())
        {
          changed.Targets.push_back(touchTarget);
        }
      }
    }
    _touchPressed = touches->Length() > 0;
    if (allTouchReleased)
    {
      _originatingTouchPointDocument = nullptr;
    }

    // Now iterate the changedTouches list and the targets within it, sending events to the targets as
    // required.
    bool swallowedEvent = false;
    RefPtr<dom::TouchList> emptyList = dom::TouchList::Create();
    for (uint32 state = 0; state != PlatformTouchPoint::TouchStateEnd; ++state)
    {
      auto &changed = changedTouches[state];
      if (!changed.TouchList)
      {
        continue;
      }

      // When sending a touch cancel event, use empty touches and targetTouches lists.
      bool isTouchCancelEvent = state == PlatformTouchPoint::TouchCancelled;
      RefPtr<dom::TouchList> &effectiveTouches = isTouchCancelEvent ? emptyList : touches;
      auto stateName = EventNameForTouchPointState(static_cast<PlatformTouchPoint::State>(state));

      for (auto &target : changed.Targets)
      {
        auto *targetNode = DynamicDowncast<dom::Node>(target.get());
        krys_debug_assert(targetNode);

        RefPtr<dom::TouchList> targetTouches =
          isTouchCancelEvent ? emptyList : touchesByTarget.find(target.get())->second;
        krys_debug_assert(targetTouches);

        auto *targetFrame = targetNode->NodeDocument().Frame();
        Ref<dom::TouchEvent> touchEvent = dom::TouchEvent::Create(
          effectiveTouches.get(), targetTouches.get(), changed.TouchList.get(), stateName,
          targetFrame ? targetFrame->View() : nullptr, {}, event.Modifiers());
        (void)dom::EventDispatcher::DispatchToTarget(*touchEvent, target.get());
        swallowedEvent = swallowedEvent || touchEvent->DefaultPrevented() || touchEvent->DefaultHandled();
      }
    }

    return swallowedEvent;
  }

  bool EventHandler::HandleMousePressEventDoubleClick(const MouseEventWithHitTestResults &event) noexcept
  {
    if (event.Event().Button() != MouseButton::Left)
    {
      return false;
    }

    auto &selection = _frame.Document()->Selection();
    if (selection.IsRange())
    {
      // A double-click when range is already selected
      // should not change the selection.  So, do not call
      // SelectClosestWordFromHitTestResult, but do set
      // _selectionInitiationState to prevent HandleMouseReleaseEvent
      // from setting caret selection.
      _selectionInitiationState = ExtendedSelection;
      _dragStartSelection = selection.Selection().Range();
    }
    else if (MouseDownMayStartSelect())
    {
      SelectClosestWordFromHitTestResult(event.GetHitTestResult(), AppendTrailingWhitespaceForEvent(event));
    }

    return true;
  }

  bool EventHandler::HandleMousePressEventTripleClick(const MouseEventWithHitTestResults &event) noexcept
  {
    if (event.Event().Button() != MouseButton::Left)
    {
      return false;
    }

    RefPtr<dom::Node> targetNode = ShareRefPtr(event.TargetNode());
    if (!(targetNode && targetNode->Renderer() && MouseDownMayStartSelect()))
    {
      return false;
    }

    VisibleSelection newSelection;
    VisiblePosition pos(
      targetNode->Renderer()->VisiblePositionForPoint(event.LocalPoint(), HitTestSource::User));
    if (pos.IsNotNull())
    {
      newSelection = VisibleSelection(pos);
      newSelection.ExpandUsingGranularity(TextGranularity::ParagraphGranularity);
    }

    return ExpandAndUpdateSelectionForMouseDownIfNeeded(*targetNode, newSelection,
                                                        TextGranularity::ParagraphGranularity);
  }

  // FIXME: Taps put the caret at the nearest word boundary (wordBoundaryForPositionWithoutCrossingLine), once
  // the visible units have the granularity queries; the editor client's
  // shouldAllowSingleClickToChangeSelection, with the editor.
  bool EventHandler::HandleMousePressEventSingleClick(const MouseEventWithHitTestResults &event) noexcept
  {
    Ref<LocalFrame> frame = ShareRef(_frame);
    (void)frame->Document()->UpdateLayoutIgnorePendingStylesheets();
    RefPtr<dom::Node> targetNode = ShareRefPtr(event.TargetNode());
    if (!targetNode || !targetNode->Renderer() || !MouseDownMayStartSelect())
    {
      return false;
    }

    auto &selection = frame->Document()->Selection();

    // Extend the selection if the Shift key is down, unless the click is in a link.
    bool extendSelection = event.Event().ShiftKey() && !event.IsOverLink();

    // Don't restart the selection when the mouse is pressed on an
    // existing selection so we can allow for text dragging.
    if (auto *view = frame->View())
    {
      LayoutPoint vPoint {view->WindowToContents(FlooredIntPoint(event.Event().Position()))};
      if (!extendSelection && selection.Contains(vPoint))
      {
        _mouseDownWasSingleClickInSelection = true;
        return false;
      }
    }

    VisiblePosition visiblePosition(
      targetNode->Renderer()->VisiblePositionForPoint(event.LocalPoint(), HitTestSource::User));
    if (visiblePosition.IsNull())
    {
      visiblePosition = VisiblePosition(FirstPositionInOrBeforeNode(targetNode.get()));
    }
    dom::Position pos = visiblePosition.DeepEquivalent();

    VisibleSelection newSelection = selection.Selection();
    TextGranularity granularity = TextGranularity::CharacterGranularity;

    if (extendSelection && newSelection.IsCaretOrRange())
    {
      VisibleSelection selectionInUserSelectAll = ExpandSelectionToRespectSelectOnMouseDown(
        *targetNode, VisibleSelection(pos, VisibleSelection::DefaultAffinity));
      if (selectionInUserSelectAll.IsRange())
      {
        if (selectionInUserSelectAll.Start() < newSelection.Start())
        {
          pos = selectionInUserSelectAll.Start();
        }
        else if (newSelection.End() < selectionInUserSelectAll.End())
        {
          pos = selectionInUserSelectAll.End();
        }
      }

      if (!EditingBehavior(frame->Document()->GetSettings().EditingBehaviorType)
             .ShouldConsiderSelectionAsDirectional()
          && pos.IsNotNull())
      {
        // Shift-click shouldn't deselect a selection that was made right-to-left.
        dom::Position start = newSelection.Start();
        dom::Position end = newSelection.End();
        auto distanceToStart = TextDistance(start, pos);
        auto distanceToEnd = TextDistance(pos, end);
        if (distanceToStart <= distanceToEnd)
        {
          newSelection = VisibleSelection(end, pos);
        }
        else
        {
          newSelection = VisibleSelection(start, pos);
        }
      }
      else
      {
        if (newSelection.GetDirectionality() == Directionality::Strong)
        {
          RefPtr<dom::Node> baseNode =
            ShareRefPtr(newSelection.IsBaseFirst() ? newSelection.Base().ComputeNodeAfterPosition()
                                                   : newSelection.Base().ComputeNodeBeforePosition());
          if (!baseNode)
          {
            baseNode = ShareRefPtr(newSelection.Base().GetContainerNode());
          }
          if (baseNode)
          {
            auto expandedBaseSelection = ExpandSelectionToRespectSelectOnMouseDown(
              *baseNode, VisibleSelection {newSelection.VisibleBase()});
            expandedBaseSelection.ExpandUsingGranularity(selection.Granularity());
            if (expandedBaseSelection.IsRange())
            {
              if (newSelection.IsBaseFirst() && pos < newSelection.Start())
              {
                newSelection.SetBase(expandedBaseSelection.End());
              }
              else if (!newSelection.IsBaseFirst() && newSelection.End() < pos)
              {
                newSelection.SetBase(expandedBaseSelection.Start());
              }
            }
          }
        }
        newSelection.SetExtent(pos);
      }

      if (selection.Granularity() != TextGranularity::CharacterGranularity)
      {
        granularity = selection.Granularity();
        newSelection.ExpandUsingGranularity(selection.Granularity());
      }
    }
    else
    {
      newSelection =
        ExpandSelectionToRespectSelectOnMouseDown(*targetNode, VisibleSelection(visiblePosition));
    }

    return UpdateSelectionForMouseDownDispatchingSelectStart(targetNode.get(), newSelection, granularity);
  }

  // FIXME: The chrome client's shouldUseMouseEventForSelection, quirks and image overlay text, once there are
  // any.
  bool EventHandler::CanMouseDownStartSelect(const MouseEventWithHitTestResults &event) const noexcept
  {
    auto *node = event.TargetNode();

    if (!node || !node->Renderer())
    {
      return true;
    }

    return node->CanStartSelection() || dom::Position::NodeIsUserSelectAll(node);
  }

  // FIXME: The textInteractionEnabled setting, once there's one.
  bool EventHandler::MouseDownMayStartSelect() const noexcept
  {
    return _mouseDownMayStartSelect;
  }

  // FIXME: Widgets, SVG panning and the scrollbar corner (isPointInScrollbarCorner), with widgets, SVG and
  // resizers; image overlays' text, once there are image overlays.
  bool EventHandler::HandleMousePressEvent(const MouseEventWithHitTestResults &event) noexcept
  {
    Ref<LocalFrame> frame = ShareRef(_frame);

    // Reset drag state.
    SetDragStateSource(nullptr);

    CancelFakeMouseMoveEvent();

    (void)frame->Document()->UpdateLayoutIgnorePendingStylesheets();

    // If we got the event back, that must mean it wasn't prevented,
    // so it's allowed to start a drag or selection if it wasn't in a scrollbar.
    _mouseDownMayStartSelect = CanMouseDownStartSelect(event) && !event.GetScrollbar();

    bool singleClick = event.Event().ClickCount() <= 1;

    // Careful that the drag starting logic stays in sync with EventMayStartDrag()
    // FIXME: EventMayStartDrag() does not check for shift key press, link or image event targets.
    // Single mouse down on links or images can always trigger drag-n-drop.
    bool isMouseDownOnLinkOrImage = event.IsOverLink() || event.GetHitTestResult().GetImage();
    _mouseDownMayStartDrag = singleClick && (!event.Event().ShiftKey() || isMouseDownOnLinkOrImage)
                             && ShouldAllowMouseDownToStartDrag();

    _mouseDownWasSingleClickInSelection = false;

    _mouseDownEvent = event.Event();

    _mousePressNode = ShareRefPtr(event.TargetNode());
    frame->Document()->SetSequentialFocusNavigationStartingPoint(event.TargetNode());

    _dragStartPosition = FlooredIntPoint(event.Event().Position());

    _mousePressed = true;
    _selectionInitiationState = HaveNotStartedSelection;

    bool swallowEvent = false;
    if (event.Event().ClickCount() == 2)
    {
      swallowEvent = HandleMousePressEventDoubleClick(event);
    }
    else if (event.Event().ClickCount() >= 3)
    {
      swallowEvent = HandleMousePressEventTripleClick(event);
    }
    else
    {
      swallowEvent = HandleMousePressEventSingleClick(event);
    }

    _mouseDownMayStartAutoscroll = [&]
    {
      // FIXME: Only within the visual viewport (visualViewportRect), once there's one.
      if (MouseDownMayStartSelect())
      {
        return true;
      }

      auto *mousePressBox =
        _mousePressNode ? DynamicDowncast<RenderBox>(_mousePressNode->Renderer()) : nullptr;
      if (mousePressBox && mousePressBox->CanBeProgramaticallyScrolled())
      {
        return true;
      }

      return false;
    }();

    return swallowEvent;
  }

  VisiblePosition EventHandler::SelectionExtentRespectingEditingBoundary(const VisibleSelection &selection,
                                                                         const LayoutPoint &localPoint,
                                                                         dom::Node *targetNode) noexcept
  {
    FloatPoint selectionEndPoint = localPoint;
    auto *editableElement = selection.RootEditableElement();

    if (!targetNode || !targetNode->Renderer())
    {
      return {};
    }

    dom::Node *adjustedTarget = targetNode;
    if (editableElement && !editableElement->Contains(targetNode))
    {
      if (!editableElement->Renderer())
      {
        return {};
      }

      FloatPoint absolutePoint = targetNode->Renderer()->LocalToAbsolute(FloatPoint(selectionEndPoint));
      selectionEndPoint = editableElement->Renderer()->AbsoluteToLocal(absolutePoint);
      adjustedTarget = editableElement;
    }

    return adjustedTarget->Renderer()->VisiblePositionForPoint(LayoutPoint(selectionEndPoint),
                                                               HitTestSource::User);
  }

  bool EventHandler::HandleMouseDraggedEvent(const MouseEventWithHitTestResults &event,
                                             CheckDragHysteresis checkDragHysteresis) noexcept
  {
    if (!_mousePressed)
    {
      return false;
    }

    Ref<LocalFrame> frame = ShareRef(_frame);

    if (HandleDrag(event, checkDragHysteresis))
    {
      return true;
    }

    RefPtr<dom::Node> targetNode = ShareRefPtr(event.TargetNode());
    if (event.Event().Button() != MouseButton::Left || !targetNode)
    {
      return false;
    }

    RenderObject *renderer = targetNode->Renderer();
    if (!renderer)
    {
      auto *parent = dom::TreeQueries::ParentOrShadowHostElement(*targetNode);
      if (!parent)
      {
        return false;
      }

      renderer = parent->Renderer();
      if (!renderer || !renderer->IsRenderListBox())
      {
        return false;
      }
    }

    _mouseDownMayStartDrag = false;

    if (_mouseDownMayStartAutoscroll && !_autoscrollController->PanScrollInProgress())
    {
      _autoscrollController->StartAutoscrollForSelection(renderer);
      _mouseDownMayStartAutoscroll = false;
    }

    if (_selectionInitiationState != ExtendedSelection)
    {
      HitTestResult result(_mouseDownContentsPosition);
      frame->Document()->HitTest(HitTestRequest(), result);

      UpdateSelectionForMouseDrag(result);
    }
    else
    {
      (void)targetNode->NodeDocument().UpdateStyleIfNeeded();
    }
    UpdateSelectionForMouseDrag(event.GetHitTestResult());
    return true;
  }

  void EventHandler::UpdateSelectionForMouseDrag() noexcept
  {
    auto *view = _frame.View();
    if (!view)
    {
      return;
    }
    auto *document = _frame.Document();
    if (!document)
    {
      return;
    }

    constexpr EnumSet<HitTestRequest::Type> hitType {HitTestRequest::Type::ReadOnly,
                                                     HitTestRequest::Type::Active, HitTestRequest::Type::Move,
                                                     HitTestRequest::Type::DisallowUserAgentShadowContent};
    HitTestResult result(
      LayoutPoint(view->WindowToContents(FlooredIntPoint(_lastKnownMousePosition.value_or(DoublePoint())))));
    document->HitTest(HitTestRequest(hitType), result);
    UpdateSelectionForMouseDrag(result);
  }

  // FIXME: SVG text (keeping the selection in the containing block), image overlays
  // (shouldExtendSelectionToTargetNode), once there are any.
  void EventHandler::UpdateSelectionForMouseDrag(const HitTestResult &hitTestResult) noexcept
  {
    if (!MouseDownMayStartSelect())
    {
      return;
    }

    RefPtr<dom::Node> target = ShareRefPtr(hitTestResult.TargetNode());
    if (!target)
    {
      return;
    }

    auto &selection = _frame.Document()->Selection();
    VisiblePosition targetPosition = SelectionExtentRespectingEditingBoundary(
      selection.Selection(), hitTestResult.LocalPoint(), target.get());

    // Don't modify the selection if we're not on a node.
    if (targetPosition.IsNull())
    {
      return;
    }

    // Restart the selection if this is the first mouse move. This work is usually
    // done in HandleMousePressEvent, but not if the mouse press was on an existing selection.
    VisibleSelection oldSelection = selection.Selection();
    auto newSelection = oldSelection;

    if (_selectionInitiationState == HaveNotStartedSelection && !DispatchSelectStart(target.get()))
    {
      _mouseDownMayStartSelect = false;
      return;
    }

    bool shouldSetDragStartSelection = false;
    if (_selectionInitiationState != ExtendedSelection)
    {
      // Always extend selection here because it's caused by a mouse drag
      _selectionInitiationState = ExtendedSelection;
      newSelection = VisibleSelection(targetPosition);
      shouldSetDragStartSelection = true;
    }

    RefPtr<dom::Node> rootUserSelectAllForMousePressNode =
      dom::Position::RootUserSelectAllForNode(_mousePressNode.get());
    if (rootUserSelectAllForMousePressNode
        && rootUserSelectAllForMousePressNode == dom::Position::RootUserSelectAllForNode(target.get()))
    {
      newSelection.SetBase(dom::PositionBeforeNode(rootUserSelectAllForMousePressNode.get())
                             .Upstream(EditingBoundaryCrossingRule::CanCrossEditingBoundary));
      newSelection.SetExtent(dom::PositionAfterNode(rootUserSelectAllForMousePressNode.get())
                               .Downstream(EditingBoundaryCrossingRule::CanCrossEditingBoundary));
    }
    else
    {
      auto dragStart = [&]
      {
        return _mousePressNode->Renderer()->VisiblePositionForPoint(LayoutPoint(_dragStartPosition),
                                                                    HitTestSource::User);
      };
      auto targetVisiblePosition = [&]
      {
        return target->Renderer()->VisiblePositionForPoint(hitTestResult.LocalPoint(), HitTestSource::User);
      };

      // Reset base for user select all when base is inside user-select-all area and extent < base.
      if (rootUserSelectAllForMousePressNode && targetVisiblePosition() < dragStart())
      {
        newSelection.SetBase(dom::PositionAfterNode(rootUserSelectAllForMousePressNode.get())
                               .Downstream(EditingBoundaryCrossingRule::CanCrossEditingBoundary));
      }

      RefPtr<dom::Node> rootUserSelectAllForTarget = dom::Position::RootUserSelectAllForNode(target.get());
      if (rootUserSelectAllForTarget && _mousePressNode->Renderer() && targetVisiblePosition() < dragStart())
      {
        newSelection.SetExtent(dom::PositionBeforeNode(rootUserSelectAllForTarget.get())
                                 .Upstream(EditingBoundaryCrossingRule::CanCrossEditingBoundary));
      }
      else if (rootUserSelectAllForTarget && _mousePressNode->Renderer())
      {
        newSelection.SetExtent(dom::PositionAfterNode(rootUserSelectAllForTarget.get())
                                 .Downstream(EditingBoundaryCrossingRule::CanCrossEditingBoundary));
      }
      else
      {
        newSelection.SetExtent(targetPosition);
      }
    }

    if (selection.Granularity() != TextGranularity::CharacterGranularity)
    {
      newSelection.ExpandUsingGranularity(selection.Granularity());
      if (!newSelection.IsBaseFirst() && !oldSelection.IsBaseFirst()
          && oldSelection.End() < newSelection.End())
      {
        newSelection.SetBase(oldSelection.End());
      }
      else if (newSelection.IsBaseFirst() && !oldSelection.IsBaseFirst()
               && oldSelection.Start() < newSelection.Start() && _dragStartSelection)
      {
        VisibleSelection dragStartSelection {*_dragStartSelection};
        dragStartSelection.ExpandUsingGranularity(selection.Granularity());
        if (!dragStartSelection.IsNoneOrOrphaned())
        {
          newSelection.SetBase(dragStartSelection.Start());
        }
      }
    }

    if (shouldSetDragStartSelection)
    {
      _dragStartSelection = newSelection.Range();
    }

    selection.SetSelectionByMouseIfDifferent(newSelection, selection.Granularity(),
                                             FrameSelection::EndPointsAdjustmentMode::AdjustAtBidiBoundary);
  }

  // FIXME: The event loop's and the view's activation handling of the mouse up (handleMouseUp), caret
  // browsing, and pasting the global selection on a middle click, with the platform, caret browsing and the
  // pasteboard.
  bool EventHandler::HandleMouseReleaseEvent(const MouseEventWithHitTestResults &event) noexcept
  {
    if (AutoscrollInProgress())
    {
      StopAutoscrollTimer();
    }

    Ref<LocalFrame> frame = ShareRef(_frame);

    // Used to prevent mouseMoveEvent from initiating a drag before
    // the mouse is pressed again.
    _mousePressed = false;
    _capturesDragging = false;
    _mouseDownMayStartDrag = false;
    _mouseDownMayStartSelect = false;
    _mouseDownMayStartAutoscroll = false;

    bool handled = false;

    // Clear the selection if the mouse didn't move after the last mouse
    // press and it's not a context menu click.  We do this so when clicking
    // on the selection, the selection goes away.  However, if we are
    // editing, place the caret.
    auto &selection = frame->Document()->Selection();
    if (_mouseDownWasSingleClickInSelection && _selectionInitiationState != ExtendedSelection
        && _dragStartPosition == FlooredIntPoint(event.Event().Position()) && selection.IsRange()
        && event.Event().Button() != MouseButton::Right)
    {
      VisibleSelection newSelection;
      auto *node = event.TargetNode();
      if (node && node->Renderer() && node->HasEditableStyle())
      {
        auto pos = node->Renderer()->VisiblePositionForPoint(event.LocalPoint(), HitTestSource::User);
        newSelection = VisibleSelection(pos);
      }

      SetSelectionIfNeeded(selection, newSelection);

      handled = true;
    }

    return handled;
  }

  bool EventHandler::ShouldUpdateAutoscroll() const noexcept
  {
    return MousePressed();
  }

  IntPoint EventHandler::TargetPositionInWindowForSelectionAutoscroll() const noexcept
  {
    return FlooredIntPoint(_lastKnownMousePosition.value_or(DoublePoint()));
  }

  RenderBox *EventHandler::AutoscrollRenderer() const noexcept
  {
    return _autoscrollController->AutoscrollRenderer();
  }

  bool EventHandler::AutoscrollInProgress() const noexcept
  {
    return _autoscrollController->AutoscrollInProgress();
  }

  void EventHandler::StopAutoscrollTimer(bool rendererIsBeingDestroyed) noexcept
  {
    _autoscrollController->StopAutoscrollTimer(rendererIsBeingDestroyed);
  }

#pragma endregion

#pragma region Drag and drop

  DragState &EventHandler::GetDragState() noexcept
  {
    static DragState &state = *new DragState;
    return state;
  }

  dom::Element *EventHandler::DraggedElement() noexcept
  {
    return GetDragState().Source.get();
  }

  // FIXME: Telling the accessibility tree the drag source changed (onDragElementChanged), with accessibility.
  void EventHandler::SetDragStateSource(dom::Element *element) const noexcept
  {
    GetDragState().Source = ShareRefPtr(element);
  }

  // FIXME: The origin the dragged image's data is restricted to (restrictedOriginForImageData), once there
  // are security origins.
  bool EventHandler::CanDropCurrentlyDraggedImageAsFile() const noexcept
  {
    return true;
  }

  bool EventHandler::ShouldAllowMouseDownToStartDrag() const noexcept
  {
    return true;
  }

  EnumSet<DragSourceAction> EventHandler::UpdateDragSourceActionsAllowed() const noexcept
  {
    auto *page = _frame.GetPage();
    if (!page)
    {
      return {};
    }

    auto *view = _frame.View();
    if (!view)
    {
      return {};
    }

    // The frame's view is the root view.
    return page->GetDragController().DelegateDragSourceAction(
      view->ContentsToWindow(RoundedIntPoint(_mouseDownContentsPosition)));
  }

  bool EventHandler::EventMayStartDrag(const PlatformMouseEvent &event) const noexcept
  {
    // This is a pre-flight check of whether the event might lead to a drag being started.  Be careful
    // that its logic needs to stay in sync with HandleMouseMoveEvent() and the way we set
    // _mouseDownMayStartDrag in HandleMousePressEvent
    auto *document = _frame.Document();
    if (!document)
    {
      return false;
    }

    if (event.Button() != MouseButton::Left || event.ClickCount() != 1)
    {
      return false;
    }

    auto *view = _frame.View();
    if (!view)
    {
      return false;
    }

    auto *page = _frame.GetPage();
    if (!page)
    {
      return false;
    }

    (void)UpdateDragSourceActionsAllowed();
    HitTestResult result(LayoutPoint(view->WindowToContents(FlooredIntPoint(event.Position()))));
    document->HitTest(
      HitTestRequest(EnumSet<HitTestRequest::Type> {HitTestRequest::Type::ReadOnly,
                                                    HitTestRequest::Type::DisallowUserAgentShadowContent}),
      result);
    DragState state;
    auto *targetElement = result.TargetElement();
    return targetElement
           && page->GetDragController().DraggableElement(
             &_frame, targetElement, RoundedIntPoint(result.GetHitTestLocation().Point()), state);
  }

  // FIXME: Telling the accessibility tree of the drag events (onDraggingStarted and the others), with
  // accessibility.
  bool EventHandler::DispatchDragEvent(dom::DOMStringAtom eventType, dom::Element &dragTarget,
                                       const PlatformMouseEvent &event,
                                       dom::DataTransfer &dataTransfer) noexcept
  {
    Ref<LocalFrame> frame = ShareRef(_frame);
    RefPtr<LocalFrameView> view = ShareRefPtr(frame->View());

    // FIXME: We might want to dispatch a dragleave even if the view is gone.
    if (!view)
    {
      return false;
    }

    auto dragEvent = dom::DragEvent::Create(
      eventType, true, true, true, static_cast<dom::DOMHighResTimeStamp>(event.Timestamp().count()),
      view.get(), 0, DoublePoint(FlooredIntPoint(event.GlobalPosition())),
      DoublePoint(FlooredIntPoint(event.Position())), event.MovementDelta().X(), event.MovementDelta().Y(),
      event.Modifiers(), MouseButton::Left, 0, nullptr, event.Force(), SyntheticClickType::NoTap,
      &dataTransfer);

    (void)dragTarget.DispatchEvent(*dragEvent);

    return dragEvent->DefaultPrevented();
  }

  EventHandler::DragTargetResponse EventHandler::DispatchDragEnterOrDragOverEvent(
    dom::DOMStringAtom eventType, dom::Element &target, const PlatformMouseEvent &event,
    UniquePtr<Pasteboard> &&pasteboard, EnumSet<DragOperation> sourceOperationMask,
    bool draggingFiles) noexcept
  {
    auto dataTransfer = dom::DataTransfer::CreateForUpdatingDropTarget(
      target.NodeDocument(), krys::move(pasteboard), sourceOperationMask, draggingFiles);
    bool accept = DispatchDragEvent(eventType, target, event, *dataTransfer);
    dataTransfer->MakeInvalidForSecurity();
    if (accept && !dataTransfer->DropEffectIsUninitialized())
    {
      return {true, dataTransfer->DestinationOperationMask()};
    }
    return {accept, null};
  }

  // FIXME: Subframes' drags (contentFrameForNode), once there are subframes.
  EventHandler::DragTargetResponse
    EventHandler::UpdateDragAndDrop(const PlatformMouseEvent &event,
                                    const Func<UniquePtr<Pasteboard>()> &makePasteboard,
                                    EnumSet<DragOperation> sourceOperationMask, bool draggingFiles) noexcept
  {
    Ref<LocalFrame> frame = ShareRef(_frame);
    if (!frame->View())
    {
      return {};
    }

    MouseEventWithHitTestResults mouseEvent = PrepareMouseEvent(
      HitTestRequest(EnumSet<HitTestRequest::Type> {HitTestRequest::Type::ReadOnly,
                                                    HitTestRequest::Type::DisallowUserAgentShadowContent}),
      event);

    RefPtr<dom::Element> newTarget;
    if (auto *targetNode = mouseEvent.TargetNode())
    {
      // Drag events should never go to non-element nodes (following IE, and proper mouseover/out dispatch)
      if (auto *element = DynamicDowncast<dom::Element>(*targetNode))
      {
        newTarget = ShareRefPtr(element);
      }
      else
      {
        newTarget = ShareRefPtr(dom::TreeQueries::ParentOrShadowHostElement(*targetNode));
      }
    }

    _autoscrollController->UpdateDragAndDrop(newTarget.get(), FlooredIntPoint(event.Position()),
                                             event.Timestamp());

    DragTargetResponse response;
    if (_dragTarget != newTarget)
    {
      // FIXME: this ordering was explicitly chosen to match WinIE. However,
      // it is sometimes incorrect when dragging within subframes.
      //
      // Moreover, this ordering conforms to the HTML spec's drag-and-drop processing model.
      if (newTarget)
      {
        // As per the HTML spec, we must always fire a drag event before firing a dragenter, dragleave, or
        // dragover event.
        DispatchEventToDragSourceElement(dom::DOMStringAtom(EventNames::Drag), event);
        response =
          DispatchDragEnterOrDragOverEvent(dom::DOMStringAtom(EventNames::DragEnter), *newTarget, event,
                                           makePasteboard(), sourceOperationMask, draggingFiles);
      }

      if (RefPtr<dom::Element> dragTarget = _dragTarget)
      {
        auto dataTransfer = dom::DataTransfer::CreateForUpdatingDropTarget(
          dragTarget->NodeDocument(), makePasteboard(), sourceOperationMask, draggingFiles);
        (void)DispatchDragEvent(dom::DOMStringAtom(EventNames::DragLeave), *dragTarget, event, *dataTransfer);
        dataTransfer->MakeInvalidForSecurity();
      }

      if (newTarget)
      {
        // We do not explicitly call DispatchDragEvent here because it could ultimately result in the
        // appearance that two dragover events fired. So, we mark that we should only fire a dragover event on
        // the next call to this function.
        _shouldOnlyFireDragOverEvent = true;
      }
    }
    else if (newTarget)
    {
      // Note, when dealing with sub-frames, we may need to fire only a dragover event as a drag event may
      // have been fired earlier.
      if (!_shouldOnlyFireDragOverEvent)
      {
        DispatchEventToDragSourceElement(dom::DOMStringAtom(EventNames::Drag), event);
      }
      response = DispatchDragEnterOrDragOverEvent(dom::DOMStringAtom(EventNames::DragOver), *newTarget, event,
                                                  makePasteboard(), sourceOperationMask, draggingFiles);
      _shouldOnlyFireDragOverEvent = false;
    }
    _dragTarget = krys::move(newTarget);
    return response;
  }

  void EventHandler::CancelDragAndDrop(const PlatformMouseEvent &event, UniquePtr<Pasteboard> &&pasteboard,
                                       EnumSet<DragOperation> sourceOperationMask,
                                       bool draggingFiles) noexcept
  {
    Ref<LocalFrame> frame = ShareRef(_frame);

    if (RefPtr<dom::Element> dragTarget = _dragTarget)
    {
      DispatchEventToDragSourceElement(dom::DOMStringAtom(EventNames::Drag), event);

      auto dataTransfer = dom::DataTransfer::CreateForUpdatingDropTarget(
        dragTarget->NodeDocument(), krys::move(pasteboard), sourceOperationMask, draggingFiles);
      (void)DispatchDragEvent(dom::DOMStringAtom(EventNames::DragLeave), *dragTarget, event, *dataTransfer);
      dataTransfer->MakeInvalidForSecurity();
    }
    ClearDragState();
  }

  bool EventHandler::PerformDragAndDrop(const PlatformMouseEvent &event, UniquePtr<Pasteboard> &&pasteboard,
                                        EnumSet<DragOperation> sourceOperationMask,
                                        bool draggingFiles) noexcept
  {
    Ref<LocalFrame> frame = ShareRef(_frame);

    bool preventedDefault = false;
    if (RefPtr<dom::Element> dragTarget = _dragTarget)
    {
      Ref<dom::DataTransfer> dataTransfer = dom::DataTransfer::CreateForDrop(
        dragTarget->NodeDocument(), krys::move(pasteboard), sourceOperationMask, draggingFiles);
      preventedDefault =
        DispatchDragEvent(dom::DOMStringAtom(EventNames::Drop), *dragTarget, event, *dataTransfer);
      dataTransfer->MakeInvalidForSecurity();
    }
    ClearDragState();
    return preventedDefault;
  }

  void EventHandler::ClearDragState() noexcept
  {
    StopAutoscrollTimer();
    _dragStartSelection = null;
    _dragTarget = nullptr;
    _capturingMouseEventsElement = nullptr;
    _shouldOnlyFireDragOverEvent = false;
  }

  bool EventHandler::DragHysteresisExceeded(const FloatPoint &dragViewportLocation) const noexcept
  {
    auto dragOperation = GetDragState().Type.ToSingleValue();
    krys_debug_assert(dragOperation);
    int32 threshold = GeneralDragHysteresis;
    if (dragOperation)
    {
      switch (*dragOperation)
      {
        case DragSourceAction::Selection: threshold = TextDragHysteresis; break;
        case DragSourceAction::Image:     threshold = ImageDragHysteresis; break;
        case DragSourceAction::Link:      threshold = LinkDragHysteresis; break;
        case DragSourceAction::Color:     threshold = ColorDragHystersis; break;
        case DragSourceAction::DHTML:     break;
      }
    }

    return MouseMovementExceedsThreshold(dragViewportLocation, threshold);
  }

  bool EventHandler::MouseMovementExceedsThreshold(const FloatPoint &viewportLocation,
                                                   int32 pointsThreshold) const noexcept
  {
    auto *view = _frame.View();
    if (!view)
    {
      return false;
    }
    IntPoint location = view->WindowToContents(FlooredIntPoint(viewportLocation));
    IntSize delta = location - RoundedIntPoint(_mouseDownContentsPosition);

    return std::abs(delta.Width()) >= pointsThreshold || std::abs(delta.Height()) >= pointsThreshold;
  }

  void EventHandler::InvalidateDataTransfer() noexcept
  {
    if (!GetDragState().DataTransfer)
    {
      return;
    }
    GetDragState().DataTransfer->MakeInvalidForSecurity();
    GetDragState().DataTransfer = nullptr;
  }

  // The dragged content's markers (removeDraggedContentDocumentMarkersFromAllFramesInPage) are iOS's.
  void EventHandler::DragCancelled() noexcept
  {
  }

  // The dragged content's markers (addDraggedContentMarker) are iOS's.
  void EventHandler::DidStartDrag() noexcept
  {
  }

  void EventHandler::DragSourceEndedAt(const PlatformMouseEvent &event,
                                       EnumSet<DragOperation> dragOperationMask,
                                       MayExtendDragSession) noexcept
  {
    // Send a hit test request so that RenderLayer gets a chance to update the :hover and :active
    // pseudoclasses.
    (void)PrepareMouseEvent(
      HitTestRequest(EnumSet<HitTestRequest::Type> {HitTestRequest::Type::Release,
                                                    HitTestRequest::Type::DisallowUserAgentShadowContent}),
      event);

    if (ShouldDispatchEventsToDragSourceElement())
    {
      GetDragState().DataTransfer->SetDestinationOperationMask(dragOperationMask);
      DispatchEventToDragSourceElement(dom::DOMStringAtom(EventNames::DragEnd), event);
    }
    InvalidateDataTransfer();

    SetDragStateSource(nullptr);
    // In case the drag was ended due to an escape key press we need to ensure
    // that consecutive mousemove events don't reinitiate the drag and drop.
    _mouseDownMayStartDrag = false;
  }

  void EventHandler::UpdateDragStateAfterEditDragIfNeeded(dom::Element &rootEditableElement) noexcept
  {
    // If inserting the dragged contents removed the drag source, we still want to fire dragend at the root
    // editable element.
    if (DraggedElement() && !DraggedElement()->IsConnected())
    {
      SetDragStateSource(&rootEditableElement);
    }
  }

  bool EventHandler::ShouldDispatchEventsToDragSourceElement() noexcept
  {
    return DraggedElement() && GetDragState().DataTransfer && GetDragState().ShouldDispatchEvents;
  }

  void EventHandler::DispatchEventToDragSourceElement(dom::DOMStringAtom eventType,
                                                      const PlatformMouseEvent &event) noexcept
  {
    if (ShouldDispatchEventsToDragSourceElement())
    {
      Ref<dom::Element> draggedElement = ShareRef(*DraggedElement());
      (void)DispatchDragEvent(eventType, *draggedElement, event, *GetDragState().DataTransfer);
    }
  }

  bool EventHandler::DispatchDragStartEventOnSourceElement(dom::DataTransfer &dataTransfer) noexcept
  {
    Ref<dom::Element> draggedElement = ShareRef(*DraggedElement());
    if (auto *page = _frame.GetPage())
    {
      page->GetDragController().PrepareForDragStart(_frame, GetDragState().Type, *draggedElement,
                                                    dataTransfer,
                                                    RoundedIntPoint(_mouseDownContentsPosition));
    }
    return !DispatchDragEvent(dom::DOMStringAtom(EventNames::DragStart), *draggedElement, _mouseDownEvent,
                              dataTransfer)
           && !_frame.Document()->Selection().Selection().IsInPasswordField();
  }

  // FIXME: The event loop's handling of the drag (eventLoopHandleMouseDragged) and the view activated by the
  // press (eventActivatedView), which are the Mac's; the drag image of an element the page didn't give one
  // (setDragImage), with drag images; images' origins (restrictedOriginForImageData), once there are security
  // origins.
  bool EventHandler::HandleDrag(const MouseEventWithHitTestResults &event,
                                CheckDragHysteresis checkDragHysteresis) noexcept
  {
    if (event.Event().Button() != MouseButton::Left
        || event.Event().GetType() != PlatformEvent::Type::MouseMoved)
    {
      // If we allowed the other side of the bridge to handle a drag
      // last time, then _mousePressed might still be set. So we
      // clear it now to make sure the next move after a drag
      // doesn't look like a drag.
      _mousePressed = false;
      return false;
    }

    Ref<LocalFrame> frame = ShareRef(_frame);
    auto &dragState = GetDragState();

    // Careful that the drag starting logic stays in sync with EventMayStartDrag().
    if (_mouseDownMayStartDrag && !DraggedElement())
    {
      dragState.ShouldDispatchEvents = UpdateDragSourceActionsAllowed().Contains(DragSourceAction::DHTML);

      // Try to find an element that wants to be dragged.
      HitTestResult result(_mouseDownContentsPosition);
      frame->Document()->HitTest(
        HitTestRequest(EnumSet<HitTestRequest::Type> {HitTestRequest::Type::ReadOnly,
                                                      HitTestRequest::Type::DisallowUserAgentShadowContent}),
        result);
      if (auto *page = frame->GetPage())
      {
        SetDragStateSource(page->GetDragController()
                             .DraggableElement(frame.get(), result.TargetElement(),
                                               RoundedIntPoint(_mouseDownContentsPosition), dragState)
                             .get());
      }

      if (!DraggedElement())
      {
        _mouseDownMayStartDrag = false; // no element is draggable
      }
      else
      {
        _dragMayStartSelectionInstead = dragState.Type.Contains(DragSourceAction::Selection);
      }
    }

    // For drags starting in the selection, the user must wait between the mousedown and mousedrag,
    // or else we bail on the dragging stuff and allow selection to occur
    if (_mouseDownMayStartDrag && _dragMayStartSelectionInstead
        && dragState.Type.Contains(DragSourceAction::Selection)
        && event.Event().Timestamp() - _mouseDownTimestamp < TextDragDelay)
    {
      krys_debug_assert(event.Event().GetType() == PlatformEvent::Type::MouseMoved);
      if (dragState.Type.Contains(DragSourceAction::Image))
      {
        // ... unless the mouse is over an image, then we start dragging just the image
        dragState.Type = DragSourceAction::Image;
      }
      else if (!dragState.Type.ContainsAny({DragSourceAction::DHTML, DragSourceAction::Link}))
      {
        // ... but only bail if we're not over an unselectable element.
        _mouseDownMayStartDrag = false;
        SetDragStateSource(nullptr);
      }
      else
      {
        // Prevent the following case from occuring:
        // 1. User starts a drag immediately after mouse down over an unselectable element.
        // 2. We enter this block and decided that since we're over an unselectable element, don't cancel the
        //    drag.
        // 3. The drag gets resolved as a potential selection drag below /but/ we haven't exceeded the drag
        //    hysteresis yet.
        // 4. We enter this block again, and since it's now marked as a selection drag, we cancel the drag.
        _dragMayStartSelectionInstead = false;
      }
    }

    if (!_mouseDownMayStartDrag)
    {
      return !MouseDownMayStartSelect() && !_mouseDownMayStartAutoscroll;
    }
    krys_debug_assert(DraggedElement());

    if (!dragState.Type.HasExactlyOneBitSet())
    {
      krys_debug_assert(dragState.Type.Contains(DragSourceAction::Selection));
      dragState.Type = DragSourceAction::Selection;
    }

    // We are starting a text/image/url drag, so the cursor should be an arrow
    // FIXME: Custom cursors aren't supported during drag and drop (default to pointer).
    _currentMouseCursor = Cursor(Cursor::Type::Pointer);

    if (checkDragHysteresis == ShouldCheckDragHysteresis
        && !DragHysteresisExceeded(FloatPoint(event.Event().Position())))
    {
      return true;
    }

    // Once we're past the hysteresis point, we don't want to treat this gesture as a click
    InvalidateClick();

    EnumSet<DragOperation> sourceOperationMask;

    // This does work only if we missed a dragEnd. Do it anyway, just to make sure the old dataTransfer gets
    // numbed.
    // FIXME: Consider doing this earlier in this function as the earliest point we're sure it would be safe
    // to drop an old drag.
    InvalidateDataTransfer();

    RefPtr<dom::Document> document = ShareRefPtr(frame->Document());
    if (!document)
    {
      return false;
    }

    dragState.DataTransfer = dom::DataTransfer::CreateForDrag(*document);
    auto hasNonDefaultPasteboardData = HasNonDefaultPasteboardData::No;

    if (dragState.ShouldDispatchEvents)
    {
      krys_debug_assert(DraggedElement());
      auto dragStartDataTransfer =
        dom::DataTransfer::CreateForDragStartEvent(DraggedElement()->NodeDocument());
      _mouseDownMayStartDrag = DispatchDragStartEventOnSourceElement(*dragStartDataTransfer);
      if (Downcast<StaticPasteboard>(dragStartDataTransfer->GetPasteboard()).HasNonDefaultData())
      {
        hasNonDefaultPasteboardData = HasNonDefaultPasteboardData::Yes;
      }
      dragState.DataTransfer->MoveDragState(krys::move(dragStartDataTransfer));

      if (RefPtr<dom::Element> draggedElement = ShareRefPtr(DraggedElement());
          draggedElement && dragState.Type == EnumSet<DragSourceAction> {DragSourceAction::DHTML}
          && !dragState.DataTransfer->HasDragImage())
      {
        (void)draggedElement->NodeDocument().UpdateStyleIfNeeded();
        if (!draggedElement->Renderer())
        {
          DispatchEventToDragSourceElement(dom::DOMStringAtom(EventNames::DragEnd), event.Event());
          _mouseDownMayStartDrag = false;
          InvalidateDataTransfer();
          SetDragStateSource(nullptr);
          return true;
        }
      }

      dragState.DataTransfer->MakeInvalidForSecurity();

      if (_mouseDownMayStartDrag)
      {
        // Gather values from DHTML element, if it set any.
        sourceOperationMask = dragState.DataTransfer->SourceOperationMask();

        // A drag image update can be fired as a result of kicking off the drag. Because of that
        // reentrancy, we may think we've not started the drag when that happens. So we have to assume it's
        // started before we kick it off.
        dragState.DataTransfer->SetDragHasStarted();
      }
    }

    if (_mouseDownMayStartDrag)
    {
      auto *page = frame->GetPage();
      _didStartDrag = page
                      && page->GetDragController().StartDrag(
                        *frame, dragState, sourceOperationMask, event.Event(),
                        RoundedIntPoint(_mouseDownContentsPosition), hasNonDefaultPasteboardData);
      // The host could re-enter this code and start another drag.
      if (_didStartDrag)
      {
        _mouseDownMayStartDrag = false;
        return true;
      }
      if (ShouldDispatchEventsToDragSourceElement())
      {
        // Drag was canned at the last minute. We owe dragSource a dragend event.
        DispatchEventToDragSourceElement(dom::DOMStringAtom(EventNames::DragEnd), event.Event());
        _mouseDownMayStartDrag = false;
      }
    }

    if (!_mouseDownMayStartDrag)
    {
      // Something failed to start the drag, clean up.
      InvalidateDataTransfer();
      SetDragStateSource(nullptr);
    }

    // No more default handling (like selection), whether we're past the hysteresis bounds or not
    return true;
  }

#pragma endregion
}
