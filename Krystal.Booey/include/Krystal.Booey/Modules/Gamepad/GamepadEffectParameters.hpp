#pragma once

namespace krys::boo
{
  /// @brief How a haptic effect plays.
  /// @see https://w3c.github.io/gamepad/#dom-gamepadeffectparameters
  struct GamepadEffectParameters
  {
    /// @brief How long the effect plays for, in milliseconds.
    double Duration {0.0};
    /// @brief How long to wait before the effect starts, in milliseconds.
    double StartDelay {0.0};
    /// @brief The heavy motor's strength, from 0 to 1.
    double StrongMagnitude {0.0};
    /// @brief The light motor's strength, from 0 to 1.
    double WeakMagnitude {0.0};

    /// @brief The longest an effect plays for, in milliseconds.
    /// @note A maximum duration of 5 seconds is recommended by the specification.
    constexpr static double MaximumDuration = 5'000.0;
  };
}
