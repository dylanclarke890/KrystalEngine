#include "Krystal.Booey/Events/Pointer/WheelEvent.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"

namespace krys::boo
{
  namespace
  {
    /// @brief Pages for a host that scrolls by the page, pixels otherwise; the host never gives lines.
    KRYS_NODISCARD uint32 WheelDeltaMode(const PlatformWheelEvent &event) noexcept
    {
      if (event.Granularity() == PlatformWheelEventGranularity::ScrollByPageWheelEvent)
      {
        return WheelEvent::DOM_DELTA_PAGE;
      }
      return WheelEvent::DOM_DELTA_PIXEL;
    }

    /// @brief How a trusted `wheel` event is fired: it bubbles and is composed.
    /// @see https://w3c.github.io/pointerevents/#events-wheel-types
    KRYS_NODISCARD UIEventInit WheelEventInitFor(WheelEvent::IsCancelable isCancelable) noexcept
    {
      return {
        {.Bubbles = true, .Cancelable = isCancelable == WheelEvent::IsCancelable::Yes, .Composed = true}};
    }

    KRYS_NODISCARD PointD WheelPoint(const PointI &point) noexcept
    {
      return {static_cast<double>(point.X()), static_cast<double>(point.Y())};
    }
  }

  WheelEvent::WheelEvent(DOMStringAtom type, const WheelEventInit &initializer) noexcept
      : MouseEvent(krys::move(type), initializer), _deltaX(initializer.DeltaX), _deltaY(initializer.DeltaY),
        _deltaZ(initializer.DeltaZ), _deltaMode(initializer.DeltaMode)
  {
  }

  WheelEvent::WheelEvent(const PlatformWheelEvent &event, boo::FrameView *view,
                         IsCancelable isCancelable) noexcept
      : MouseEvent(event_names::Wheel, WheelEventInitFor(isCancelable), view,
                   WheelPoint(event.GlobalPosition()), WheelPoint(event.Position()), 0, 0, event.Modifiers(),
                   MouseButton::Left, 0, nullptr, 0, SyntheticClickType::NoTap, {}, {}, Simulated::No),
        // The host's deltas are the way the content moves, the DOM's the way the view scrolls.
        _deltaX(-event.DeltaX()), _deltaY(-event.DeltaY()), _deltaMode(WheelDeltaMode(event)),
        _underlyingPlatformEvent(event)
  {
    _createTime = event.Timestamp();
  }

  Ref<WheelEvent> WheelEvent::Create(const PlatformWheelEvent &event, boo::FrameView *view,
                                     IsCancelable isCancelable) noexcept
  {
    return AdoptRef(*new WheelEvent(event, view, isCancelable));
  }
}
