#include "Krystal.UI/GamepadProvider.hpp"
#include "Krystal.Booey/Modules/Gamepad/GamepadEffectParameters.hpp"
#include "Krystal.Booey/Platform/Gamepad/GamepadConstants.hpp"
#include "Krystal.Booey/Platform/Gamepad/GamepadProviderClient.hpp"
#include "Krystal.Booey/Platform/Gamepad/PlatformGamepad.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.PAL/Input.hpp"
#include <algorithm>
#include <chrono>
#include <utility>

namespace krys::ui
{
  /// @brief The pad in one of the input's slots.
  class GamepadProvider::Gamepad final : public boo::PlatformGamepad
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Gamepad);

  public:
    /// @brief An effect waiting to start, or playing.
    struct Effect
    {
      MonotonicTime::duration Start {};
      MonotonicTime::duration End {};
      float Strong {0};
      float Weak {0};
      bool Started {false};
      Func<void(bool)> CompletionHandler;
    };

  private:
    List<double> _axes = List<double>(pal::GamepadAxisCount, 0.0);
    List<double> _buttons = List<double>(pal::GamepadButtonCount, 0.0);
    /// @brief When the platform layer last saw the pad change (on the monotonic clock, not the host's).
    nanoseconds _lastChanged {};

  public:
    Maybe<Effect> CurrentEffect;

    Gamepad(const pal::Gamepad &pad, MonotonicTime::duration now) noexcept : PlatformGamepad(pad.Slot())
    {
      _id = pad.Name();
      _mapping = utf8_string(boo::StandardGamepadMapping);
      _connectTime = now;
      _supportedEffectTypes.Add(boo::GamepadHapticEffectType::DualRumble);
      (void)Update(pad, now);
    }

    KRYS_NODISCARD const List<double> &AxisValues() const noexcept final
    {
      return _axes;
    }

    KRYS_NODISCARD const List<double> &ButtonValues() const noexcept final
    {
      return _buttons;
    }

    KRYS_NODISCARD utf8_stringview Source() const noexcept final
    {
      return u8"PAL";
    }

    /// @brief Takes @p pad's values; if they changed, they were last updated at the host's time @p now.
    /// @return Whether they changed since the last time.
    bool Update(const pal::Gamepad &pad, MonotonicTime::duration now) noexcept
    {
      for (size_t i = 0; i < pal::GamepadAxisCount; ++i)
      {
        _axes[i] = pad.AxisValue(static_cast<pal::GamepadAxis>(i));
      }
      for (size_t i = 0; i < pal::GamepadButtonCount; ++i)
      {
        _buttons[i] = pad.ButtonValue(static_cast<pal::GamepadButton>(i));
      }

      if (std::exchange(_lastChanged, pad.LastChanged()) == pad.LastChanged())
      {
        return false;
      }
      _lastUpdateTime = now;
      return true;
    }
  };

  namespace
  {
    /// @brief Whether a button of @p pad was pressed or released this frame.
    KRYS_NODISCARD bool ButtonChanged(const pal::Gamepad &pad) noexcept
    {
      for (size_t i = 0; i < pal::GamepadButtonCount; ++i)
      {
        auto button = static_cast<pal::GamepadButton>(i);
        if (pad.IsButtonPressed(button) || pad.WasButtonReleased(button))
        {
          return true;
        }
      }
      return false;
    }
  }

  GamepadProvider::GamepadProvider(pal::Input &input) noexcept : _input(input)
  {
  }

  GamepadProvider::~GamepadProvider() noexcept
  {
    for (auto &gamepad : _gamepads)
    {
      if (gamepad && gamepad->CurrentEffect)
      {
        (void)_input.SetGamepadRumble(gamepad->Index(), 0, 0);
      }
    }
  }

  void GamepadProvider::Update(MonotonicTime::duration now) noexcept
  {
    _now = now;
    if (_clients.empty())
    {
      return;
    }

    bool hadInput = false;
    bool makesVisible = false;
    auto pads = _input.GetGamepads();
    for (uint32 slot = 0; slot < pads.size(); ++slot)
    {
      auto &pad = pads[slot];
      auto &gamepad = _gamepads[slot];
      if (pad.Connected() && !gamepad)
      {
        Connect(slot);
        for (auto *client : List<boo::GamepadProviderClient *>(_clients))
        {
          client->PlatformGamepadConnected(*_gamepads[slot], boo::EventMakesGamepadsVisible::Yes);
        }
        continue;
      }
      if (!pad.Connected())
      {
        if (gamepad)
        {
          Disconnect(slot);
        }
        continue;
      }

      if (gamepad->Update(pad, _now))
      {
        hadInput = true;
        makesVisible = makesVisible || ButtonChanged(pad);
      }
      UpdateEffect(*gamepad);
    }

    if (hadInput)
    {
      if (makesVisible)
      {
        SetShouldMakeGamepadsVisible();
      }
      DispatchPlatformGamepadInputActivity();
    }
  }

  void GamepadProvider::StartMonitoringGamepads(boo::GamepadProviderClient &client) noexcept
  {
    bool shouldStart = _clients.empty();
    krys_debug_assert(std::ranges::find(_clients, &client) == _clients.end());
    _clients.push_back(&client);
    if (!shouldStart)
    {
      return;
    }

    // The pads already there connect without making the pads visible.
    auto pads = _input.GetGamepads();
    _gamepads.resize(pads.size());
    _platformGamepads.resize(pads.size());
    for (uint32 slot = 0; slot < pads.size(); ++slot)
    {
      if (pads[slot].Connected())
      {
        Connect(slot);
        client.PlatformGamepadConnected(*_gamepads[slot], boo::EventMakesGamepadsVisible::No);
      }
    }
  }

  void GamepadProvider::StopMonitoringGamepads(boo::GamepadProviderClient &client) noexcept
  {
    krys_debug_assert(std::ranges::find(_clients, &client) != _clients.end());
    std::erase(_clients, &client);
    if (!_clients.empty())
    {
      return;
    }

    for (auto &gamepad : std::exchange(_gamepads, {}))
    {
      if (gamepad && gamepad->CurrentEffect)
      {
        (void)_input.SetGamepadRumble(gamepad->Index(), 0, 0);
        auto effect = std::exchange(gamepad->CurrentEffect, null);
        if (effect->CompletionHandler)
        {
          effect->CompletionHandler(false);
        }
      }
    }
    _platformGamepads.clear();
  }

  const List<WeakPtr<boo::PlatformGamepad>> &GamepadProvider::PlatformGamepads() noexcept
  {
    return _platformGamepads;
  }

  void GamepadProvider::PlayEffect(uint32 gamepadIndex, utf8_stringview gamepadId,
                                   boo::GamepadHapticEffectType,
                                   const boo::GamepadEffectParameters &parameters,
                                   Func<void(bool)> &&completionHandler) noexcept
  {
    auto *gamepad = GamepadFor(gamepadIndex, gamepadId);
    if (!gamepad)
    {
      completionHandler(false);
      return;
    }

    // A new effect takes the place of the one playing.
    if (auto previous = std::exchange(gamepad->CurrentEffect, null); previous && previous->CompletionHandler)
    {
      previous->CompletionHandler(false);
    }

    auto start = _now
                 + std::chrono::duration_cast<MonotonicTime::duration>(
                   std::chrono::duration<double, std::milli>(parameters.StartDelay));
    auto end = start
               + std::chrono::duration_cast<MonotonicTime::duration>(
                 std::chrono::duration<double, std::milli>(parameters.Duration));
    gamepad->CurrentEffect = Gamepad::Effect {start,
                                              end,
                                              static_cast<float>(parameters.StrongMagnitude),
                                              static_cast<float>(parameters.WeakMagnitude),
                                              false,
                                              krys::move(completionHandler)};
    UpdateEffect(*gamepad);
  }

  void GamepadProvider::StopEffects(uint32 gamepadIndex, utf8_stringview gamepadId,
                                    Func<void()> &&completionHandler) noexcept
  {
    if (auto *gamepad = GamepadFor(gamepadIndex, gamepadId))
    {
      if (auto effect = std::exchange(gamepad->CurrentEffect, null))
      {
        if (effect->Started)
        {
          (void)_input.SetGamepadRumble(gamepadIndex, 0, 0);
        }
        if (effect->CompletionHandler)
        {
          effect->CompletionHandler(false);
        }
      }
    }
    completionHandler();
  }

  GamepadProvider::Gamepad *GamepadProvider::GamepadFor(uint32 index, utf8_stringview id) const noexcept
  {
    if (index >= _gamepads.size())
    {
      return nullptr;
    }
    auto *gamepad = _gamepads[index].get();
    return gamepad && gamepad->Id() == id ? gamepad : nullptr;
  }

  void GamepadProvider::Connect(uint32 slot) noexcept
  {
    auto gamepad = CreateUnique<Gamepad>(_input.GetGamepads()[slot], _now);
    _platformGamepads[slot] = CreateWeakPtr<boo::PlatformGamepad>(gamepad.get());
    _gamepads[slot] = krys::move(gamepad);
  }

  void GamepadProvider::Disconnect(uint32 slot) noexcept
  {
    auto gamepad = std::exchange(_gamepads[slot], nullptr);
    _platformGamepads[slot] = nullptr;
    if (auto effect = std::exchange(gamepad->CurrentEffect, null); effect && effect->CompletionHandler)
    {
      effect->CompletionHandler(false);
    }
    for (auto *client : List<boo::GamepadProviderClient *>(_clients))
    {
      client->PlatformGamepadDisconnected(*gamepad);
    }
  }

  void GamepadProvider::UpdateEffect(Gamepad &gamepad) noexcept
  {
    auto &effect = gamepad.CurrentEffect;
    if (!effect)
    {
      return;
    }

    if (_now >= effect->End)
    {
      if (effect->Started)
      {
        (void)_input.SetGamepadRumble(gamepad.Index(), 0, 0);
      }
      auto finished = std::exchange(effect, null);
      if (finished->CompletionHandler)
      {
        finished->CompletionHandler(true);
      }
      return;
    }

    if (!effect->Started && _now >= effect->Start)
    {
      effect->Started = true;
      (void)_input.SetGamepadRumble(gamepad.Index(), effect->Strong, effect->Weak);
    }
  }
}
