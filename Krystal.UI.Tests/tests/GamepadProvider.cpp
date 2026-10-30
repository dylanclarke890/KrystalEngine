#include "Krystal.UI/GamepadProvider.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Modules/Gamepad/GamepadEffectParameters.hpp"
#include "Krystal.Booey/Platform/Gamepad/GamepadProviderClient.hpp"
#include "Krystal.Booey/Platform/Gamepad/PlatformGamepad.hpp"
#include "Krystal.Core/Events/EventManager.hpp"
#include "Krystal.PAL/Gamepad.hpp"
#include "Krystal.PAL/Input.hpp"
#include <chrono>
#include <string>

namespace krys::ui::tests
{
  namespace
  {
    using namespace std::chrono_literals;

    /// @brief Two slots whose pads the test plugs in and moves, recording the rumble it's asked for.
    struct FakeGamepads : pal::GamepadBackend
    {
      Maybe<pal::GamepadReading> Slots[2];
      float Strong {0};
      float Weak {0};

      KRYS_NODISCARD uint32 SlotCount() const noexcept override
      {
        return 2;
      }

      KRYS_NODISCARD Maybe<pal::GamepadReading> Read(uint32 slot) noexcept override
      {
        return Slots[slot];
      }

      KRYS_NODISCARD utf8_string Name(uint32 slot) const noexcept override
      {
        return slot == 0 ? u8"first" : u8"second";
      }

      bool SetRumble(uint32, float strong, float weak) noexcept override
      {
        Strong = strong;
        Weak = weak;
        return true;
      }
    };

    /// @brief Records what the provider tells it as `connected 0 visible`, `disconnected 1`, `input hidden`.
    struct RecordingClient final : boo::GamepadProviderClient
    {
      List<std::string> Log;

      KRYS_NODISCARD static std::string Visibility(boo::EventMakesGamepadsVisible visibility)
      {
        return visibility == boo::EventMakesGamepadsVisible::Yes ? "visible" : "hidden";
      }

      void PlatformGamepadConnected(boo::PlatformGamepad &gamepad,
                                    boo::EventMakesGamepadsVisible visibility) noexcept final
      {
        Log.push_back("connected " + std::to_string(gamepad.Index()) + " " + Visibility(visibility));
      }

      void PlatformGamepadDisconnected(boo::PlatformGamepad &gamepad) noexcept final
      {
        Log.push_back("disconnected " + std::to_string(gamepad.Index()));
      }

      void PlatformGamepadInputActivity(boo::EventMakesGamepadsVisible visibility) noexcept final
      {
        Log.push_back("input " + Visibility(visibility));
      }
    };

    void Set(pal::GamepadReading &reading, pal::GamepadButton button, float value) noexcept
    {
      reading.Buttons[static_cast<size_t>(button)] = value;
      ++reading.PacketNumber;
    }
  }

  TEST_CASE("GamepadProvider: pads from the platform layer", "[GamepadProvider]")
  {
    EventManager events;
    auto backend = CreateUnique<FakeGamepads>();
    auto &fake = *backend;
    fake.Slots[0] = pal::GamepadReading {};
    pal::Input input(&events, krys::move(backend));
    GamepadProvider provider(input);
    RecordingClient client;
    auto now = MonotonicTime::Now();
    auto frame = [&](MonotonicTime::duration time)
    {
      client.Log.clear();
      input.BeginFrame();
      events.DispatchAll();
      provider.Update(time);
    };

    // Nothing is read until a client monitors the pads; then the pad already there connects, hidden.
    frame(now);
    CHECK(provider.PlatformGamepads().empty());
    provider.StartMonitoringGamepads(client);
    CHECK(client.Log == List<std::string> {"connected 0 hidden"});
    REQUIRE(provider.PlatformGamepads().size() == 2);
    CHECK(provider.PlatformGamepads()[1].get() == nullptr);
    auto *gamepad = provider.PlatformGamepads()[0].get();
    REQUIRE(gamepad);
    CHECK(gamepad->Index() == 0);
    CHECK(gamepad->Id() == u8"first");
    CHECK(gamepad->Mapping() == u8"standard");
    CHECK(gamepad->ButtonValues().size() == 17);
    CHECK(gamepad->AxisValues().size() == 4);
    CHECK(gamepad->SupportedEffectTypes().Contains(boo::GamepadHapticEffectType::DualRumble));

    // A frame without changes says nothing; moving a stick is input that doesn't show the pads.
    frame(now);
    CHECK(client.Log.empty());
    auto &reading = *fake.Slots[0];
    reading.Axes[static_cast<size_t>(pal::GamepadAxis::RightX)] = -0.5f;
    ++reading.PacketNumber;
    frame(now);
    CHECK(client.Log == List<std::string> {"input hidden"});
    CHECK(gamepad->AxisValues()[2] == -0.5);

    // Pressing a button (or releasing it) shows them, in the standard layout's order.
    Set(reading, pal::GamepadButton::DPadUp, 1);
    frame(now);
    CHECK(client.Log == List<std::string> {"input visible"});
    CHECK(gamepad->ButtonValues()[12] == 1);
    Set(reading, pal::GamepadButton::DPadUp, 0);
    frame(now);
    CHECK(client.Log == List<std::string> {"input visible"});

    // An effect rumbles at once, and completes (stopping the motors) when its duration is up.
    List<std::string> completions;
    auto completion = [&completions](std::string name)
    {
      return [&completions, name](bool success)
      {
        completions.push_back(name + (success ? " done" : " cut"));
      };
    };
    provider.PlayEffect(0, u8"first", boo::GamepadHapticEffectType::DualRumble,
                        {.Duration = 100, .StrongMagnitude = 1, .WeakMagnitude = 0.5}, completion("first"));
    CHECK(fake.Strong == 1);
    CHECK(fake.Weak == 0.5f);
    frame(now + 50ms);
    CHECK(completions.empty());
    frame(now + 100ms);
    CHECK(completions == List<std::string> {"first done"});
    CHECK(fake.Strong == 0);
    CHECK(fake.Weak == 0);

    // A delayed effect waits; a new one cuts it short.
    completions.clear();
    provider.PlayEffect(0, u8"first", boo::GamepadHapticEffectType::DualRumble,
                        {.Duration = 100, .StartDelay = 20, .StrongMagnitude = 0.25}, completion("delayed"));
    CHECK(fake.Strong == 0);
    frame(now + 120ms);
    CHECK(fake.Strong == 0.25f);
    provider.PlayEffect(0, u8"first", boo::GamepadHapticEffectType::DualRumble,
                        {.Duration = 100, .WeakMagnitude = 0.75}, completion("next"));
    CHECK(completions == List<std::string> {"delayed cut"});
    CHECK(fake.Weak == 0.75f);

    // Stopping cuts it short and stops the motors; an effect for a pad that isn't there fails.
    bool stopped = false;
    provider.StopEffects(0, u8"first", [&stopped] { stopped = true; });
    CHECK(stopped);
    CHECK(completions == List<std::string> {"delayed cut", "next cut"});
    CHECK(fake.Weak == 0);
    provider.PlayEffect(0, u8"second", boo::GamepadHapticEffectType::DualRumble, {.Duration = 100},
                        completion("wrong"));
    provider.PlayEffect(1, u8"second", boo::GamepadHapticEffectType::DualRumble, {.Duration = 100},
                        completion("missing"));
    CHECK(completions == List<std::string> {"delayed cut", "next cut", "wrong cut", "missing cut"});

    // A change is stamped with the host's time of the frame that saw it; a frame without one keeps it.
    reading.Axes[static_cast<size_t>(pal::GamepadAxis::LeftX)] = 0.25f;
    ++reading.PacketNumber;
    frame(now + 150ms);
    CHECK(gamepad->LastUpdateTime() == now + 150ms);
    frame(now + 160ms);
    CHECK(gamepad->LastUpdateTime() == now + 150ms);

    // A pad that goes away disconnects; when no one monitors, the pads are dropped.
    fake.Slots[0] = null;
    frame(now + 200ms);
    CHECK(client.Log == List<std::string> {"disconnected 0"});
    CHECK(provider.PlatformGamepads()[0].get() == nullptr);
    provider.StopMonitoringGamepads(client);
    CHECK(provider.PlatformGamepads().empty());
  }

  TEST_CASE("GamepadProvider: pads connecting while monitored", "[GamepadProvider]")
  {
    EventManager events;
    auto backend = CreateUnique<FakeGamepads>();
    auto &fake = *backend;
    fake.Slots[0] = pal::GamepadReading {};
    fake.Slots[1] = pal::GamepadReading {};
    pal::Input input(&events, krys::move(backend));
    GamepadProvider provider(input);
    RecordingClient client;
    auto frame = [&]
    {
      client.Log.clear();
      input.BeginFrame();
      events.DispatchAll();
      provider.Update(MonotonicTime::Now());
    };

    // The input hasn't read the slots yet, so the pads connect on the next frame, which shows them.
    provider.StartMonitoringGamepads(client);
    CHECK(client.Log.empty());
    frame();
    CHECK(client.Log == List<std::string> {"connected 0 visible", "connected 1 visible"});

    // An effect playing as its pad goes is cut short.
    bool cut = false;
    provider.PlayEffect(1, u8"second", boo::GamepadHapticEffectType::DualRumble, {.Duration = 1'000},
                        [&cut](bool success) { cut = !success; });
    fake.Slots[1] = null;
    frame();
    CHECK(client.Log == List<std::string> {"disconnected 1"});
    CHECK(cut);
    provider.StopMonitoringGamepads(client);
  }
}
