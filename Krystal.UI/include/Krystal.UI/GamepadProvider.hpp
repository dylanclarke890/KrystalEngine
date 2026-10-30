#pragma once

#include "Krystal.Booey/Platform/Gamepad/GamepadProvider.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::pal
{
  class Input;
}

namespace krys::ui
{
  /// @brief The platform layer's gamepads, for pages: each of the input's gamepad slots is a pad at the same
  /// index, with the standard mapping (the platform layer's buttons and axes are in its order) and rumble.
  /// The host makes it the shared provider (`boo::GamepadProvider::SetSharedProvider`) and updates it each
  /// frame, after the input has read the pads.
  class GamepadProvider final : public boo::GamepadProvider
  {
    KRYS_NON_COPY_MOVABLE(GamepadProvider);

    class Gamepad;

  private:
    pal::Input &_input;
    /// @brief The pad in each slot, while monitoring.
    List<UniquePtr<Gamepad>> _gamepads;
    List<WeakPtr<boo::PlatformGamepad>> _platformGamepads;
    /// @brief The time of the last update, which effects start from.
    MonotonicTime::duration _now {};

  public:
    explicit GamepadProvider(pal::Input &input) noexcept;
    ~GamepadProvider() noexcept final;

    /// @brief Takes the pads' connections and values from the input, telling the clients, and starts and
    /// ends the effects due by @p now, the host's time (the event loop's, which the pads' timestamps are
    /// on).
    void Update(MonotonicTime::duration now) noexcept;

    void StartMonitoringGamepads(boo::GamepadProviderClient &client) noexcept final;
    void StopMonitoringGamepads(boo::GamepadProviderClient &client) noexcept final;
    KRYS_NODISCARD const List<WeakPtr<boo::PlatformGamepad>> &PlatformGamepads() noexcept final;
    void PlayEffect(uint32 gamepadIndex, utf8_stringview gamepadId, boo::GamepadHapticEffectType type,
                    const boo::GamepadEffectParameters &parameters,
                    Func<void(bool)> &&completionHandler) noexcept final;
    void StopEffects(uint32 gamepadIndex, utf8_stringview gamepadId,
                     Func<void()> &&completionHandler) noexcept final;

  private:
    /// @brief The pad at @p index, if it's still the one with @p id.
    KRYS_NODISCARD Gamepad *GamepadFor(uint32 index, utf8_stringview id) const noexcept;
    void Connect(uint32 slot) noexcept;
    void Disconnect(uint32 slot) noexcept;
    void UpdateEffect(Gamepad &gamepad) noexcept;
  };
}
