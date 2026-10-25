#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Platform/Gamepad/GamepadProviderClient.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  class LocalDOMWindow;
  class Navigator;

  /// @brief Connects the gamepad provider to the navigators that have looked at gamepads and the windows
  /// with gamepad event listeners: it monitors the pads while there are any, and keeps the pads hidden from
  /// each until a pad is used (as the standard asks, so that pads can't be used to fingerprint the user).
  /// @see https://w3c.github.io/gamepad/#fingerprinting-mitigation
  class GamepadManager final : public GamepadProviderClient
  {
    KRYS_NON_COPY_MOVABLE(GamepadManager);

  private:
    bool _isMonitoringGamepads {false};
    List<Navigator *> _navigators;
    List<Navigator *> _gamepadBlindNavigators;
    List<LocalDOMWindow *> _domWindows;
    List<LocalDOMWindow *> _gamepadBlindDOMWindows;

  public:
    GamepadManager() noexcept = default;

    KRYS_NODISCARD static GamepadManager &Singleton() noexcept;

    void PlatformGamepadConnected(PlatformGamepad &platformGamepad,
                                  EventMakesGamepadsVisible eventVisibility) noexcept final;
    void PlatformGamepadDisconnected(PlatformGamepad &platformGamepad) noexcept final;
    void PlatformGamepadInputActivity(EventMakesGamepadsVisible eventVisibility) noexcept final;

    void RegisterNavigator(Navigator &navigator) noexcept;
    void UnregisterNavigator(Navigator &navigator) noexcept;
    void RegisterDOMWindow(LocalDOMWindow &window) noexcept;
    void UnregisterDOMWindow(LocalDOMWindow &window) noexcept;

  private:
    void MakeGamepadVisible(PlatformGamepad &platformGamepad, const List<Navigator *> &navigatorSet,
                            const List<LocalDOMWindow *> &domWindowSet) noexcept;
    void MaybeStartMonitoringGamepads() noexcept;
    void MaybeStopMonitoringGamepads() noexcept;
  };
}
