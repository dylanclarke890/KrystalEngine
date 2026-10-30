#include "Krystal.Booey/Modules/Gamepad/Gamepad.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Modules/Gamepad/GamepadButton.hpp"
#include "Krystal.Booey/Modules/Gamepad/GamepadEffectParameters.hpp"
#include "Krystal.Booey/Modules/Gamepad/GamepadEvent.hpp"
#include "Krystal.Booey/Modules/Gamepad/GamepadHapticActuator.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/Navigator.hpp"
#include "Krystal.Booey/Platform/Gamepad/GamepadConstants.hpp"
#include "Krystal.Booey/Platform/Gamepad/GamepadProvider.hpp"
#include "Krystal.Booey/Platform/Gamepad/GamepadProviderClient.hpp"
#include "Krystal.Booey/Platform/Gamepad/PlatformGamepad.hpp"
#include <algorithm>
#include <chrono>
#include <string>

namespace krys::boo::tests
{
  namespace
  {
    using namespace std::chrono_literals;

    /// @brief A standard pad with 17 buttons and 4 axes, with rumble motors if asked.
    class FakeGamepad final : public PlatformGamepad
    {
      KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(FakeGamepad);

    public:
      List<double> Axes = List<double>(4, 0.0);
      List<double> Buttons = List<double>(17, 0.0);

      FakeGamepad(uint32 index, bool rumble, MonotonicTime::duration now) noexcept : PlatformGamepad(index)
      {
        _id = u8"Fake pad";
        _mapping = utf8_string(StandardGamepadMapping);
        _connectTime = now;
        _lastUpdateTime = now;
        if (rumble)
        {
          _supportedEffectTypes.Add(GamepadHapticEffectType::DualRumble);
        }
      }

      KRYS_NODISCARD const List<double> &AxisValues() const noexcept final
      {
        return Axes;
      }

      KRYS_NODISCARD const List<double> &ButtonValues() const noexcept final
      {
        return Buttons;
      }

      void Updated(MonotonicTime::duration time) noexcept
      {
        _lastUpdateTime = time;
      }
    };

    /// @brief Pads the test connects, moves and disconnects, recording the effects it's asked to play.
    class FakeProvider final : public GamepadProvider
    {
    public:
      List<UniquePtr<FakeGamepad>> Owned;
      List<WeakPtr<PlatformGamepad>> Pads;
      List<GamepadEffectParameters> Played;
      uint32 Stops {0};
      Func<void(bool)> PendingCompletion;

      KRYS_NODISCARD bool Monitoring() const noexcept
      {
        return !_clients.empty();
      }

      void StartMonitoringGamepads(GamepadProviderClient &client) noexcept final
      {
        _clients.push_back(&client);
      }

      void StopMonitoringGamepads(GamepadProviderClient &client) noexcept final
      {
        std::erase(_clients, &client);
      }

      KRYS_NODISCARD const List<WeakPtr<PlatformGamepad>> &PlatformGamepads() noexcept final
      {
        return Pads;
      }

      void PlayEffect(uint32, utf8_stringview, GamepadHapticEffectType,
                      const GamepadEffectParameters &parameters,
                      Func<void(bool)> &&completionHandler) noexcept final
      {
        Played.push_back(parameters);
        if (PendingCompletion)
        {
          std::exchange(PendingCompletion, nullptr)(false);
        }
        PendingCompletion = krys::move(completionHandler);
      }

      void StopEffects(uint32, utf8_stringview, Func<void()> &&completionHandler) noexcept final
      {
        ++Stops;
        if (PendingCompletion)
        {
          std::exchange(PendingCompletion, nullptr)(false);
        }
        completionHandler();
      }

      FakeGamepad &Connect(uint32 index, EventMakesGamepadsVisible visibility, bool rumble = true) noexcept
      {
        auto pad = CreateUnique<FakeGamepad>(index, rumble, html::EventLoop::Current().Now());
        auto &result = *pad;
        if (Pads.size() <= index)
        {
          Pads.resize(index + 1);
        }
        Pads[index] = CreateWeakPtr<PlatformGamepad>(pad.get());
        Owned.push_back(krys::move(pad));
        for (auto *client : List<GamepadProviderClient *>(_clients))
        {
          client->PlatformGamepadConnected(result, visibility);
        }
        return result;
      }

      void Disconnect(uint32 index) noexcept
      {
        auto *pad = Pads[index].get();
        for (auto *client : List<GamepadProviderClient *>(_clients))
        {
          client->PlatformGamepadDisconnected(*pad);
        }
        Pads[index] = nullptr;
        std::erase_if(Owned, [&](const UniquePtr<FakeGamepad> &owned) { return owned.get() == pad; });
      }

      /// @brief Presses (or releases, with 0) @p button of the pad at @p index, which makes the pads visible.
      void Press(uint32 index, size_t button, double value = 1) noexcept
      {
        auto &pad = static_cast<FakeGamepad &>(*Pads[index].get());
        pad.Buttons[button] = value;
        SetShouldMakeGamepadsVisible();
        DispatchPlatformGamepadInputActivity();
      }

      /// @brief Moves @p axis of the pad at @p index, which doesn't make the pads visible.
      void Move(uint32 index, size_t axis, double value) noexcept
      {
        auto &pad = static_cast<FakeGamepad &>(*Pads[index].get());
        pad.Axes[axis] = value;
        DispatchPlatformGamepadInputActivity();
      }
    };

    /// @brief Makes the fake the shared provider for the test's duration.
    struct ProviderScope
    {
      FakeProvider Provider;
      GamepadProvider &Previous {GamepadProvider::Singleton()};

      ProviderScope() noexcept
      {
        GamepadProvider::SetSharedProvider(Provider);
      }

      ~ProviderScope() noexcept
      {
        GamepadProvider::SetSharedProvider(Previous);
      }
    };

    struct GamepadFixture : ProviderScope, PaintFixture
    {
      GamepadFixture() noexcept : PaintFixture(u8"<!DOCTYPE html>", {200, 100}, true)
      {
      }

      KRYS_NODISCARD LocalDOMWindow &Window() const noexcept
      {
        return *Document->Window();
      }

      KRYS_NODISCARD const List<RefPtr<Gamepad>> &Gamepads() const noexcept
      {
        return Window().GetNavigator().GetGamepads();
      }

      /// @brief Records the window's gamepad events as `type index connected`.
      KRYS_NODISCARD dom::EventListenerHandle Listen(dom::DOMStringAtom type, List<std::string> &log) noexcept
      {
        return Window().AddEventListener<GamepadEvent>(
          type,
          [&log](GamepadEvent &event)
          {
            auto *gamepad = event.Gamepad();
            log.push_back(std::string(event.Type().View().begin(), event.Type().View().end()) + " "
                          + std::to_string(gamepad->Index())
                          + (gamepad->Connected() ? " connected" : " gone"));
          });
      }
    };

    KRYS_NODISCARD std::string ResultName(const dom::ExceptionOr<GamepadHapticsResult> &result)
    {
      if (result.HasException())
      {
        return result.GetException().Code() == dom::ExceptionCode::TypeError ? "TypeError" : "exception";
      }
      return result.Value() == GamepadHapticsResult::Complete ? "complete" : "preempted";
    }
  }

  TEST_CASE("Gamepad: pads appear after a button press", "[Modules][Gamepad]")
  {
    GamepadFixture fixture;
    auto &provider = fixture.Provider;

    // Looking at the gamepads starts monitoring them; there are none to see yet.
    CHECK_FALSE(provider.Monitoring());
    CHECK(fixture.Gamepads().empty());
    CHECK(provider.Monitoring());

    // A pad that was already connected, and moving its stick, don't show it.
    auto &pad = provider.Connect(0, EventMakesGamepadsVisible::No);
    provider.Move(0, 1, 0.5);
    CHECK(fixture.Gamepads().empty());

    // Pressing a button shows every pad, with the standard mapping.
    provider.Press(0, 0);
    REQUIRE(fixture.Gamepads().size() == 1);
    Ref<Gamepad> gamepad = ShareRef(*fixture.Gamepads()[0]);
    CHECK(gamepad->Id() == u8"Fake pad");
    CHECK(gamepad->Index() == 0);
    CHECK(gamepad->Mapping() == u8"standard");
    CHECK(gamepad->Connected());
    REQUIRE(gamepad->Buttons().size() == 17);
    REQUIRE(gamepad->Axes().size() == 4);

    // Each call takes the pad's current values into the same objects.
    CHECK(gamepad->Buttons()[0]->Pressed());
    CHECK(gamepad->Buttons()[0]->Value() == 1);
    CHECK(gamepad->Axes()[1] == 0.5);
    pad.Buttons[0] = 0.05;
    pad.Buttons[7] = 0.5;
    pad.Axes[1] = -1;
    pad.Updated(fixture.Window().TimeOrigin() + 25ms);
    CHECK(fixture.Gamepads()[0].get() == gamepad.get());
    CHECK_FALSE(gamepad->Buttons()[0]->Pressed());
    CHECK(gamepad->Buttons()[0]->Value() == 0.05);
    CHECK(gamepad->Buttons()[7]->Pressed());
    CHECK(gamepad->Axes()[1] == -1);
    CHECK(gamepad->Timestamp() == 25.0);

    // A pad connecting later appears at its index.
    provider.Connect(1, EventMakesGamepadsVisible::Yes, false);
    REQUIRE(fixture.Gamepads().size() == 2);
    CHECK(fixture.Gamepads()[1]->Index() == 1);
    CHECK(fixture.Gamepads()[1]->VibrationActuator() == nullptr);
    CHECK(fixture.Gamepads()[0]->VibrationActuator() != nullptr);

    // A pad that disconnects leaves a hole, and its object says it's gone.
    provider.Disconnect(0);
    REQUIRE(fixture.Gamepads().size() == 2);
    CHECK(fixture.Gamepads()[0] == nullptr);
    CHECK_FALSE(gamepad->Connected());
    provider.Disconnect(1);
  }

  TEST_CASE("Gamepad: connection events", "[Modules][Gamepad]")
  {
    GamepadFixture fixture;
    auto &provider = fixture.Provider;
    List<std::string> log;

    {
      // A listener registers the window, which starts monitoring the pads.
      auto connected = fixture.Listen(dom::DOMStringAtom(EventNames::GamepadConnected), log);
      auto disconnected = fixture.Listen(dom::DOMStringAtom(EventNames::GamepadDisconnected), log);
      CHECK(provider.Monitoring());

      // A pad already there doesn't fire, and nor does its going while the window can't see the pads.
      provider.Connect(0, EventMakesGamepadsVisible::No);
      provider.Disconnect(0);
      CHECK(log.empty());

      // A pad that connects later fires at the window, with the pad, and shows them all.
      provider.Connect(1, EventMakesGamepadsVisible::No);
      provider.Connect(0, EventMakesGamepadsVisible::Yes);
      CHECK(log == List<std::string> {"gamepadconnected 1 connected", "gamepadconnected 0 connected"});
      CHECK(fixture.Gamepads().size() == 2);
      CHECK(fixture.Document->Window()->HasTransientActivation());

      // Using a pad that's already visible fires nothing more.
      log.clear();
      provider.Press(1, 3);
      CHECK(log.empty());

      // A pad going fires with the pad, no longer connected.
      provider.Disconnect(1);
      CHECK(log == List<std::string> {"gamepaddisconnected 1 gone"});

      // The event handler attributes are gamepad listeners too.
      log.clear();
      fixture.Window().SetOnGamepadConnected(
        [&log](dom::Event &event)
        { log.push_back("handler " + std::to_string(Downcast<GamepadEvent>(event).Gamepad()->Index())); });
      provider.Connect(2, EventMakesGamepadsVisible::Yes);
      CHECK(log == List<std::string> {"gamepadconnected 2 connected", "handler 2"});
      fixture.Window().SetOnGamepadConnected(nullptr);
      provider.Disconnect(2);
      provider.Disconnect(0);
    }

    // Removing the listeners unregisters the window; the navigator still looks.
    CHECK(provider.Monitoring());

    // An event made by the page.
    GamepadEventInit init;
    init.Bubbles = true;
    auto event = GamepadEvent::Create(dom::DOMStringAtom(EventNames::GamepadConnected), init);
    CHECK(event->Gamepad() == nullptr);
    CHECK(event->Bubbles());
    CHECK_FALSE(event->IsTrusted());
  }

  TEST_CASE("Gamepad: a window's listeners alone monitor the pads", "[Modules][Gamepad]")
  {
    ProviderScope scope;
    auto &provider = scope.Provider;
    {
      PaintFixture fixture(u8"<!DOCTYPE html>", {200, 100}, true);
      List<std::string> log;
      {
        auto handle =
          fixture.Document->Window()->AddEventListener(dom::DOMStringAtom(EventNames::GamepadDisconnected),
                                                       [&log](dom::Event &) { log.push_back("gone"); });
        CHECK(provider.Monitoring());
      }
      // The window's navigator was made for it, and keeps looking until the window goes.
      CHECK(provider.Monitoring());
      CHECK(fixture.Document->Window()->NavigatorIfExists());
    }
  }

  TEST_CASE("Gamepad: haptics", "[Modules][Gamepad]")
  {
    GamepadFixture fixture;
    auto &provider = fixture.Provider;
    (void)fixture.Gamepads();
    provider.Connect(0, EventMakesGamepadsVisible::No);
    provider.Press(0, 0);
    REQUIRE(fixture.Gamepads().size() == 1);
    auto *actuator = fixture.Gamepads()[0]->VibrationActuator();
    REQUIRE(actuator);
    CHECK(actuator->GetType() == GamepadHapticActuator::Type::DualRumble);
    CHECK(actuator->CanPlayEffectType(GamepadHapticEffectType::DualRumble));

    List<std::string> results;
    auto record = [&results](utf8_stringview name)
    {
      return [&results, name](dom::ExceptionOr<GamepadHapticsResult> &&result)
      {
        results.push_back(std::string(name.begin(), name.end()) + " " + ResultName(result));
      };
    };
    auto runTasks = []
    {
      html::EventLoop::Current().RunUntilIdle();
    };

    // Out of range parameters are a TypeError.
    actuator->PlayEffect(GamepadHapticEffectType::DualRumble, {.StrongMagnitude = 2}, record(u8"strong"));
    actuator->PlayEffect(GamepadHapticEffectType::DualRumble, {.Duration = -1}, record(u8"duration"));
    runTasks();
    CHECK(results == List<std::string> {"strong TypeError", "duration TypeError"});
    CHECK(provider.Played.empty());

    // An effect reaches the provider, its duration capped at 5s, and completes when the provider says so.
    results.clear();
    actuator->PlayEffect(GamepadHapticEffectType::DualRumble,
                         {.Duration = 10'000, .StrongMagnitude = 1, .WeakMagnitude = 0.5}, record(u8"first"));
    REQUIRE(provider.Played.size() == 1);
    CHECK(provider.Played[0].Duration == 5'000);
    CHECK(provider.Played[0].StrongMagnitude == 1);
    CHECK(provider.Played[0].WeakMagnitude == 0.5);
    runTasks();
    CHECK(results.empty());
    std::exchange(provider.PendingCompletion, nullptr)(true);
    runTasks();
    CHECK(results == List<std::string> {"first complete"});

    // A new effect preempts the one playing.
    results.clear();
    actuator->PlayEffect(GamepadHapticEffectType::DualRumble, {.Duration = 100}, record(u8"second"));
    actuator->PlayEffect(GamepadHapticEffectType::DualRumble, {.Duration = 100}, record(u8"third"));
    runTasks();
    CHECK(results == List<std::string> {"second preempted"});

    // Resetting stops the effect playing, which is preempted, and completes.
    results.clear();
    actuator->Reset(record(u8"reset"));
    runTasks();
    CHECK(provider.Stops == 1);
    CHECK(results == List<std::string> {"third preempted", "reset complete"});

    // Stopping the document's active objects stops the effect playing.
    results.clear();
    actuator->PlayEffect(GamepadHapticEffectType::DualRumble, {.Duration = 100}, record(u8"fourth"));
    fixture.Document->SuspendActiveDOMObjects(dom::ReasonForSuspension::BackForwardCache);
    CHECK(provider.Stops == 2);
    fixture.Document->ResumeActiveDOMObjects(dom::ReasonForSuspension::BackForwardCache);
    runTasks();
    CHECK(results == List<std::string> {"fourth preempted"});
    provider.Disconnect(0);
  }
}
