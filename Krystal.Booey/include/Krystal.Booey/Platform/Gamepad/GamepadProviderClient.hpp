#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  class PlatformGamepad;

  /// @brief Whether an event lets documents that haven't seen a gamepad yet see them all.
  enum class EventMakesGamepadsVisible : bool
  {
    No,
    Yes,
  };

  /// @brief Hears from a gamepad provider about pads connecting, disconnecting and being used.
  class GamepadProviderClient
  {
  public:
    virtual ~GamepadProviderClient() noexcept = default;

    virtual void PlatformGamepadConnected(PlatformGamepad &gamepad,
                                          EventMakesGamepadsVisible eventVisibility) noexcept = 0;
    virtual void PlatformGamepadDisconnected(PlatformGamepad &gamepad) noexcept = 0;
    virtual void PlatformGamepadInputActivity(EventMakesGamepadsVisible eventVisibility) noexcept = 0;
  };
}
