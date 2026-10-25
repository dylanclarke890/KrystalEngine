#pragma once

#include "Krystal.Booey/Modules/Gamepad/GamepadHapticEffectType.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class GamepadProviderClient;
  class PlatformGamepad;
  struct GamepadEffectParameters;

  /// @brief Where gamepads come from: the host's pads, for the clients monitoring them. There's one, shared;
  /// it has no pads unless the host sets its own (`SetSharedProvider`).
  class GamepadProvider
  {
  protected:
    List<GamepadProviderClient *> _clients;

  private:
    bool _shouldMakeGamepadsVisible {false};

  public:
    virtual ~GamepadProvider() noexcept = default;

    KRYS_NODISCARD static GamepadProvider &Singleton() noexcept;
    static void SetSharedProvider(GamepadProvider &newProvider) noexcept;

    virtual void StartMonitoringGamepads(GamepadProviderClient &client) noexcept = 0;
    virtual void StopMonitoringGamepads(GamepadProviderClient &client) noexcept = 0;

    /// @brief The connected pads, each at its index; null where none is.
    KRYS_NODISCARD virtual const List<WeakPtr<PlatformGamepad>> &PlatformGamepads() noexcept = 0;

    /// @brief Plays @p type's effect on the pad at @p gamepadIndex, if it's still the one with @p gamepadId,
    /// calling @p completionHandler with whether it played to the end.
    virtual void PlayEffect(uint32 gamepadIndex, utf8_stringview gamepadId, GamepadHapticEffectType type,
                            const GamepadEffectParameters &parameters,
                            Func<void(bool)> &&completionHandler) noexcept = 0;
    /// @brief Stops the effects of the pad at @p gamepadIndex, then calls @p completionHandler.
    virtual void StopEffects(uint32 gamepadIndex, utf8_stringview gamepadId,
                             Func<void()> &&completionHandler) noexcept = 0;

  protected:
    /// @brief Tells the clients that the pads were used, making them visible if a button was pressed or
    /// released since the last time.
    void DispatchPlatformGamepadInputActivity() noexcept;

    void SetShouldMakeGamepadsVisible() noexcept
    {
      _shouldMakeGamepadsVisible = true;
    }
  };
}
