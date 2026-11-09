#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.PAL/KeyModifiers.hpp"

namespace krys::pal
{
  /// @brief Where a precision touchpad's pan is: the fingers move (`Began`, then `Changed`), then either
  /// lift (`Ended`) or flick, the content coasting on (`MomentumBegan`, `MomentumChanged`, `MomentumEnded`).
  /// A pan is over at `Ended` or `MomentumEnded`.
  enum class TouchpadPanPhase : uint8
  {
    /// @brief The fingers started moving.
    Began,
    /// @brief They moved on.
    Changed,
    /// @brief They lifted without flicking.
    Ended,
    /// @brief They flicked as they lifted, and the content coasts on.
    MomentumBegan,
    /// @brief The content coasted on.
    MomentumChanged,
    /// @brief The content stopped coasting, or a new touch stopped it.
    MomentumEnded,
  };

  /// @brief A step of a precision touchpad's two-finger pan, as the window reports it.
  struct TouchpadPanInput
  {
    TouchpadPanPhase Phase {TouchpadPanPhase::Changed};
    /// @brief How far the content moves with the fingers since the last step, in client pixels: positive is
    /// right and down, which scrolls towards the left and the top.
    float DeltaX {0};
    float DeltaY {0};
    /// @brief The cursor's position in client space, where the pan is.
    float ClientX {0};
    float ClientY {0};
    KeyModifiers Modifiers;
  };
}
