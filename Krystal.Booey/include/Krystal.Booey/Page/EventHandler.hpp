#pragma once

#include "Krystal.Booey/Core/ScrollTypes.hpp"
#include "Krystal.Booey/DOM/FocusOptions.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Editing/TextGranularity.hpp"
#include "Krystal.Booey/Graphics/DoublePoint.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Graphics/LayoutPoint.hpp"
#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/Page/DragActions.hpp"
#include "Krystal.Booey/Platform/Cursor.hpp"
#include "Krystal.Booey/Platform/PlatformKeyboardEvent.hpp"
#include "Krystal.Booey/Platform/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Platform/PlatformTouchEvent.hpp"
#include "Krystal.Booey/Platform/PlatformWheelEvent.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Booey/Rendering/HitTestRequest.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class DataTransfer;
  class Element;
  class Event;
  class KeyboardEvent;
  class WheelEvent;
  class Node;
  enum class TextEventInputType : uint8;
}

namespace krys::boo
{
  class AutoscrollController;
  struct DragState;
  class FloatQuad;
  class LocalFrame;
  class LocalFrameView;
  class Pasteboard;
  class RenderLayer;
  class MouseEventWithHitTestResults;
  class RenderBox;
  class VisiblePosition;
  class VisibleSelection;

  enum AppendTrailingWhitespace : bool
  {
    ShouldAppendTrailingWhitespace,
    DontAppendTrailingWhitespace
  };
  class ScrollableArea;
  class Scrollbar;

  // The link drag hysteresis is much larger than the others because there
  // needs to be enough space to cancel the link press without starting a link drag,
  // and because dragging links is rare.
  constexpr int32 LinkDragHysteresis = 40;
  constexpr int32 ImageDragHysteresis = 5;
  constexpr int32 TextDragHysteresis = 3;
  constexpr int32 ColorDragHystersis = 3;
  constexpr int32 GeneralDragHysteresis = 3;

  enum CheckDragHysteresis : bool
  {
    ShouldCheckDragHysteresis,
    DontCheckDragHysteresis
  };

  /// @brief Turns the host's mouse and keyboard input into DOM events for a frame: `mousemove`, `mousedown`
  /// and `mouseup` at the element under the mouse; `mouseover`, `mouseout`, `mouseenter` and `mouseleave` as
  /// that element changes; `click`, `auxclick` and `dblclick`; the hover and active state; the cursor to
  /// show; the scrollbars under the mouse; `wheel`, which scrolls; and `keydown`, `keypress` and `keyup` at
  /// the focused element, with Tab and Shift+Tab moving the focus and the arrows, Page Up and Page Down,
  /// Home, End and Space scrolling; selecting with the mouse: a press places the selection, a drag extends it
  /// (scrolling as it reaches an edge), double and triple presses select words and paragraphs, and Shift
  /// extends it; and drag and drop: a press on something draggable that moves far enough starts a drag
  /// (`dragstart`, then `drag` as it moves and `dragend` when it ends, at the source), and a drag over the
  /// page gets `dragenter`, `dragover`, `dragleave` and `drop` at its targets.
  // FIXME: The rest of the event handler: the keyboard's other default actions (selection by the keyboard is
  // with the keyboard and the DOM's Selection), context menus, resizers, subframes, pan scrolling, pointer
  // lock and touch events.

  // Pointer events from the mouse are dispatched by the elements as they get mouse events
  // (`Element::DispatchMouseEvent`).
  class EventHandler
  {
    KRYS_NON_COPY_MOVABLE(EventHandler);

    enum class FireMouseOverOut : bool
    {
      No,
      Yes
    };

    enum class IgnoreAncestorNodesForClickEvent : bool
    {
      No,
      Yes
    };

    enum class SetOrClearLastScrollbar : bool
    {
      Clear,
      Set
    };

    enum SelectionInitiationState : uint8
    {
      HaveNotStartedSelection,
      PlacedCaret,
      ExtendedSelection
    };

    /// @brief Whether the press may go on to drag or select, or why it can't.
    class CapturesDragging
    {
    public:
      enum class InabilityReason : uint8
      {
        MousePressIsCancelled,
        MouseMoveIsCancelled,
        Unknown,
      };

    private:
      Maybe<InabilityReason> _inabilityReason {InabilityReason::Unknown};

    public:
      CapturesDragging(bool capturesDragging) noexcept
      {
        *this = capturesDragging;
      }

      CapturesDragging(InabilityReason inabilityReason) noexcept : _inabilityReason(inabilityReason)
      {
      }

      CapturesDragging &operator=(bool capturesDragging) noexcept
      {
        if (capturesDragging)
        {
          _inabilityReason = null;
        }
        else
        {
          _inabilityReason = InabilityReason::Unknown;
        }
        return *this;
      }

      CapturesDragging &operator=(InabilityReason inabilityReason) noexcept
      {
        _inabilityReason = inabilityReason;
        return *this;
      }

      explicit operator bool() const noexcept
      {
        return !_inabilityReason;
      }
      KRYS_NODISCARD InabilityReason GetInabilityReason() const noexcept
      {
        return *_inabilityReason;
      }
    };

  private:
    LocalFrame &_frame;

    RefPtr<dom::Node> _mousePressNode;
    bool _mousePressed {false};
    bool _mouseDownMayStartSelect {false};
    bool _mouseDownMayStartAutoscroll {false};
    bool _mouseDownWasSingleClickInSelection {false};
    SelectionInitiationState _selectionInitiationState {HaveNotStartedSelection};
    LayoutPoint _mouseDownContentsPosition;
    IntPoint _dragStartPosition;
    Maybe<dom::SimpleRange> _dragStartSelection;
    RefPtr<dom::Element> _dragTarget;
    bool _mouseDownMayStartDrag {false};
    bool _dragMayStartSelectionInstead {false};
    bool _shouldOnlyFireDragOverEvent {false};
    bool _didStartDrag {false};
    CapturesDragging _capturesDragging {false};
    PlatformMouseEvent _mouseDownEvent;
    UniquePtr<AutoscrollController> _autoscrollController;

    RefPtr<dom::Element> _elementUnderMouse;
    RefPtr<dom::Element> _lastElementUnderMouse;
    List<RefPtr<dom::Element>> _ancestorsOfLastElementUnderMouse;
    RefPtr<dom::Element> _mouseMoveTargetOverride;
    RefPtr<dom::Element> _capturingMouseEventsElement;

    RefPtr<Scrollbar> _lastScrollbarUnderMouse;

    // The layer whose resizer the user is dragging, and the mouse's offset from its resize corner when the
    // drag began (in the layer's coordinates).
    WeakPtr<RenderLayer> _resizeLayer;
    LayoutSize _offsetFromResizeCorner;

    Cursor _currentMouseCursor;

    RefPtr<dom::Node> _clickNode;
    /// @brief The element capturing the mouse when the click began, which gets the click.
    RefPtr<dom::Element> _clickCaptureElement;
    int32 _clickCount {0};

    // Same coordinates as PlatformMouseEvent::Position().
    Maybe<DoublePoint> _lastKnownMousePosition;
    Maybe<DoublePoint> _lastKnownMouseGlobalPosition;
    MonotonicTime::duration _mouseDownTimestamp {};
    /// @brief How long the longest mouse move took to handle (real time), which picks the fake mouse move
    /// interval.
    MonotonicTime::duration _maxMouseMovedDuration {};
    Timer _fakeMouseMoveEventTimer;
    bool _hasScheduledCursorUpdate {false};
    bool _isHandlingWheelEvent {false};
    PlatformMouseEvent _lastPlatformMouseEvent;

    // The element each touch began on, by its id plus one (so no key is 0).
    HashMap<uint32, RefPtr<dom::EventTarget>> _originatingTouchPointTargets;
    RefPtr<dom::Document> _originatingTouchPointDocument;
    uint32 _originatingTouchPointTargetKey {0};
    bool _touchPressed {false};
    /// @brief The pointers of the touches that ended, forgotten at the next touch event.
    List<PointerID> _endedTouchPointerIds;

  public:
    explicit EventHandler(LocalFrame &frame) noexcept;
    ~EventHandler() noexcept;

    /// @brief Forgets the mouse's state, as when the frame's document is replaced.
    void Clear() noexcept;

    /// @brief Forgets @p nodeToBeRemoved and its descendants as the click node and the element under the
    /// mouse.
    void NodeWillBeRemoved(dom::Node &nodeToBeRemoved) noexcept;

    /// @brief Called when the layer whose resizer is being dragged is destroyed.
    void ResizeLayerDestroyed() noexcept;

    /// @brief Sends mouse events to @p element, wherever the mouse is, until it's cleared.
    void SetCapturingMouseEventsElement(RefPtr<dom::Element> &&element) noexcept;

    /// @brief Called when @p element captures the mouse's pointer (`setPointerCapture()`), or null when the
    /// capture is released: mouse events, and the click, go to it.
    void PointerCaptureElementDidChange(dom::Element *element) noexcept;

    /// @brief Hit tests at @p point (in the frame's contents), after bringing the layout up to date.
    KRYS_NODISCARD HitTestResult HitTestResultAtPoint(const LayoutPoint &point,
                                                      EnumSet<HitTestRequest::Type> hitType) const noexcept;

    KRYS_NODISCARD bool MousePressed() const noexcept
    {
      return _mousePressed;
    }

    KRYS_NODISCARD Maybe<DoublePoint> LastKnownMousePosition() const noexcept
    {
      return _lastKnownMousePosition;
    }

    KRYS_NODISCARD dom::Node *MousePressNode() const noexcept
    {
      return _mousePressNode.get();
    }

    /// @brief What a drop target said of a drag over it: whether it accepts it, and the operations it chose.
    struct DragTargetResponse
    {
      bool Accept {false};
      Maybe<EnumSet<DragOperation>> OperationMask;
    };

    /// @brief How long a press on the selection must be held before moving drags it rather than selecting.
    static constexpr MonotonicTime::duration TextDragDelay {};

    /// @brief A drag moved over the frame: `dragenter` and `dragleave` as its target changes, else
    /// `dragover`, each after `drag` at the source.
    DragTargetResponse UpdateDragAndDrop(const PlatformMouseEvent &event,
                                         const Func<UniquePtr<Pasteboard>()> &makePasteboard,
                                         EnumSet<DragOperation> sourceOperationMask,
                                         bool draggingFiles) noexcept;
    /// @brief A drag left the frame: `dragleave` at its target.
    void CancelDragAndDrop(const PlatformMouseEvent &event, UniquePtr<Pasteboard> &&pasteboard,
                           EnumSet<DragOperation> sourceOperationMask, bool draggingFiles) noexcept;
    /// @brief A drag dropped on the frame: `drop` at its target.
    /// @return Whether the page cancelled it, doing the drop itself.
    bool PerformDragAndDrop(const PlatformMouseEvent &event, UniquePtr<Pasteboard> &&pasteboard,
                            EnumSet<DragOperation> sourceOperationMask, bool draggingFiles) noexcept;
    void UpdateDragStateAfterEditDragIfNeeded(dom::Element &rootEditableElement) noexcept;
    /// @brief The element being dragged, if a drag started in a page.
    KRYS_NODISCARD static dom::Element *DraggedElement() noexcept;

    /// @brief Whether a press @p event may start a drag.
    KRYS_NODISCARD bool EventMayStartDrag(const PlatformMouseEvent &event) const noexcept;

    void DidStartDrag() noexcept;
    void DragCancelled() noexcept;
    /// @brief The drag that started in the frame ended at @p event, with @p dragOperationMask done: `dragend`
    /// at the source.
    void DragSourceEndedAt(const PlatformMouseEvent &event, EnumSet<DragOperation> dragOperationMask,
                           MayExtendDragSession mayExtendDragSession = MayExtendDragSession::No) noexcept;

    /// @brief Whether a drag of an image may be dropped in the frame as a file.
    // FIXME: The origin the image's data is restricted to, once there are security origins.
    KRYS_NODISCARD bool CanDropCurrentlyDraggedImageAsFile() const noexcept;

    /// @brief Extends the selection to the last known mouse position, as dragging (and autoscrolling) does.
    void UpdateSelectionForMouseDrag() noexcept;
    KRYS_NODISCARD bool ShouldUpdateAutoscroll() const noexcept;
    KRYS_NODISCARD IntPoint TargetPositionInWindowForSelectionAutoscroll() const noexcept;
    KRYS_NODISCARD RenderBox *AutoscrollRenderer() const noexcept;

    void SetMouseDownMayStartAutoscroll() noexcept
    {
      _mouseDownMayStartAutoscroll = true;
    }
    KRYS_NODISCARD bool AutoscrollInProgress() const noexcept;
    void StopAutoscrollTimer(bool rendererIsBeingDestroyed = false) noexcept;

    KRYS_NODISCARD static DragState &GetDragState() noexcept;

    /// @brief The element the mouse is over, as of the last mouse event.
    KRYS_NODISCARD dom::Element *ElementUnderMouse() const noexcept
    {
      return _elementUnderMouse.get();
    }

    /// @brief The cursor to show, as chosen by the last mouse move or cursor update.
    KRYS_NODISCARD Cursor CurrentMouseCursor() const noexcept
    {
      return _currentMouseCursor;
    }

    /// @brief Handles a press of a mouse button: `mousedown` at the element under the mouse, which becomes
    /// `:active` with its ancestors.
    /// @return Whether the event was swallowed (cancelled).
    bool HandleMousePressEvent(const PlatformMouseEvent &platformMouseEvent) noexcept;

    /// @brief Handles a move of the mouse: `mouseover`, `mouseout`, `mouseenter` and `mouseleave` if the
    /// element under it changed, then `mousemove`; the hover state and the cursor follow.
    /// @return Whether the event was swallowed (cancelled).
    bool MouseMoved(const PlatformMouseEvent &platformMouseEvent) noexcept;

    /// @param hitTestResult Set to what the mouse is over, if given.
    bool HandleMouseMoveEvent(const PlatformMouseEvent &platformMouseEvent,
                              HitTestResult *hitTestResult = nullptr) noexcept;

    /// @brief Handles a release of a mouse button: `mouseup` at the element under the mouse, then `click` (or
    /// `auxclick`) at the nearest common ancestor of the pressed and released elements.
    /// @return Whether the event was swallowed (cancelled).
    bool HandleMouseReleaseEvent(const PlatformMouseEvent &platformMouseEvent) noexcept;

    /// @brief Handles a change to the touches on the screen: `touchstart`, `touchmove`, `touchend` and
    /// `touchcancel` at the elements the touches began on, each preceded by the touches' pointer events.
    /// @return Whether a touch event was cancelled or handled.
    bool HandleTouchEvent(const PlatformTouchEvent &event) noexcept;

    /// @brief Sends `contextmenu` to the element under @p event, as the host does after a press of the right
    /// button or a long press: the press is over, so no drag starts; with the Mac's editing behaviour, the
    /// word (or link) under it is selected first. Not while the pointer is locked, nor over a scrollbar.
    /// @return Whether the event was cancelled.
    // FIXME: The keyboard's context menu (sendContextMenuEventForKey), once the host reports the menu key.
    bool SendContextMenuEvent(const PlatformMouseEvent &event) noexcept;

    /// @brief Chooses the cursor again for the last known mouse position, as after the page changes under a
    /// still mouse.
    void UpdateCursor() noexcept;
    /// @brief Asks for the cursor to be chosen again in the next rendering update, as after a style change.
    void ScheduleCursorUpdate() noexcept;
    /// @brief Chooses the cursor again if an update was scheduled (the rendering update's cursor step).
    void UpdateCursorIfNeeded() noexcept;

    /// @brief Moves the mouse where it is, soon (after 100 milliseconds, or 250 if moves are slow to handle),
    /// so the hover state, the element under the mouse and the cursor follow content that moved under it, as
    /// after a scroll.
    void DispatchFakeMouseMoveEventSoon() noexcept;
    /// @brief As `DispatchFakeMouseMoveEventSoon`, if the mouse is in @p quad (in absolute coordinates).
    void DispatchFakeMouseMoveEventSoonInQuad(const FloatQuad &quad) noexcept;
    void CancelFakeMouseMoveEvent() noexcept;

    /// @brief Handles a key event: a key up becomes `keyup` and a char `keypress`; a raw key down becomes
    /// `keydown`, and a combined key down `keydown` then, unless it's cancelled and if the key types text,
    /// `keypress`. They go to the focused element, or the body. A key press makes the focused element focus
    /// visible.
    /// @return Whether the event was cancelled.
    bool KeyEvent(const PlatformKeyboardEvent &keyEvent) noexcept;

    /// @brief Handles a turn of the mouse wheel: `wheel` at the element under the mouse, then, unless it's
    /// cancelled, a scroll of the nearest box that can scroll that way, or else the view.
    /// @return Whether the event was cancelled or scrolled something.
    // FIXME: Latching, the delta filter, scroll snapping, gesture states and asynchronous scrolling come with
    // touchpad gestures, scroll snapping and a scrolling thread.
    bool HandleWheelEvent(const PlatformWheelEvent &event) noexcept;
    /// @brief The default action of a `wheel` event at @p startNode: scrolls the nearest enclosing box that
    /// can scroll by the event's deltas, passing over boxes at their end unless `overscroll-behavior` stops
    /// it.
    /// @return Whether it scrolled (or `overscroll-behavior` stopped the scroll there).
    bool DefaultWheelEventHandler(dom::Node *startNode, dom::WheelEvent &wheelEvent) noexcept;
    /// @brief Scrolls for a touch's pan, or the fling after it, by @p event's delta from where the touch
    /// began (its position): the nearest box under it that can scroll that way, passing over boxes at their
    /// end unless `overscroll-behavior` stops it, then the view, as the wheel scrolls; unlike the wheel's, no
    /// event fires.
    /// @return Whether it scrolled (or `overscroll-behavior` stopped the scroll there).
    bool HandleTouchScroll(const PlatformWheelEvent &event) noexcept;
    KRYS_NODISCARD bool IsHandlingWheelEvent() const noexcept
    {
      return _isHandlingWheelEvent;
    }

    /// @brief Queues a `scroll` event at the document, after the view scrolled.
    void ScheduleScrollEvent() noexcept;

    /// @brief Notes that the user scrolled the frame, which scroll restoration then leaves as it is.
    void SetFrameWasScrolledByUser() noexcept;

    /// @brief The default actions of a keyboard event that wasn't cancelled: Tab and Shift+Tab move the
    /// focus; the arrows scroll by a line, Page Up and Page Down by a page, and Home and End to the start or
    /// end, and a `keypress` of Space (Shift+Space) a page down (up).
    /// @return Whether the event was handled.
    bool DefaultKeyboardEventHandler(dom::KeyboardEvent &event) noexcept;

    /// @brief Fires `textInput` for text the user typed (or pasted, or dropped) at @p underlyingEvent's
    /// target, or else the focused element, and has the editor insert it unless a listener cancels it.
    /// @return Whether the editor handled it.
    bool HandleTextInputEvent(utf8_stringview text, dom::Event *underlyingEvent,
                              dom::TextEventInputType inputType) noexcept;

    /// @brief Scrolls the box of @p startingNode (the focused element, or else the last pressed node, if
    /// null), or else its nearest containing block that scrolls, by a step of @p granularity.
    /// @return Whether it scrolled.
    bool ScrollOverflow(ScrollDirection direction, ScrollGranularity granularity,
                        dom::Node *startingNode = nullptr) noexcept;
    bool LogicalScrollOverflow(ScrollLogicalDirection direction, ScrollGranularity granularity,
                               dom::Node *startingNode = nullptr) noexcept;
    /// @brief As `ScrollOverflow`, or else scrolls the view.
    // FIXME: Then the parent frame, once there are subframes.
    bool ScrollRecursively(ScrollDirection direction, ScrollGranularity granularity,
                           dom::Node *startingNode = nullptr) noexcept;
    bool LogicalScrollRecursively(ScrollLogicalDirection direction, ScrollGranularity granularity,
                                  dom::Node *startingNode = nullptr) noexcept;

  private:
    bool InternalKeyEvent(const PlatformKeyboardEvent &initialKeyEvent) noexcept;
    bool DefaultTabEventHandler(dom::KeyboardEvent &event) noexcept;
    bool DefaultKeyboardScrollEventHandler(dom::KeyboardEvent &event, ScrollLogicalDirection direction,
                                           ScrollGranularity granularity) noexcept;
    bool DefaultPageUpDownEventHandler(dom::KeyboardEvent &event) noexcept;
    bool DefaultHomeEndEventHandler(dom::KeyboardEvent &event) noexcept;
    bool DefaultSpaceEventHandler(dom::KeyboardEvent &event) noexcept;
    bool DefaultArrowEventHandler(dom::FocusDirection focusDirection, dom::KeyboardEvent &event) noexcept;

    KRYS_NODISCARD MouseEventWithHitTestResults
      PrepareMouseEvent(const HitTestRequest &request, const PlatformMouseEvent &mouseEvent) noexcept;

#pragma region Drag and drop

    void SetDragStateSource(dom::Element *element) const noexcept;
    KRYS_NODISCARD EnumSet<DragSourceAction> UpdateDragSourceActionsAllowed() const noexcept;
    KRYS_NODISCARD bool ShouldAllowMouseDownToStartDrag() const noexcept;

    bool DispatchDragEvent(dom::DOMStringAtom eventType, dom::Element &target,
                           const PlatformMouseEvent &event, dom::DataTransfer &dataTransfer) noexcept;
    DragTargetResponse DispatchDragEnterOrDragOverEvent(dom::DOMStringAtom eventType, dom::Element &target,
                                                        const PlatformMouseEvent &event,
                                                        UniquePtr<Pasteboard> &&pasteboard,
                                                        EnumSet<DragOperation> sourceOperationMask,
                                                        bool draggingFiles) noexcept;
    bool HandleDrag(const MouseEventWithHitTestResults &event,
                    CheckDragHysteresis checkDragHysteresis) noexcept;
    void ClearDragState() noexcept;

    KRYS_NODISCARD static bool ShouldDispatchEventsToDragSourceElement() noexcept;
    void DispatchEventToDragSourceElement(dom::DOMStringAtom eventType,
                                          const PlatformMouseEvent &event) noexcept;
    bool DispatchDragStartEventOnSourceElement(dom::DataTransfer &dataTransfer) noexcept;

    KRYS_NODISCARD bool DragHysteresisExceeded(const FloatPoint &dragViewportLocation) const noexcept;
    KRYS_NODISCARD bool MouseMovementExceedsThreshold(const FloatPoint &viewportLocation,
                                                      int32 pointsThreshold) const noexcept;
    static void InvalidateDataTransfer() noexcept;

#pragma endregion

    void SetLastKnownMousePosition(DoublePoint position, DoublePoint globalPosition) noexcept;
    void InvalidateClick() noexcept;
    bool SwallowAnyClickEvent(const PlatformMouseEvent &platformMouseEvent,
                              const MouseEventWithHitTestResults &mouseEvent,
                              IgnoreAncestorNodesForClickEvent ignoreAncestorNodesForClickEvent) noexcept;

    /// @return Whether the click was dispatched without being cancelled.
    bool DispatchAnyClickEvent(dom::DOMStringAtom eventType, dom::Node *clickNode, int32 clickCount,
                               const PlatformMouseEvent &platformMouseEvent) noexcept;
    /// @return Whether the event was dispatched without being cancelled.
    bool DispatchMouseEvent(dom::DOMStringAtom eventType, dom::Node *targetNode, int32 clickCount,
                            const PlatformMouseEvent &platformMouseEvent,
                            FireMouseOverOut fireMouseOverOut) noexcept;
    void UpdateMouseEventTargetNode(dom::DOMStringAtom eventType, dom::Node *targetNode,
                                    const PlatformMouseEvent &platformMouseEvent,
                                    FireMouseOverOut fireMouseOverOut) noexcept;
    void ClearElementUnderMouse() noexcept;
    KRYS_NODISCARD bool IsElementAnAncestorOfLastElementUnderMouse(dom::Element *element) const noexcept;

    void UpdateCursor(LocalFrameView &view, const HitTestResult &result, bool shiftKey) noexcept;
    KRYS_NODISCARD Maybe<Cursor> SelectCursor(const HitTestResult &result, bool shiftKey) const noexcept;
    KRYS_NODISCARD bool UseHandCursor(dom::Node *node, bool isOverLink, bool shiftKey) const noexcept;

    void FakeMouseMoveEventTimerFired() noexcept;

    bool HandleWheelEventInternal(const PlatformWheelEvent &event, EnumSet<EventHandling> &handling) noexcept;
    bool ProcessWheelEventForScrolling(const PlatformWheelEvent &event) noexcept;
    /// @param biasedDelta The delta whose axes `overscroll-behavior` may stop from passing to the boxes
    /// further out.
    bool HandleWheelEventInAppropriateEnclosingBox(dom::Node *startNode, const PlatformWheelEvent &event,
                                                   FloatSize biasedDelta) noexcept;
    bool HandleWheelEventInAppropriateEnclosingBox(dom::Node *startNode, const dom::WheelEvent &wheelEvent,
                                                   const FloatSize &filteredPlatformDelta) noexcept;
    KRYS_NODISCARD static bool ScrollableAreaCanHandleEvent(const PlatformWheelEvent &wheelEvent,
                                                            ScrollableArea &scrollableArea) noexcept;
    bool HandleWheelEventInScrollableArea(const PlatformWheelEvent &wheelEvent,
                                          ScrollableArea &scrollableArea) noexcept;

    bool PassMousePressEventToScrollbar(MouseEventWithHitTestResults &mouseEvent,
                                        Scrollbar *scrollbar) noexcept;
    /// @brief Sends `MouseExited` to the last scrollbar under the mouse if @p scrollbar is a different one,
    /// and `MouseEntered` to @p scrollbar if it's set as the new one.
    void UpdateLastScrollbarUnderMouse(Scrollbar *scrollbar, SetOrClearLastScrollbar setOrClear) noexcept;
    KRYS_NODISCARD bool IsInsideScrollbar(const IntPoint &windowPoint) const noexcept;

#pragma region Selection

    bool HandleMousePressEvent(const MouseEventWithHitTestResults &event) noexcept;
    bool HandleMousePressEventSingleClick(const MouseEventWithHitTestResults &event) noexcept;
    bool HandleMousePressEventDoubleClick(const MouseEventWithHitTestResults &event) noexcept;
    bool HandleMousePressEventTripleClick(const MouseEventWithHitTestResults &event) noexcept;
    KRYS_NODISCARD bool CanMouseDownStartSelect(const MouseEventWithHitTestResults &event) const noexcept;
    KRYS_NODISCARD bool MouseDownMayStartSelect() const noexcept;
    bool ExpandAndUpdateSelectionForMouseDownIfNeeded(dom::Node &targetNode,
                                                      const VisibleSelection &selection,
                                                      TextGranularity granularity) noexcept;
    bool UpdateSelectionForMouseDownDispatchingSelectStart(dom::Node *targetNode,
                                                           const VisibleSelection &selection,
                                                           TextGranularity granularity) noexcept;
    void SelectClosestWordFromHitTestResult(const HitTestResult &result,
                                            AppendTrailingWhitespace appendTrailingWhitespace) noexcept;
    void SelectClosestContextualWordFromHitTestResult(
      const HitTestResult &result, AppendTrailingWhitespace appendTrailingWhitespace) noexcept;
    void SelectClosestContextualWordOrLinkFromHitTestResult(
      const HitTestResult &result, AppendTrailingWhitespace appendTrailingWhitespace) noexcept;
    KRYS_NODISCARD VisiblePosition SelectionExtentRespectingEditingBoundary(const VisibleSelection &selection,
                                                                            const LayoutPoint &localPoint,
                                                                            dom::Node *targetNode) noexcept;
    bool
      HandleMouseDraggedEvent(const MouseEventWithHitTestResults &event,
                              CheckDragHysteresis checkDragHysteresis = ShouldCheckDragHysteresis) noexcept;
    void UpdateSelectionForMouseDrag(const HitTestResult &hitTestResult) noexcept;
    bool HandleMouseReleaseEvent(const MouseEventWithHitTestResults &event) noexcept;

#pragma endregion
  };
}
