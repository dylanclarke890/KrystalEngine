#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/MouseEventWithHitTestResults.hpp"
#include "Krystal.Booey/Rendering/HitTestRequest.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Booey/Style/Values/UI/Cursor.hpp"
#include <algorithm>

namespace krys::boo
{
  namespace
  {
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

  EventHandler::EventHandler(LocalFrame &frame) noexcept : _frame(frame)
  {
  }

  EventHandler::~EventHandler() noexcept = default;

  void EventHandler::Clear() noexcept
  {
    ClearElementUnderMouse();
    _lastElementUnderMouse = nullptr;
    _ancestorsOfLastElementUnderMouse.clear();
    _mouseMoveTargetOverride = nullptr;
    _clickCount = 0;
    _clickNode = nullptr;
    _lastKnownMousePosition = null;
    _lastKnownMouseGlobalPosition = null;
    _mousePressNode = nullptr;
    _mousePressed = false;
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
    // FIXME: Dragging, selection and autoscroll (mouseDownMayStartDrag, mouseDownMayStartSelect,
    // mouseDownMayStartAutoscroll and the mouse down's contents position).
    if (!frame->View())
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
    // FIXME: The focus navigation starting point, with focus on click.

    // FIXME: Scrollbars, subframes and pan scrolling.

    _clickCount = platformMouseEvent.ClickCount();
    _clickNode = ShareRefPtr(mouseEvent.TargetNode());

    // FIXME: Resizers (isPointInResizeControl), once layers resize.

    RefPtr<dom::Node> targetNode = ShareRefPtr(mouseEvent.TargetNode());
    bool swallowEvent = !DispatchMouseEvent(dom::DOMStringAtom(EventNames::MouseDown), targetNode.get(),
                                            _clickCount, platformMouseEvent, FireMouseOverOut::Yes);

    // FIXME: Selection (handleMousePressEvent for the hit test, after refetching the target if it was
    // removed).
    return swallowEvent;
  }

  bool EventHandler::MouseMoved(const PlatformMouseEvent &event) noexcept
  {
    Ref frame = ShareRef(_frame);
    RefPtr<LocalFrameView> protectedView = ShareRefPtr(frame->View());

    // FIXME: Page overlays, and telling the chrome the mouse moved over an element.
    HitTestResult hitTestResult;
    return HandleMouseMoveEvent(event, &hitTestResult);
  }

  bool EventHandler::HandleMouseMoveEvent(const PlatformMouseEvent &platformMouseEvent,
                                          HitTestResult *hitTestResult) noexcept
  {
    Ref frame = ShareRef(_frame);
    RefPtr<LocalFrameView> protectedView = ShareRefPtr(frame->View());

    // FIXME: Synthetic touch events, pointer lock, hover timers, fake mouse moves, SVG panning, frame set
    // resizing and scrollbars.

    SetLastKnownMousePosition(platformMouseEvent.Position(), platformMouseEvent.GlobalPosition());

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

    // FIXME: Resizers, subframes and remote frames.

    if (auto *view = frame->View())
    {
      UpdateCursor(*view, mouseEvent.GetHitTestResult(), platformMouseEvent.ShiftKey());
    }

    RefPtr<dom::Node> targetNode = ShareRefPtr(mouseEvent.TargetNode());
    bool swallowEvent = !DispatchMouseEvent(dom::DOMStringAtom(EventNames::MouseMove), targetNode.get(), 0,
                                            platformMouseEvent, FireMouseOverOut::Yes);

    // FIXME: Dragging (handleMouseDraggedEvent).
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

    // FIXME: Caret blinking, pointer lock, page overlays, synthetic touch events, user gestures and pan
    // scrolling.

    _mousePressed = false;
    SetLastKnownMousePosition(platformMouseEvent.Position(), platformMouseEvent.GlobalPosition());

    // FIXME: SVG panning, frame set resizing, immediate actions and scrollbars.

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

    // FIXME: Resizers, and selection (handleMouseReleaseEvent for the hit test).

    InvalidateClick();

    return swallowMouseUpEvent || swallowClickEvent;
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

    // FIXME: Clicking a frame scrollbar doesn't change the focus, once there are scrollbars.

    // Walk up the tree from the element under the mouse to an element to focus, or focus the viewport if
    // there's none.
    // FIXME: A press inside a selected range in the focused element doesn't move the focus, once there's a
    // selection; form controls that aren't mouse focusable still set the latest focus trigger, once there are
    // form controls.
    if (RefPtr<dom::Element> element = _elementUnderMouse)
    {
      html::FocusAlgorithms::FocusForClick(*element);
    }
    else if (RefPtr<dom::Document> document = ShareRefPtr(_frame.Document()))
    {
      html::FocusAlgorithms::FocusingSteps(*document, nullptr, {.Trigger = dom::FocusTrigger::Click});
    }
    // FIXME: Selecting the contents of a shadow host's focus delegate (findTargetAndUpdateFocusAppearance).
    return true;
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
    // FIXME: Resizing layers, pan scrolling, scrollbars (always the pointer) and auto-hiding cursors.

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
        // FIXME: Image overlay text and editable content (hasEditableStyle), with editing.
        bool editable = false;

        if (UseHandCursor(node, result.IsOverLink(), shiftKey))
        {
          return Cursor(Cursor::Type::Hand);
        }

        // FIXME: Resizers (isPointInResizeControl), and an I-beam during selection, with selection.

        // FIXME: Nodes that can't start a selection (canStartSelection), with editing and dragging.
        if (editable
            || (renderer && renderer->IsRenderText()
                && renderer->Style().UsedUserSelect() != UserSelect::None))
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
}
