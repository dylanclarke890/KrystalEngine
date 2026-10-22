#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/DOM/WheelEvent.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/EditingBehavior.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/Editing/VisibleSelection.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/Page/AutoscrollController.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/MouseEventWithHitTestResults.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/Scrollbar.hpp"
#include "Krystal.Booey/Rendering/HitTestRequest.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/HitTestSource.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Booey/Style/Values/UI/Cursor.hpp"
#include <algorithm>

namespace krys::boo
{
  namespace
  {
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

      // FIXME: The shared shadow host of the two nodes (giving listeners the illusion that a host with a user
      // agent shadow tree is a single element), once there are user agent shadow trees.
      return nullptr;
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

  void EventHandler::Clear() noexcept
  {
    ClearElementUnderMouse();
    _lastElementUnderMouse = nullptr;
    _ancestorsOfLastElementUnderMouse.clear();
    _mouseMoveTargetOverride = nullptr;
    _lastScrollbarUnderMouse = nullptr;
    _clickCount = 0;
    _clickNode = nullptr;
    _lastKnownMousePosition = null;
    _lastKnownMouseGlobalPosition = null;
    _mousePressNode = nullptr;
    _mousePressed = false;
    _mouseDownMayStartSelect = false;
    _mouseDownWasSingleClickInSelection = false;
    _selectionInitiationState = HaveNotStartedSelection;
    _mouseDownMayStartAutoscroll = false;
    _dragStartSelection = null;
    _maxMouseMovedDuration = {};
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

    // FIXME: Pointer lock, page overlays, synthetic touch events and user gestures.

    _mousePressed = true;
    SetLastKnownMousePosition(platformMouseEvent.Position(), platformMouseEvent.GlobalPosition());
    _mouseDownTimestamp = platformMouseEvent.Timestamp();
    // FIXME: Dragging (mouseDownMayStartDrag), with drag and drop.
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

    // FIXME: Resizers (isPointInResizeControl), once layers resize.

    _frame.Document()->Selection().SetCaretBlinkingSuspended(true);

    RefPtr<dom::Node> targetNode = ShareRefPtr(mouseEvent.TargetNode());
    bool swallowEvent = !DispatchMouseEvent(dom::DOMStringAtom(EventNames::MouseDown), targetNode.get(),
                                            _clickCount, platformMouseEvent, FireMouseOverOut::Yes);

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

    // FIXME: The input element's shadow tree (shouldRefetchEventTarget), with form controls.
    if (!swallowEvent && !mouseEvent.TargetNode()->ParentNode())
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

    // FIXME: Synthetic touch events, pointer lock, hover timers, fake mouse moves, SVG panning and frame set
    // resizing.

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

    MouseEventWithHitTestResults mouseEvent = PrepareMouseEvent(HitTestRequest(hitType), platformMouseEvent);
    if (hitTestResult)
    {
      *hitTestResult = mouseEvent.GetHitTestResult();
    }

    // FIXME: Resizers.
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

    // FIXME: Mouse moves cancelled while capturing a drag (the capturesDragging inability reason), with drag
    // and drop.
    if (!swallowEvent)
    {
      swallowEvent = HandleMouseDraggedEvent(mouseEvent);
    }

    return swallowEvent;
  }

  void EventHandler::InvalidateClick() noexcept
  {
    _clickCount = 0;
    _clickNode = nullptr;
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

    // FIXME: The click capture element, with pointer capture.
    if (!nodeToClick)
    {
      return false;
    }

    // The auxclick event should only be fired for the non-primary pointer buttons.
    // In the case of right button, the auxclick event is dispatched after any contextmenu event.
    //
    // The click event should only be fired for the primary pointer button.
    bool isPrimaryPointerButton = platformMouseEvent.Button() == MouseButton::Left;
    dom::DOMStringAtom eventName(isPrimaryPointerButton ? EventNames::Click : EventNames::AuxClick);

    return !DispatchMouseEvent(eventName, nodeToClick.get(), _clickCount, platformMouseEvent,
                               FireMouseOverOut::No);
  }

  bool EventHandler::HandleMouseReleaseEvent(const PlatformMouseEvent &platformMouseEvent) noexcept
  {
    Ref frame = ShareRef(_frame);
    RefPtr<LocalFrameView> protectedView = ShareRefPtr(frame->View());

    frame->Document()->Selection().SetCaretBlinkingSuspended(false);

    // FIXME: Pointer lock, page overlays, synthetic touch events, user gestures and pan scrolling.

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

    // FIXME: Resizers, once layers resize.

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

    // FIXME: The capturing mouse events element, with mouse capture.
    // If the target node is a non-element, dispatch on the parent.
    _elementUnderMouse = GetElementOrAncestorElementForNode(targetNode);

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
      // FIXME: Whether a cancelled mousedown or mousemove stops dragging (capturesDragging), with dragging.
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
    // selection; form controls that aren't mouse focusable still set the latest focus trigger, once there are
    // form controls.
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
    // FIXME: Noting the frame was scrolled by the user (setFrameWasScrolledByUser), once that's kept.
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

    // FIXME: Escape unlocking the pointer, hiding validation messages and leaving fullscreen; the caps lock
    // state of text fields; pan scrolling.

    // Check for cases where we are too early for events -- possible unmatched key up from pressing return in
    // the location bar.
    RefPtr<dom::Element> element = ShareRefPtr(EventTargetElementForDocument(_frame.Document()));
    if (!element)
    {
      return false;
    }

    // FIXME: User gestures, typing gestures and access keys (handleAccessKey).

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
    Ref<dom::KeyboardEvent> keydown = dom::KeyboardEvent::Create(keyDownEvent);

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
      return keydown->DefaultPrevented() || DefaultKeyboardEventHandler(*keydown);
    }

    // FIXME: Input methods (handleInputMethodKeydown), which run before the keydown is dispatched, and
    // accessibility stopping the propagation.
    (void)dom::EventDispatcher::DispatchToTarget(*keydown, element.get());

    // FIXME: Also when the focused frame changed, once there are subframes; the backward compatibility mode,
    // which dispatches a cancelled keypress anyway.
    bool keydownResult = keydown->DefaultPrevented() || DefaultKeyboardEventHandler(*keydown);
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
    Ref<dom::KeyboardEvent> keypress = dom::KeyboardEvent::Create(keyPressEvent);
    // FIXME: A composing keypress goes to the editor (handleKeyboardEvent), once there are text fields.
    (void)dom::EventDispatcher::DispatchToTarget(*keypress, element.get());

    return keydownResult || keypress->DefaultPrevented() || DefaultKeyboardEventHandler(*keypress);
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

      // FIXME: Escape (close watchers, modal dialogs and popovers) and Backspace (history navigation), once
      // there are dialogs, popovers and navigation.
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

    // FIXME: Spatial navigation (advanceFocus in the arrow's direction), once the host can turn it on.
    ScrollLogicalDirection direction = ScrollLogicalDirection::ScrollBlockDirectionForward;
    switch (focusDirection)
    {
      case dom::FocusDirection::Down: direction = ScrollLogicalDirection::ScrollBlockDirectionForward; break;
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
        // FIXME: Whether the user scrolled the frame (setFrameWasScrolledByUser), with scroll restoration.
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
    // FIXME: Resizing layers, pan scrolling and auto-hiding cursors.

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

        // FIXME: Resizers (isPointInResizeControl), once layers resize.

        // Disabled: during a selection the cursor follows the content beneath it, as Firefox's does, rather
        // than staying an I-beam over boxes and gaps with no text.
        // During selection, use an I-beam regardless of the content beneath the cursor.
        // If a drag may be starting or we're capturing mouse events for a particular node, don't treat this
        // as a selection.
        // FIXME: Dragging (mouseDownMayStartDrag) and capturing mouse events, with drag and drop and pointer
        // capture, if this is enabled again.
        // if (_mousePressed && MouseDownMayStartSelect() && _frame.Document()->Selection().IsCaretOrRange()
        //     && renderer && renderer->Style().UsedUserSelect() != UserSelect::None)
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
    return scrollbar->MouseDown(mouseEvent.Event());
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

    // FIXME: Pointer lock, which sends the wheel to the locked element, once there's pointer lock.

    _isHandlingWheelEvent = true;

    // FIXME: Whether the user scrolled the frame (setFrameWasScrolledByUser), latching and the wheel delta
    // filter, once there are scroll restoration, latching and touchpad gestures.
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

    // FIXME: List boxes, which are scrollable areas of their own, once there are list boxes.
    auto scrollableAreaForBox = [](RenderBox &box) -> ScrollableArea *
    {
      if (box.HasLayer())
      {
        return box.Layer()->GetScrollableArea();
      }
      return nullptr;
    };

    RenderBox *currentEnclosingBox = &startNode->Renderer()->EnclosingBox();
    auto biasedDelta =
      FloatSize(static_cast<float>(wheelEvent.DeltaX()), static_cast<float>(wheelEvent.DeltaY()));

    while (currentEnclosingBox)
    {
      if (auto *boxScrollableArea = scrollableAreaForBox(*currentEnclosingBox))
      {
        // Script-made wheel events don't scroll (scrollViaNonPlatformEvent).
        auto &platformEvent = wheelEvent.UnderlyingPlatformEvent();
        if (!platformEvent)
        {
          return false;
        }
        auto copiedEvent = platformEvent->CopyWithDelta(filteredPlatformDelta);
        bool scrollingWasHandled = ScrollableAreaCanHandleEvent(copiedEvent, *boxScrollableArea)
                                   && HandleWheelEventInScrollableArea(copiedEvent, *boxScrollableArea);
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

  // FIXME: Synthetic clicks (syntheticClickType, which snap to a word boundary), and the editor client's
  // shouldAllowSingleClickToChangeSelection, with touch and the editor.
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

  // FIXME: Dragging (setDragStateSource, mouseDownMayStartDrag), widgets, SVG panning and the scrollbar
  // corner (isPointInScrollbarCorner), with drag and drop, widgets, SVG and resizers.
  bool EventHandler::HandleMousePressEvent(const MouseEventWithHitTestResults &event) noexcept
  {
    Ref<LocalFrame> frame = ShareRef(_frame);

    CancelFakeMouseMoveEvent();

    (void)frame->Document()->UpdateLayoutIgnorePendingStylesheets();

    // If we got the event back, that must mean it wasn't prevented,
    // so it's allowed to start a drag or selection if it wasn't in a scrollbar.
    _mouseDownMayStartSelect = CanMouseDownStartSelect(event) && !event.GetScrollbar();

    _mouseDownWasSingleClickInSelection = false;

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

  // FIXME: Dragging (handleDrag) and list boxes (isRenderListBox), with drag and drop and form controls.
  bool EventHandler::HandleMouseDraggedEvent(const MouseEventWithHitTestResults &event) noexcept
  {
    if (!_mousePressed)
    {
      return false;
    }

    Ref<LocalFrame> frame = ShareRef(_frame);

    RefPtr<dom::Node> targetNode = ShareRefPtr(event.TargetNode());
    if (event.Event().Button() != MouseButton::Left || !targetNode)
    {
      return false;
    }

    auto *renderer = targetNode->Renderer();
    if (!renderer)
    {
      return false;
    }

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
}
