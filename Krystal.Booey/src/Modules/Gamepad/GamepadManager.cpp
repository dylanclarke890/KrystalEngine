#include "Krystal.Booey/Modules/Gamepad/GamepadManager.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Modules/Gamepad/Gamepad.hpp"
#include "Krystal.Booey/Modules/Gamepad/GamepadEvent.hpp"
#include "Krystal.Booey/Modules/Gamepad/NavigatorGamepad.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/Navigator.hpp"
#include "Krystal.Booey/Platform/Gamepad/GamepadProvider.hpp"
#include "Krystal.Booey/Platform/Gamepad/PlatformGamepad.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include <algorithm>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD NavigatorGamepad &NavigatorGamepadFromDOMWindow(LocalDOMWindow &window) noexcept
    {
      return NavigatorGamepad::From(window.GetNavigator());
    }

    template <typename T>
    KRYS_NODISCARD bool Contains(const List<T *> &set, const T &value) noexcept
    {
      return std::ranges::find(set, &value) != set.end();
    }

    template <typename T>
    void Remove(List<T *> &set, const T &value) noexcept
    {
      std::erase(set, &value);
    }

    /// @brief The windows in @p windows, held weakly, as dispatching events may make them go away.
    KRYS_NODISCARD List<WeakPtr<LocalDOMWindow>> WeakWindows(const List<LocalDOMWindow *> &windows) noexcept
    {
      List<WeakPtr<LocalDOMWindow>> weakWindows;
      weakWindows.reserve(windows.size());
      for (auto *window : windows)
      {
        weakWindows.push_back(CreateWeakPtr(window));
      }
      return weakWindows;
    }

    /// @brief Fires @p event at @p window.
    void DispatchEvent(LocalDOMWindow &window, dom::Event &event) noexcept
    {
      if (RefPtr<dom::Document> document = ShareRefPtr(window.Document()))
      {
        document->DispatchWindowEvent(event);
      }
    }
  }

  GamepadManager &GamepadManager::Singleton() noexcept
  {
    static NeverDestroyed<GamepadManager> sharedManager;
    return sharedManager;
  }

  void GamepadManager::PlatformGamepadConnected(PlatformGamepad &platformGamepad,
                                                EventMakesGamepadsVisible eventVisibility) noexcept
  {
    if (eventVisibility == EventMakesGamepadsVisible::No)
    {
      return;
    }

    // Notify blind Navigators and Windows about all gamepads except for this one.
    for (auto &weakGamepad : List<WeakPtr<PlatformGamepad>>(GamepadProvider::Singleton().PlatformGamepads()))
    {
      auto *gamepad = weakGamepad.get();
      if (!gamepad || gamepad == &platformGamepad)
      {
        continue;
      }
      MakeGamepadVisible(*gamepad, _gamepadBlindNavigators, _gamepadBlindDOMWindows);
    }

    _gamepadBlindNavigators.clear();
    _gamepadBlindDOMWindows.clear();

    // Notify everyone of this new gamepad.
    MakeGamepadVisible(platformGamepad, _navigators, _domWindows);
  }

  void GamepadManager::PlatformGamepadDisconnected(PlatformGamepad &platformGamepad) noexcept
  {
    List<Navigator *> notifiedNavigators;

    // Handle the disconnect for all DOMWindows with event listeners and their Navigators.
    for (auto &window : WeakWindows(_domWindows))
    {
      // Event dispatch might have made this window go away.
      if (!window)
      {
        continue;
      }

      Ref<Navigator> navigator = ShareRef(window->GetNavigator());

      // If this Navigator hasn't seen gamepads yet then its Window should not get the disconnect event.
      if (Contains(_gamepadBlindNavigators, *navigator))
      {
        continue;
      }

      auto &navigatorGamepad = NavigatorGamepad::From(*navigator);
      Ref<Gamepad> gamepad = navigatorGamepad.GamepadFromPlatformGamepad(platformGamepad);
      navigatorGamepad.GamepadDisconnected(platformGamepad);
      notifiedNavigators.push_back(navigator.get());
      DispatchEvent(*window,
                    *GamepadEvent::Create(dom::DOMStringAtom(EventNames::GamepadDisconnected), *gamepad));
    }

    // Notify all the Navigators that haven't already been notified.
    for (auto *navigator : List<Navigator *>(_navigators))
    {
      if (!Contains(notifiedNavigators, *navigator))
      {
        NavigatorGamepad::From(*navigator).GamepadDisconnected(platformGamepad);
      }
    }
  }

  void GamepadManager::PlatformGamepadInputActivity(EventMakesGamepadsVisible eventVisibility) noexcept
  {
    if (eventVisibility == EventMakesGamepadsVisible::No)
    {
      return;
    }

    if (_gamepadBlindNavigators.empty() && _gamepadBlindDOMWindows.empty())
    {
      return;
    }

    for (auto &weakGamepad : List<WeakPtr<PlatformGamepad>>(GamepadProvider::Singleton().PlatformGamepads()))
    {
      if (auto *gamepad = weakGamepad.get())
      {
        MakeGamepadVisible(*gamepad, _gamepadBlindNavigators, _gamepadBlindDOMWindows);
      }
    }

    _gamepadBlindNavigators.clear();
    _gamepadBlindDOMWindows.clear();
  }

  void GamepadManager::MakeGamepadVisible(PlatformGamepad &platformGamepad,
                                          const List<Navigator *> &navigatorSet,
                                          const List<LocalDOMWindow *> &domWindowSet) noexcept
  {
    if (navigatorSet.empty() && domWindowSet.empty())
    {
      return;
    }

    for (auto *navigator : List<Navigator *>(navigatorSet))
    {
      NavigatorGamepad::From(*navigator).GamepadConnected(platformGamepad);
    }

    for (auto &window : WeakWindows(domWindowSet))
    {
      // Event dispatch might have made this window go away.
      if (!window)
      {
        continue;
      }

      auto &navigator = NavigatorGamepadFromDOMWindow(*window);
      Ref<Gamepad> gamepad = navigator.GamepadFromPlatformGamepad(platformGamepad);
      // Connecting a gamepad counts as a user gesture.
      window->NotifyActivated();
      DispatchEvent(*window,
                    *GamepadEvent::Create(dom::DOMStringAtom(EventNames::GamepadConnected), *gamepad));
    }
  }

  void GamepadManager::RegisterNavigator(Navigator &navigator) noexcept
  {
    krys_debug_assert(!Contains(_navigators, navigator));
    _navigators.push_back(&navigator);
    _gamepadBlindNavigators.push_back(&navigator);
    MaybeStartMonitoringGamepads();
  }

  void GamepadManager::UnregisterNavigator(Navigator &navigator) noexcept
  {
    krys_debug_assert(Contains(_navigators, navigator));
    Remove(_navigators, navigator);
    Remove(_gamepadBlindNavigators, navigator);
    MaybeStopMonitoringGamepads();
  }

  void GamepadManager::RegisterDOMWindow(LocalDOMWindow &window) noexcept
  {
    krys_debug_assert(!Contains(_domWindows, window));
    _domWindows.push_back(&window);

    // Anytime we register a LocalDOMWindow, we should make sure its NavigatorGamepad is constructed.
    auto &navigator = NavigatorGamepadFromDOMWindow(window).GetNavigator();
    if (Contains(_navigators, navigator))
    {
      // If this LocalDOMWindow's NavigatorGamepad was already registered but was still blind, then this
      // LocalDOMWindow should be blind.
      if (Contains(_gamepadBlindNavigators, navigator))
      {
        _gamepadBlindDOMWindows.push_back(&window);
      }
      MaybeStartMonitoringGamepads();
    }
  }

  void GamepadManager::UnregisterDOMWindow(LocalDOMWindow &window) noexcept
  {
    krys_debug_assert(Contains(_domWindows, window));
    Remove(_domWindows, window);
    Remove(_gamepadBlindDOMWindows, window);
    MaybeStopMonitoringGamepads();
  }

  void GamepadManager::MaybeStartMonitoringGamepads() noexcept
  {
    if (_isMonitoringGamepads)
    {
      return;
    }

    if (!_navigators.empty() || !_domWindows.empty())
    {
      _isMonitoringGamepads = true;
      GamepadProvider::Singleton().StartMonitoringGamepads(*this);
    }
  }

  void GamepadManager::MaybeStopMonitoringGamepads() noexcept
  {
    if (!_isMonitoringGamepads)
    {
      return;
    }

    if (_navigators.empty() && _domWindows.empty())
    {
      _isMonitoringGamepads = false;
      GamepadProvider::Singleton().StopMonitoringGamepads(*this);
    }
  }
}
