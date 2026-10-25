#pragma once

#include "Krystal.Booey/Platform/Gamepad/GamepadProvider.hpp"

namespace krys::boo
{
  /// @brief A provider without gamepads, for hosts that don't have any.
  class EmptyGamepadProvider final : public GamepadProvider
  {
  public:
    ~EmptyGamepadProvider() noexcept final = default;

  private:
    void StartMonitoringGamepads(GamepadProviderClient &client) noexcept final;
    void StopMonitoringGamepads(GamepadProviderClient &client) noexcept final;
    KRYS_NODISCARD const List<WeakPtr<PlatformGamepad>> &PlatformGamepads() noexcept final;
    void PlayEffect(uint32 gamepadIndex, utf8_stringview gamepadId, GamepadHapticEffectType type,
                    const GamepadEffectParameters &parameters,
                    Func<void(bool)> &&completionHandler) noexcept final;
    void StopEffects(uint32 gamepadIndex, utf8_stringview gamepadId,
                     Func<void()> &&completionHandler) noexcept final;
  };
}
