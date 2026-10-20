#include "Krystal.Booey/DOM/WheelEvent.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"

namespace krys::boo::dom
{
  namespace
  {
    KRYS_NODISCARD uint32 DetermineDeltaMode(const PlatformWheelEvent &event) noexcept
    {
      return event.Granularity() == PlatformWheelEventGranularity::ScrollByPageWheelEvent
               ? WheelEvent::DOM_DELTA_PAGE
               : WheelEvent::DOM_DELTA_PIXEL;
    }

    KRYS_NODISCARD UIEventInit TrustedInit(WheelEvent::IsCancelable isCancelable) noexcept
    {
      UIEventInit init;
      init.Bubbles = true;
      init.Cancelable = isCancelable == WheelEvent::IsCancelable::Yes;
      init.Composed = true;
      return init;
    }

    KRYS_NODISCARD DoublePoint ToDoublePoint(const IntPoint &point) noexcept
    {
      return {static_cast<double>(point.X()), static_cast<double>(point.Y())};
    }
  }

  WheelEvent::WheelEvent(DOMStringAtom type, const WheelEventInit &initializer) noexcept
      : MouseEvent(type, initializer), _deltaX(initializer.DeltaX), _deltaY(initializer.DeltaY),
        _deltaZ(initializer.DeltaZ), _deltaMode(initializer.DeltaMode)
  {
  }

  WheelEvent::WheelEvent(const PlatformWheelEvent &event, LocalFrameView *view,
                         IsCancelable isCancelable) noexcept
      : MouseEvent(DOMStringAtom(EventNames::Wheel), TrustedInit(isCancelable), view,
                   ToDoublePoint(event.GlobalPosition()), ToDoublePoint(event.Position()), 0, 0,
                   event.Modifiers(), MouseButton::Left, 0, nullptr, Simulated::No),
        _deltaX(-event.DeltaX()), _deltaY(-event.DeltaY()), _deltaMode(DetermineDeltaMode(event)),
        _underlyingPlatformEvent(event)
  {
    _timeStamp = static_cast<DOMHighResTimeStamp>(event.Timestamp().count());
  }

  Ref<WheelEvent> WheelEvent::Create(const PlatformWheelEvent &event, LocalFrameView *view,
                                     IsCancelable isCancelable) noexcept
  {
    return AdoptRef(*new WheelEvent(event, view, isCancelable));
  }
}
