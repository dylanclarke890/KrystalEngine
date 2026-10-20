#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Graphics/DoublePoint.hpp"
#include "Krystal.Booey/Platform/Cursor.hpp"
#include "Krystal.Booey/Platform/PlatformKeyboardEvent.hpp"
#include "Krystal.Booey/Platform/PlatformMouseEvent.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Element;
  class Node;
}

namespace krys::boo
{
  class HitTestRequest;
  class HitTestResult;
  class LocalFrame;
  class LocalFrameView;
  class MouseEventWithHitTestResults;

  /// @brief Turns the host's mouse and keyboard input into DOM events for a frame: `mousemove`, `mousedown`
  /// and `mouseup` at the element under the mouse; `mouseover`, `mouseout`, `mouseenter` and `mouseleave` as
  /// that element changes; `click`, `auxclick` and `dblclick`; the hover and active state; the cursor to
  /// show; and `keydown`, `keypress` and `keyup` at the focused element.
  // FIXME: The rest of the event handler: the keyboard's default actions, wheel scrolling, drag and drop,
  // selection, context menus, scrollbars, resizers, subframes, autoscroll, pointer lock, touch and pointer
  // events.
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

  private:
    LocalFrame &_frame;

    RefPtr<dom::Node> _mousePressNode;
    bool _mousePressed {false};

    RefPtr<dom::Element> _elementUnderMouse;
    RefPtr<dom::Element> _lastElementUnderMouse;
    List<RefPtr<dom::Element>> _ancestorsOfLastElementUnderMouse;
    RefPtr<dom::Element> _mouseMoveTargetOverride;

    Cursor _currentMouseCursor;

    RefPtr<dom::Node> _clickNode;
    int32 _clickCount {0};

    // Same coordinates as PlatformMouseEvent::Position().
    Maybe<DoublePoint> _lastKnownMousePosition;
    Maybe<DoublePoint> _lastKnownMouseGlobalPosition;
    MonotonicTime::duration _mouseDownTimestamp {};
    PlatformMouseEvent _lastPlatformMouseEvent;

  public:
    explicit EventHandler(LocalFrame &frame) noexcept;
    ~EventHandler() noexcept;

    /// @brief Forgets the mouse's state, as when the frame's document is replaced.
    void Clear() noexcept;

    /// @brief Forgets @p nodeToBeRemoved and its descendants as the click node and the element under the
    /// mouse.
    void NodeWillBeRemoved(dom::Node &nodeToBeRemoved) noexcept;

    KRYS_NODISCARD bool MousePressed() const noexcept
    {
      return _mousePressed;
    }

    KRYS_NODISCARD Maybe<DoublePoint> LastKnownMousePosition() const noexcept
    {
      return _lastKnownMousePosition;
    }

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

    /// @brief Chooses the cursor again for the last known mouse position, as after the page changes under a
    /// still mouse.
    void UpdateCursor() noexcept;

    /// @brief Handles a key event: a key up becomes `keyup` and a char `keypress`; a raw key down becomes
    /// `keydown`, and a combined key down `keydown` then, unless it's cancelled and if the key types text,
    /// `keypress`. They go to the focused element, or the body. A key press makes the focused element focus
    /// visible.
    /// @return Whether the event was cancelled.
    bool KeyEvent(const PlatformKeyboardEvent &keyEvent) noexcept;

  private:
    bool InternalKeyEvent(const PlatformKeyboardEvent &initialKeyEvent) noexcept;

    KRYS_NODISCARD MouseEventWithHitTestResults
      PrepareMouseEvent(const HitTestRequest &request, const PlatformMouseEvent &mouseEvent) noexcept;

    void SetLastKnownMousePosition(DoublePoint position, DoublePoint globalPosition) noexcept;
    void InvalidateClick() noexcept;
    bool SwallowAnyClickEvent(const PlatformMouseEvent &platformMouseEvent,
                              const MouseEventWithHitTestResults &mouseEvent,
                              IgnoreAncestorNodesForClickEvent ignoreAncestorNodesForClickEvent) noexcept;

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

    // FIXME: Selection: handleMousePressEvent and handleMouseReleaseEvent for a hit test, which select text.
  };
}
