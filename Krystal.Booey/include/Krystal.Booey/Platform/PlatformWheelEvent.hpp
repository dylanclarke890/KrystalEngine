#pragma once

#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Platform/PlatformEvent.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  /// @brief What happened to an event the host sent: whether it reached the DOM, and whether a listener
  /// cancelled it or a default action handled it.
  enum class EventHandling : uint8
  {
    DispatchedToDOM,
    DefaultPrevented,
    DefaultHandled,
  };

  // The ScrollByPixelWheelEvent is a fine-grained event that specifies the precise number of pixels to
  // scroll. It is sent directly by touch pads on macOS, or synthesized when platforms generate line-by-line
  // scrolling events.
  //
  // The ScrollByPageWheelEvent indicates that the wheel event should scroll an entire page.
  // In this case, built in paging behavior is used to page up and down.
  // This yields the same behavior as clicking in a scrollbar track to page up and down.
  enum class PlatformWheelEventGranularity : uint8
  {
    ScrollByPageWheelEvent,
    ScrollByPixelWheelEvent,
  };

  // How many pixels should we scroll per line? Gecko uses the height of the
  // current line, which means scroll distance changes as you go through the
  // page or go to different pages. IE 7 is ~50 px/line, although the value
  // seems to vary slightly by page and zoom level. Since IE 7 has a
  // smoothing algorithm on scrolling, it can get away with slightly larger
  // scroll values without feeling jerky. Here we use 100 px per three lines
  // (the default scroll amount on Windows is three lines per wheel tick).
  constexpr float ScrollbarPixelsPerLine = 100.0f / 3.0f;

  /// @brief A turn of the mouse wheel (or a touchpad scroll) from the host.
  /// @note The deltas are positive when scrolling up or left: the direction the content moves, the opposite
  /// of the DOM's `deltaX` and `deltaY`.
  // FIXME: Gesture and momentum phases, scrolling velocities, precise deltas and inverted directions come
  // with touchpad gestures and momentum scrolling.
  class PlatformWheelEvent : public PlatformEvent
  {
  protected:
    PlatformWheelEventGranularity _granularity {PlatformWheelEventGranularity::ScrollByPixelWheelEvent};
    IntPoint _position;
    IntPoint _globalPosition;
    float _deltaX {0};
    float _deltaY {0};
    float _wheelTicksX {0};
    float _wheelTicksY {0};

  public:
    PlatformWheelEvent() noexcept : PlatformEvent(Type::Wheel)
    {
    }

    /// @param position The point in the view's coordinates.
    /// @param globalPosition The point in the screen's coordinates.
    /// @param deltaX How far to scroll: pixels, or a page's direction when @p granularity is by page.
    /// @param wheelTicksX How far the wheel turned, in notches.
    PlatformWheelEvent(IntPoint position, IntPoint globalPosition, float deltaX, float deltaY,
                       float wheelTicksX, float wheelTicksY, PlatformWheelEventGranularity granularity,
                       EnumSet<PlatformEvent::Modifier> modifiers, MonotonicTime::duration timestamp) noexcept
        : PlatformEvent(Type::Wheel, modifiers, timestamp), _granularity(granularity), _position(position),
          _globalPosition(globalPosition), _deltaX(deltaX), _deltaY(deltaY), _wheelTicksX(wheelTicksX),
          _wheelTicksY(wheelTicksY)
    {
    }

    KRYS_NODISCARD PlatformWheelEvent CopySwappingDirection() const noexcept
    {
      PlatformWheelEvent copy = *this;
      std::swap(copy._deltaX, copy._deltaY);
      std::swap(copy._wheelTicksX, copy._wheelTicksY);
      return copy;
    }

    KRYS_NODISCARD PlatformWheelEvent CopyWithDelta(FloatSize delta) const noexcept
    {
      PlatformWheelEvent copy = *this;
      copy._deltaX = delta.Width();
      copy._deltaY = delta.Height();
      return copy;
    }

    // PlatformWindow coordinates.
    KRYS_NODISCARD const IntPoint &Position() const noexcept
    {
      return _position;
    }
    // Screen coordinates.
    KRYS_NODISCARD const IntPoint &GlobalPosition() const noexcept
    {
      return _globalPosition;
    }

    KRYS_NODISCARD float DeltaX() const noexcept
    {
      return _deltaX;
    }
    KRYS_NODISCARD float DeltaY() const noexcept
    {
      return _deltaY;
    }
    KRYS_NODISCARD FloatSize Delta() const noexcept
    {
      return {_deltaX, _deltaY};
    }

    KRYS_NODISCARD float WheelTicksX() const noexcept
    {
      return _wheelTicksX;
    }
    KRYS_NODISCARD float WheelTicksY() const noexcept
    {
      return _wheelTicksY;
    }

    KRYS_NODISCARD PlatformWheelEventGranularity Granularity() const noexcept
    {
      return _granularity;
    }
  };
}
