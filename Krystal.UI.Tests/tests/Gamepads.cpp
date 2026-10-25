#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Events/EventManager.hpp"
#include "Krystal.PAL/Events.hpp"
#include "Krystal.PAL/Gamepad.hpp"
#include "Krystal.PAL/Input.hpp"
#include <string>

namespace krys::ui::tests
{
  namespace
  {
    /// @brief Two slots whose pads the test plugs in and moves.
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

    void Set(pal::GamepadReading &reading, pal::GamepadButton button, float value) noexcept
    {
      reading.Buttons[static_cast<size_t>(button)] = value;
      ++reading.PacketNumber;
    }
  }

  TEST_CASE("Input: gamepads", "[PAL]")
  {
    EventManager events;
    List<std::string> log;
    events.On<pal::GamepadConnectionEvent>(
      [&](const pal::GamepadConnectionEvent &event)
      {
        log.push_back(std::string(event.Connected() ? "connected " : "disconnected ")
                      + std::to_string(event.Slot()));
        return true;
      });
    events.On<pal::GamepadButtonEvent>(
      [&](const pal::GamepadButtonEvent &event)
      {
        log.push_back(
          std::string(event.State() == pal::GamepadButtonState::Pressed ? "pressed " : "released ")
          + std::to_string(static_cast<int>(event.Button())) + " on " + std::to_string(event.Slot()));
        return true;
      });
    pal::Input input(&events, CreateUnique<FakeGamepads>());
    auto frame = [&]
    {
      log.clear();
      input.BeginFrame();
      events.DispatchAll();
    };

    REQUIRE(input.GetGamepads().size() == 2);
    frame();
    CHECK(log.empty());
    CHECK_FALSE(input.GetGamepads()[0].Connected());
    CHECK_FALSE(input.SetGamepadRumble(0, 1, 1));

    // A pad already there when the slots are first read connects.
    auto secondBackend = CreateUnique<FakeGamepads>();
    auto &secondFake = *secondBackend;
    secondFake.Slots[1] = pal::GamepadReading {};
    pal::Input second(&events, krys::move(secondBackend));
    log.clear();
    second.BeginFrame();
    events.DispatchAll();
    CHECK(log == List<std::string> {"connected 1"});
    CHECK(second.GetGamepads()[1].Connected());
    CHECK(second.GetGamepads()[1].Name() == u8"second");
    CHECK(second.GetGamepads()[1].Slot() == 1);

    // A connected pad's buttons and axes, frame by frame.
    auto &pad = second.GetGamepads()[1];
    auto &reading = *secondFake.Slots[1];
    Set(reading, pal::GamepadButton::South, 1);
    reading.Axes[static_cast<size_t>(pal::GamepadAxis::LeftY)] = 0.75f;
    log.clear();
    second.BeginFrame();
    events.DispatchAll();
    CHECK(log == List<std::string> {"pressed 0 on 1"});
    CHECK(pad.IsButtonPressed(pal::GamepadButton::South));
    CHECK(pad.IsButtonHeld(pal::GamepadButton::South));
    CHECK(pad.ButtonValue(pal::GamepadButton::South) == 1);
    CHECK(pad.AxisValue(pal::GamepadAxis::LeftY) == 0.75f);

    log.clear();
    second.BeginFrame();
    events.DispatchAll();
    CHECK(log.empty());
    CHECK_FALSE(pad.IsButtonPressed(pal::GamepadButton::South));
    CHECK(pad.IsButtonHeld(pal::GamepadButton::South));

    // A trigger is pressed past its threshold, and has the value between.
    Set(reading, pal::GamepadButton::South, 0);
    Set(reading, pal::GamepadButton::RightTrigger, 0.1f);
    log.clear();
    second.BeginFrame();
    events.DispatchAll();
    CHECK(log == List<std::string> {"released 0 on 1"});
    CHECK(pad.WasButtonReleased(pal::GamepadButton::South));
    CHECK_FALSE(pad.IsButtonHeld(pal::GamepadButton::RightTrigger));
    CHECK(pad.ButtonValue(pal::GamepadButton::RightTrigger) == 0.1f);
    Set(reading, pal::GamepadButton::RightTrigger, 0.5f);
    log.clear();
    second.BeginFrame();
    events.DispatchAll();
    CHECK(log == List<std::string> {"pressed 7 on 1"});

    // Rumble reaches the backend.
    CHECK(second.SetGamepadRumble(1, 0.25f, 0.5f));
    CHECK(secondFake.Strong == 0.25f);
    CHECK(secondFake.Weak == 0.5f);

    // A pad that goes away lets go of its buttons without events.
    secondFake.Slots[1] = null;
    log.clear();
    second.BeginFrame();
    events.DispatchAll();
    CHECK(log == List<std::string> {"disconnected 1"});
    CHECK_FALSE(pad.Connected());
    CHECK_FALSE(pad.IsButtonHeld(pal::GamepadButton::RightTrigger));
  }
}
