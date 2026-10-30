#include "Krystal.Booey/Modules/Gamepad/NavigatorGamepad.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Modules/Gamepad/Gamepad.hpp"
#include "Krystal.Booey/Modules/Gamepad/GamepadHapticActuator.hpp"
#include "Krystal.Booey/Modules/Gamepad/GamepadManager.hpp"
#include "Krystal.Booey/Page/Navigator.hpp"
#include "Krystal.Booey/Platform/Gamepad/GamepadProvider.hpp"
#include "Krystal.Booey/Platform/Gamepad/PlatformGamepad.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo
{
  NavigatorGamepad::NavigatorGamepad(Navigator &navigator) noexcept : _navigator(navigator)
  {
    GamepadManager::Singleton().RegisterNavigator(navigator);
  }

  NavigatorGamepad::~NavigatorGamepad() noexcept
  {
    GamepadManager::Singleton().UnregisterNavigator(_navigator);
  }

  NavigatorGamepad &NavigatorGamepad::From(Navigator &navigator) noexcept
  {
    if (!navigator._gamepad)
    {
      navigator._gamepad = CreateUnique<NavigatorGamepad>(navigator);
    }
    return *navigator._gamepad;
  }

  Ref<Gamepad> NavigatorGamepad::GamepadFromPlatformGamepad(PlatformGamepad &platformGamepad) noexcept
  {
    auto index = platformGamepad.Index();
    if (index >= _gamepads.size() || !_gamepads[index])
    {
      return Gamepad::Create(_navigator.Document(), platformGamepad);
    }
    return ShareRef(*_gamepads[index]);
  }

  const List<RefPtr<Gamepad>> &NavigatorGamepad::GetGamepads(Navigator &navigator) noexcept
  {
    auto *document = navigator.Document();
    if (!document || !document->IsFullyActive())
    {
      static NeverDestroyed<List<RefPtr<Gamepad>>> emptyGamepads;
      return emptyGamepads;
    }
    // FIXME: A `SecurityError` where the permissions policy doesn't allow the `gamepad` feature, once there
    // are child frames.
    return NavigatorGamepad::From(navigator).Gamepads();
  }

  const List<RefPtr<Gamepad>> &NavigatorGamepad::Gamepads() noexcept
  {
    if (_gamepads.empty())
    {
      return _gamepads;
    }

    auto &platformGamepads = GamepadProvider::Singleton().PlatformGamepads();
    for (size_t i = 0; i < platformGamepads.size(); ++i)
    {
      auto *gamepad = platformGamepads[i].get();
      if (!gamepad)
      {
        krys_debug_assert(i >= _gamepads.size() || !_gamepads[i]);
        continue;
      }
      _gamepads[i]->UpdateFromPlatformGamepad(*gamepad);
    }
    return _gamepads;
  }

  void NavigatorGamepad::GamepadsBecameVisible() noexcept
  {
    auto &platformGamepads = GamepadProvider::Singleton().PlatformGamepads();
    _gamepads.resize(platformGamepads.size());
    for (size_t i = 0; i < platformGamepads.size(); ++i)
    {
      if (auto *gamepad = platformGamepads[i].get())
      {
        _gamepads[i] = Gamepad::Create(_navigator.Document(), *gamepad);
      }
    }
  }

  void NavigatorGamepad::GamepadConnected(PlatformGamepad &platformGamepad) noexcept
  {
    // If this is the first gamepad this Navigator object has seen, then all gamepads just became visible.
    if (_gamepads.empty())
    {
      GamepadsBecameVisible();
      return;
    }

    auto index = platformGamepad.Index();
    krys_debug_assert(GamepadProvider::Singleton().PlatformGamepads()[index].get() == &platformGamepad);

    // The new index should already fit in the existing array, or should be exactly one past-the-end of the
    // existing array.
    krys_debug_assert(index <= _gamepads.size());
    if (index < _gamepads.size())
    {
      _gamepads[index] = Gamepad::Create(_navigator.Document(), platformGamepad);
    }
    else if (index == _gamepads.size())
    {
      _gamepads.push_back(Gamepad::Create(_navigator.Document(), platformGamepad));
    }
  }

  void NavigatorGamepad::GamepadDisconnected(PlatformGamepad &platformGamepad) noexcept
  {
    // If this Navigator hasn't seen any gamepads yet its Vector will still be empty.
    if (_gamepads.empty())
    {
      return;
    }

    krys_debug_assert(platformGamepad.Index() < _gamepads.size());
    krys_debug_assert(_gamepads[platformGamepad.Index()]);
    if (auto &gamepad = _gamepads[platformGamepad.Index()])
    {
      gamepad->SetConnected(false);
      gamepad = nullptr;
    }
  }

  void NavigatorGamepad::StopHapticEffects() noexcept
  {
    for (auto &gamepad : _gamepads)
    {
      if (auto *actuator = gamepad ? gamepad->VibrationActuator() : nullptr)
      {
        actuator->Stop();
      }
    }
  }
}
