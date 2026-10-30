#pragma once

#include "Krystal.Booey/DOM/Dicts/MouseEventInit.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <numbers>

namespace krys::boo::dom
{
  class PointerEvent;

  /// @see https://w3c.github.io/pointerevents/#pointereventinit-dictionary
  struct PointerEventInit : MouseEventInit
  {
    int32 PointerId {0};
    double Width {1};
    double Height {1};
    float Pressure {0};
    float TangentialPressure {0};
    int32 TiltX {0};
    int32 TiltY {0};
    int32 Twist {0};
    /// @brief Computed from the tilt when it's left as it is (with the azimuth).
    double AltitudeAngle {std::numbers::pi / 2};
    /// @brief Computed from the tilt when it's left as it is (with the altitude).
    double AzimuthAngle {0};
    DOMString PointerType;
    bool IsPrimary {false};
    List<Ref<PointerEvent>> CoalescedEvents;
    List<Ref<PointerEvent>> PredictedEvents;
  };
}
