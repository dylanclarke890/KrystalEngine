#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Platform/Keys.hpp"

namespace Krys::Platform
{
  class IInput;

  /// @brief Manages the state of keyboard input for the current frame.
  ///
  /// The `Keyboard` class provides methods to query the current state of keys,
  /// including whether a key was pressed, held, or released during the current frame.
  /// Key states are updated by the `InputManager` each frame, allowing for accurate
  /// per-frame input handling.
  class Keyboard
  {
    friend class IInput;

  public:
    /// @brief Constructs a `Keyboard`.
    Keyboard() noexcept = default;

    /// @brief Checks if a key was pressed this frame.
    /// @param key The key to check.
    /// @returns `true` if the key is in the state `KeyState::Pressed`.
    NO_DISCARD bool IsKeyPressed(Key key) const noexcept;

    /// @brief Checks if a key is being held this frame.
    /// @param key The key to check.
    /// @returns `true` if the key is in the state `KeyState::Held`.
    NO_DISCARD bool IsKeyHeld(Key key) const noexcept;

    /// @brief Checks if a key was released this frame.
    /// @param key The key to check.
    /// @returns `true` if the key is in the state `KeyState::Released`.
    NO_DISCARD bool WasKeyReleased(Key key) const noexcept;

  private:
    Set<Key> _pressed, _held, _released;
  };
}