#pragma once

namespace krys::boo::dom
{
  /// @see https://w3c.github.io/pointerlock/#pointerlockoptions-dictionary
  struct PointerLockOptions
  {
    /// @brief Whether the locked element's mouse moves carry the device's movement before the system's
    /// acceleration.
    bool UnadjustedMovement = false;
  };
}
