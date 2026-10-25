#include "Krystal.Booey/Platform/Gamepad/EmptyGamepadProvider.hpp"
#include "Krystal.Booey/Platform/Gamepad/PlatformGamepad.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo
{
  void EmptyGamepadProvider::StartMonitoringGamepads(GamepadProviderClient &) noexcept
  {
  }

  void EmptyGamepadProvider::StopMonitoringGamepads(GamepadProviderClient &) noexcept
  {
  }

  const List<WeakPtr<PlatformGamepad>> &EmptyGamepadProvider::PlatformGamepads() noexcept
  {
    static NeverDestroyed<List<WeakPtr<PlatformGamepad>>> emptyGamepads;
    return emptyGamepads;
  }

  void EmptyGamepadProvider::PlayEffect(uint32, utf8_stringview, GamepadHapticEffectType,
                                        const GamepadEffectParameters &,
                                        Func<void(bool)> &&completionHandler) noexcept
  {
    completionHandler(false);
  }

  void EmptyGamepadProvider::StopEffects(uint32, utf8_stringview, Func<void()> &&completionHandler) noexcept
  {
    completionHandler();
  }
}
