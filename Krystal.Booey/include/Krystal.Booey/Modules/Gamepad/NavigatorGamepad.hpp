#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class Gamepad;
  class Navigator;
  class PlatformGamepad;

  /// @brief A navigator's gamepads: none until a pad is used while the navigator is looking (after which
  /// it sees them all), then a snapshot of each connected pad at its index.
  /// @see https://w3c.github.io/gamepad/#extensions-to-the-navigator-interface
  class NavigatorGamepad
  {
    KRYS_NON_COPY_MOVABLE(NavigatorGamepad);

  private:
    Navigator &_navigator;
    List<RefPtr<Gamepad>> _gamepads;

  public:
    explicit NavigatorGamepad(Navigator &navigator) noexcept;
    ~NavigatorGamepad() noexcept;

    /// @brief @p navigator's gamepads, made (and registered with the manager) the first time.
    KRYS_NODISCARD static NavigatorGamepad &From(Navigator &navigator) noexcept;

    KRYS_NODISCARD Navigator &GetNavigator() const noexcept
    {
      return _navigator;
    }

    /// @brief The gamepads, updated from the pads' current values; null at an index without one.
    /// @note The array of Gamepads might be sparse. Null checking each entry is necessary.
    /// @see https://w3c.github.io/gamepad/#dom-navigator-getgamepads
    KRYS_NODISCARD static const List<RefPtr<Gamepad>> &GetGamepads(Navigator &navigator) noexcept;

    void GamepadConnected(PlatformGamepad &platformGamepad) noexcept;
    void GamepadDisconnected(PlatformGamepad &platformGamepad) noexcept;
    /// @brief The gamepad for @p platformGamepad: the navigator's, if it can see it, or a new one.
    KRYS_NODISCARD Ref<Gamepad> GamepadFromPlatformGamepad(PlatformGamepad &platformGamepad) noexcept;

    /// @brief Stops the effects the gamepads' haptic actuators are playing, as the document is suspended or
    /// goes away.
    void StopHapticEffects() noexcept;

  private:
    void GamepadsBecameVisible() noexcept;
    KRYS_NODISCARD const List<RefPtr<Gamepad>> &Gamepads() noexcept;
  };
}
