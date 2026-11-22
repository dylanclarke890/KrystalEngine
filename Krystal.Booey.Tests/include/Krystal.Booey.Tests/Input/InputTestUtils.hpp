#pragma once

#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Input/EventHandler.hpp"
#include "Krystal.Booey/Input/PlatformKeyboardEvent.hpp"
#include "Krystal.Booey/Input/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Page/Frame.hpp"
#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include <string>

namespace krys::boo::tests
{
  /// @brief A mouse event at @p position: a press is one click, and a move has no button down (a hover).
  KRYS_NODISCARD inline PlatformMouseEvent MouseEventAt(PlatformEvent::Type type, PointI position,
                                                        MouseButton button = MouseButton::Left) noexcept
  {
    PointD point(position.X(), position.Y());
    int32 clickCount = type == PlatformEvent::Type::MousePressed ? 1 : 0;
    return PlatformMouseEvent(point, point,
                              type == PlatformEvent::Type::MouseMoved ? MouseButton::None : button, type,
                              clickCount, {}, MonotonicTime::Now());
  }

  /// @brief A mouse event at @p position with the left button down, moves included (a drag): a press is one
  /// click.
  KRYS_NODISCARD inline PlatformMouseEvent
    DragMouseEvent(PlatformEvent::Type type, PointI position,
                   EnumSet<PlatformEvent::Modifier> modifiers = {}) noexcept
  {
    PointD point(position.X(), position.Y());
    int32 clickCount = type == PlatformEvent::Type::MousePressed ? 1 : 0;
    return PlatformMouseEvent(point, point, MouseButton::Left, type, clickCount, modifiers,
                              MonotonicTime::Now());
  }

  /// @brief Clicks the left mouse button at (@p x, @p y) in @p frame.
  inline void Click(Frame &frame, double x, double y) noexcept
  {
    auto &handler = frame.GetEventHandler();
    (void)handler.HandleMousePressEvent(PlatformMouseEvent(PointD(x, y), PointD(x, y), MouseButton::Left,
                                                           PlatformEvent::Type::MousePressed, 1, {},
                                                           MonotonicTime::Now()));
    (void)handler.HandleMouseReleaseEvent(PlatformMouseEvent(PointD(x, y), PointD(x, y), MouseButton::Left,
                                                             PlatformEvent::Type::MouseReleased, 1, {},
                                                             MonotonicTime::Now()));
  }

  /// @brief Presses @p key in @p frame.
  /// @return Whether the page handled the key.
  inline bool PressKey(Frame &frame, utf8_string key) noexcept
  {
    return frame.GetEventHandler().KeyEvent(PlatformKeyboardEvent(PlatformEvent::Type::KeyDown, u8"", u8"",
                                                                  krys::move(key), u8"", u8"", 0, false,
                                                                  false, false, {}, MonotonicTime::Now()));
  }

  /// @brief @p target's id, or its local name if it has none; "document" for another node, "null" for no
  /// node.
  KRYS_NODISCARD inline std::string TargetName(const EventTarget *target)
  {
    auto *node = DynamicDowncast<Node>(target);
    auto *element = node ? DynamicDowncast<Element>(*node) : nullptr;
    if (!element)
    {
      return node ? "document" : "null";
    }
    auto id = element->GetAttribute(u8"id").value_or(DOMString());
    auto name = id.empty() ? element->LocalName().View() : DOMStringView(id);
    return std::string(name.begin(), name.end());
  }
}
