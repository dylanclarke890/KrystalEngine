#include "Krystal.Booey/Platform/Gamepad/GamepadProvider.hpp"
#include "Krystal.Booey/Platform/Gamepad/EmptyGamepadProvider.hpp"
#include "Krystal.Booey/Platform/Gamepad/GamepadProviderClient.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo
{
  namespace
  {
    GamepadProvider *SharedProvider = nullptr;
  }

  GamepadProvider &GamepadProvider::Singleton() noexcept
  {
    if (!SharedProvider)
    {
      static NeverDestroyed<EmptyGamepadProvider> defaultProvider;
      SharedProvider = &static_cast<EmptyGamepadProvider &>(defaultProvider);
    }
    return *SharedProvider;
  }

  void GamepadProvider::SetSharedProvider(GamepadProvider &newProvider) noexcept
  {
    SharedProvider = &newProvider;
  }

  void GamepadProvider::DispatchPlatformGamepadInputActivity() noexcept
  {
    auto eventVisibility =
      _shouldMakeGamepadsVisible ? EventMakesGamepadsVisible::Yes : EventMakesGamepadsVisible::No;
    for (auto *client : List<GamepadProviderClient *>(_clients))
    {
      client->PlatformGamepadInputActivity(eventVisibility);
    }
    _shouldMakeGamepadsVisible = false;
  }
}
