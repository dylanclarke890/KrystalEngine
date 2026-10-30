#pragma once

#include "Krystal.Booey/Platform/PlatformEvent.hpp"
#include "Krystal.Booey/Platform/PlatformTouchPoint.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  /// @brief A change to the touches on the host's touch screen: every touch on it, each with its state.
  class PlatformTouchEvent : public PlatformEvent
  {
  protected:
    List<PlatformTouchPoint> _touchPoints;
    List<PlatformTouchEvent> _coalescedEvents;
    List<PlatformTouchEvent> _predictedEvents;

  public:
    PlatformTouchEvent() noexcept : PlatformEvent(PlatformEvent::Type::TouchStart)
    {
    }

    /// @param type `TouchStart`, `TouchMove`, `TouchEnd` or `TouchCancel`, as the touches changed.
    /// @param touchPoints Every touch on the screen, those that didn't change as `TouchStationary`.
    PlatformTouchEvent(PlatformEvent::Type type, List<PlatformTouchPoint> touchPoints,
                       EnumSet<PlatformEvent::Modifier> modifiers, MonotonicTime::duration timestamp) noexcept
        : PlatformEvent(type, modifiers, timestamp), _touchPoints(krys::move(touchPoints))
    {
    }

    KRYS_NODISCARD const List<PlatformTouchPoint> &TouchPoints() const noexcept
    {
      return _touchPoints;
    }

    /// @brief The changes this one stands for, oldest first, when the host merged several into it.
    KRYS_NODISCARD const List<PlatformTouchEvent> &CoalescedEvents() const noexcept
    {
      return _coalescedEvents;
    }

    void SetCoalescedEvents(List<PlatformTouchEvent> coalescedEvents) noexcept
    {
      _coalescedEvents = krys::move(coalescedEvents);
    }

    /// @brief Where the host predicts the touches will move next, soonest first.
    KRYS_NODISCARD const List<PlatformTouchEvent> &PredictedEvents() const noexcept
    {
      return _predictedEvents;
    }

    void SetPredictedEvents(List<PlatformTouchEvent> predictedEvents) noexcept
    {
      _predictedEvents = krys::move(predictedEvents);
    }
  };
}
