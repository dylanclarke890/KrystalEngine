#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLDialogElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"
#include "Krystal.Booey/Loader/Cache/MemoryCache.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Core/Events/EventManager.hpp"
#include "Krystal.Editor/PageGamepads.hpp"
#include "Krystal.Gfx.Tests/Golden.hpp"
#include "Krystal.PAL/Gamepad.hpp"
#include "Krystal.PAL/Input.hpp"
#include "Krystal.UI.Tests/ViewTestUtils.hpp"
#include "Krystal.UI/GamepadProvider.hpp"
#include "Krystal.UI/PopupMenuOverlay.hpp"
#include "Krystal.UI/View.hpp"
#include <chrono>
#include <string>

namespace krys::ui::tests
{
  namespace
  {
    using namespace std::chrono_literals;

    /// @brief A slot whose pad the test plugs in and moves, recording the rumble it's asked for.
    struct FakeGamepad : pal::GamepadBackend
    {
      Maybe<pal::GamepadReading> Slot;
      float Strong {0};
      float Weak {0};

      KRYS_NODISCARD uint32 SlotCount() const noexcept override
      {
        return 1;
      }

      KRYS_NODISCARD Maybe<pal::GamepadReading> Read(uint32) noexcept override
      {
        return Slot;
      }

      KRYS_NODISCARD utf8_string Name(uint32) const noexcept override
      {
        return u8"pad";
      }

      bool SetRumble(uint32, float strong, float weak) noexcept override
      {
        Strong = strong;
        Weak = weak;
        return true;
      }
    };

    /// @brief Makes @p provider the shared gamepad provider for as long as it lives.
    class SharedProviderScope
    {
      KRYS_NON_COPY_MOVABLE(SharedProviderScope);

    private:
      boo::GamepadProvider &_previous;

    public:
      explicit SharedProviderScope(boo::GamepadProvider &provider) noexcept
          : _previous(boo::GamepadProvider::Singleton())
      {
        boo::GamepadProvider::SetSharedProvider(provider);
      }

      ~SharedProviderScope() noexcept
      {
        boo::GamepadProvider::SetSharedProvider(_previous);
      }
    };

    struct Counter : boo::dom::EventListenerCallback
    {
      int32 Count {0};

      void HandleEvent(boo::dom::Event &) noexcept override
      {
        ++Count;
      }
    };
  }

  TEST_CASE("View: the pad drives the page", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(
      u8"<!DOCTYPE html><style>body { margin: 0; font: 16px sans-serif } button, select { position: absolute;"
      u8" width: 50px; height: 30px; top: 10px }</style>"
      u8"<button id=r1 style='left: 10px'></button><button id=r2 style='left: 80px'></button>"
      u8"<button id=r3 style='left: 150px'></button><button id=r4 style='left: 220px'></button>"
      u8"<select id=s style='left: 10px; top: 100px'><option>Apple</option><option>Banana</option></select>"
      u8"<div id=list style='position: absolute; left: 200px; top: 100px; width: 100px; height: 60px;"
      u8" overflow: auto'><button id=inner style='position: static'></button><div style='height: 400px'>"
      u8"</div></div><dialog id=d><button id=inside style='position: static'></button></dialog>"
      u8"<div style='height: 2000px'></div>");
    WaitForLoad(view);
    view.UpdateRendering();
    auto byId = [&view](boo::dom::DOMStringView id)
    {
      auto element = view.Document().GetElementById(id);
      REQUIRE(element);
      return element;
    };
    auto focused = [&view]
    {
      return view.Document().FocusedElement();
    };
    auto focus = [&byId](boo::dom::DOMStringView id)
    {
      Downcast<boo::html::HTMLElement>(*byId(id)).Focus();
    };

    EventManager events;
    auto backend = CreateUnique<FakeGamepad>();
    auto &fake = *backend;
    fake.Slot = pal::GamepadReading {};
    pal::Input input(&events, krys::move(backend));
    auto &reading = *fake.Slot;
    auto start = MonotonicTime::Now();
    auto frame = [&](MonotonicTime::duration time)
    {
      input.BeginFrame();
      events.DispatchAll();
      view.GamepadInput(input.GetGamepads(), start + time);
    };
    auto set = [&](pal::GamepadButton button, float value)
    {
      reading.Buttons[static_cast<size_t>(button)] = value;
      ++reading.PacketNumber;
    };
    auto setAxis = [&](pal::GamepadAxis axis, float value)
    {
      reading.Axes[static_cast<size_t>(axis)] = value;
      ++reading.PacketNumber;
    };
    // Presses and releases a button over two frames.
    auto press = [&](pal::GamepadButton button, MonotonicTime::duration time = 0ms)
    {
      set(button, 1);
      frame(time);
      set(button, 0);
      frame(time);
    };
    frame(0ms);

    // The right bumper is Tab, as a user activation, which makes the focus visible; the left is Shift+Tab.
    CHECK_FALSE(view.Document().Window()->HasTransientActivation());
    press(pal::GamepadButton::RightShoulder);
    CHECK(view.Document().Window()->HasTransientActivation());
    CHECK(focused() == byId(u8"r1").get());
    CHECK(byId(u8"r1")->HasFocusVisible());
    press(pal::GamepadButton::RightShoulder);
    CHECK(focused() == byId(u8"r2").get());
    press(pal::GamepadButton::LeftShoulder);
    CHECK(focused() == byId(u8"r1").get());

    // The D-pad moves the focus spatially, though the page's setting is off.
    CHECK_FALSE(view.Document().GetSettings().SpatialNavigationEnabled);
    press(pal::GamepadButton::DPadRight);
    CHECK(focused() == byId(u8"r2").get());
    press(pal::GamepadButton::DPadLeft);
    CHECK(focused() == byId(u8"r1").get());
    press(pal::GamepadButton::DPadDown);
    CHECK(focused() == byId(u8"s").get());
    press(pal::GamepadButton::DPadUp);
    CHECK(focused() == byId(u8"r1").get());

    // The left stick moves it past its threshold, along its main axis; held, it moves again after a delay,
    // then repeatedly.
    setAxis(pal::GamepadAxis::LeftX, 0.4f);
    frame(1s);
    CHECK(focused() == byId(u8"r1").get());
    setAxis(pal::GamepadAxis::LeftX, 0.8f);
    setAxis(pal::GamepadAxis::LeftY, 0.3f);
    frame(1s);
    CHECK(focused() == byId(u8"r2").get());
    frame(1s + 499ms);
    CHECK(focused() == byId(u8"r2").get());
    frame(1s + 500ms);
    CHECK(focused() == byId(u8"r3").get());
    frame(1s + 550ms);
    CHECK(focused() == byId(u8"r3").get());
    frame(1s + 600ms);
    CHECK(focused() == byId(u8"r4").get());
    setAxis(pal::GamepadAxis::LeftX, 0);
    setAxis(pal::GamepadAxis::LeftY, 0);
    frame(2s);

    // A clicks the focused element.
    Ref<Counter> clicks = CreateRef<Counter>();
    byId(u8"r4")->AddEventListener(boo::dom::DOMStringAtom(u8"click"),
                                   Ref<boo::dom::EventListenerCallback> {clicks}, false);
    press(pal::GamepadButton::South, 2s);
    CHECK(clicks->Count == 1);

    // A opens a drop-down's popup, which the D-pad and A then drive.
    focus(u8"s");
    press(pal::GamepadButton::South, 2s);
    auto *popup = static_cast<PopupMenuOverlay *>(view.ShownOverlay());
    REQUIRE(popup);
    CHECK(popup->HighlightedIndex() == 0);
    press(pal::GamepadButton::DPadDown, 2s);
    CHECK(popup->HighlightedIndex() == 1);
    press(pal::GamepadButton::South, 2s);
    CHECK_FALSE(view.ShownOverlay());
    CHECK(Downcast<boo::html::HTMLSelectElement>(*byId(u8"s")).SelectedIndex() == 1);
    CHECK(focused() == byId(u8"s").get());

    // B is a close request: it closes a modal dialog.
    auto &dialog = Downcast<boo::html::HTMLDialogElement>(*byId(u8"d"));
    REQUIRE_FALSE(dialog.ShowModal().HasException());
    CHECK(focused() == byId(u8"inside").get());
    press(pal::GamepadButton::East, 2s);
    boo::html::EventLoop::Current().RunUntilIdle();
    CHECK_FALSE(dialog.Open());

    // The right stick scrolls the viewport for the time it's pushed, squared.
    focus(u8"r1");
    setAxis(pal::GamepadAxis::RightY, 1);
    frame(3s);
    CHECK(view.ScrollPosition() == boo::IntPoint());
    frame(3s + 50ms);
    CHECK(view.ScrollPosition() == boo::IntPoint(0, 75));
    // A long frame scrolls no more than a tenth of a second does.
    frame(4s);
    CHECK(view.ScrollPosition() == boo::IntPoint(0, 225));
    setAxis(pal::GamepadAxis::RightY, -0.5f);
    frame(4s + 100ms);
    CHECK(view.ScrollPosition() == boo::IntPoint(0, 188));
    setAxis(pal::GamepadAxis::RightY, 0);
    frame(5s);
    view.ScrollTo({});

    // In a box that scrolls, it scrolls the box.
    focus(u8"inner");
    setAxis(pal::GamepadAxis::RightY, 1);
    frame(6s);
    frame(6s + 20ms);
    CHECK(byId(u8"list")->ScrollTop() == 30);
    CHECK(view.ScrollPosition() == boo::IntPoint());
    setAxis(pal::GamepadAxis::RightY, 0);
    frame(7s);

    // Turned off, the pad leaves the page alone.
    view.SetGamepadNavigationEnabled(false);
    CHECK_FALSE(view.GamepadNavigationEnabled());
    press(pal::GamepadButton::RightShoulder, 7s);
    CHECK(focused() == byId(u8"inner").get());
    view.SetGamepadNavigationEnabled(true);

    // So it does while the window doesn't have focus.
    view.SetFocused(false);
    press(pal::GamepadButton::RightShoulder, 7s);
    CHECK(focused() == byId(u8"inner").get());
    view.SetFocused(true);
    press(pal::GamepadButton::RightShoulder, 7s);
    CHECK(focused() != byId(u8"inner").get());
  }
  TEST_CASE("View: the editor page's gamepads", "[UI]")
  {
    EventManager events;
    auto backend = CreateUnique<FakeGamepad>();
    auto &fake = *backend;
    fake.Slot = pal::GamepadReading {};
    pal::Input input(&events, krys::move(backend));
    auto &reading = *fake.Slot;
    GamepadProvider provider(input);
    SharedProviderScope providerScope(provider);
    // The editor's input has read the pads before the page loads.
    input.BeginFrame();
    events.DispatchAll();

    boo::MemoryCache::Singleton().EvictResources();
    View view({1'200, 600});
    view.SetNavigationHandler([](const NavigationRequest &) { return NavigationPolicy::Allow; });
    REQUIRE(
      view.LoadFile(gfx::tests::GoldenDirectory() / ".." / ".." / "Krystal.Editor" / "pages" / "index.html")
        .has_value());
    WaitForLoad(view);
    auto &document = view.Document();
    // As the editor makes it once the page has loaded.
    editor::PageGamepads gamepads(document);
    auto byId = [&](utf8_stringview id)
    {
      auto element = document.GetElementById(id);
      REQUIRE(element);
      return element;
    };
    auto narrow = [](const boo::dom::Node *node)
    {
      auto text = node ? node->TextContent().value_or(boo::dom::DOMString {}) : boo::dom::DOMString {};
      return std::string(text.begin(), text.end());
    };
    auto textOf = [&](utf8_stringview id)
    {
      return narrow(byId(id).get());
    };
    auto focusedText = [&]
    {
      return narrow(document.FocusedElement());
    };

    // The editor's frame: the pads reach the page's Gamepad API and drive the page, then the page runs.
    auto start = std::max(MonotonicTime::Now(), boo::html::EventLoop::Current().Now());
    auto frame = [&](MonotonicTime::duration time)
    {
      input.BeginFrame();
      events.DispatchAll();
      provider.Update(start + time);
      view.GamepadInput(input.GetGamepads(), start + time);
      (void)view.Update(start + time);
    };
    auto set = [&](pal::GamepadButton button, float value)
    {
      reading.Buttons[static_cast<size_t>(button)] = value;
      ++reading.PacketNumber;
    };
    auto press = [&](pal::GamepadButton button, MonotonicTime::duration time)
    {
      set(button, 1);
      frame(time);
      set(button, 0);
      frame(time + 16ms);
    };

    // The pad is there, but the page can't see it until a button is pressed.
    frame(0ms);
    CHECK(textOf(u8"gamepad-readout") == "Connect a pad and press a button on it to show it here.");
    set(pal::GamepadButton::North, 1);
    reading.Axes[static_cast<size_t>(pal::GamepadAxis::LeftX)] = -0.25f;
    frame(16ms);
    frame(32ms);
    CHECK(textOf(u8"gamepad-readout") == "Pad 0: pad (standard)\n  Pressed: Y\n  Axes: -0.25 0.00 0.00 0.00");
    set(pal::GamepadButton::North, 0);
    reading.Axes[static_cast<size_t>(pal::GamepadAxis::LeftX)] = 0;
    frame(48ms);
    frame(64ms);
    CHECK(textOf(u8"gamepad-readout")
          == "Pad 0: pad (standard)\n  Pressed: nothing\n  Axes: 0.00 0.00 0.00 0.00");

    // The grid, at the top of the view: the D-pad moves through it, and A presses.
    auto gridRect = byId(u8"nav-grid")->GetBoundingClientRect();
    view.ScrollTo(boo::IntPoint(0, view.ScrollPosition().Y() + static_cast<int32>(gridRect->Top()) - 20));
    view.UpdateRendering();
    auto espresso = document.QuerySelector(u8"#nav-grid button");
    REQUIRE_FALSE(espresso.HasException());
    REQUIRE(espresso.Value());
    Downcast<boo::html::HTMLElement>(*espresso.Value()).Focus();
    press(pal::GamepadButton::DPadRight, 100ms);
    CHECK(focusedText() == "Filter");
    press(pal::GamepadButton::South, 200ms);
    CHECK(textOf(u8"nav-log") == "Pressed “Filter”");

    // The arrow keys move through it too, and a radio's choice shows.
    CHECK(view.KeyPressed(pal::Key::DownArrow));
    CHECK(focusedText() == "Cold brew");
    CHECK(view.KeyPressed(pal::Key::DownArrow));
    REQUIRE(document.FocusedElement());
    CHECK(document.FocusedElement()->GetAttribute(u8"value") == u8"Almond");
    press(pal::GamepadButton::South, 300ms);
    CHECK(textOf(u8"nav-log") == "Chose “Almond”");

    // The rumble button rumbles the pad, until the effect ends.
    Downcast<boo::html::HTMLElement>(*byId(u8"rumble")).Focus();
    press(pal::GamepadButton::South, 400ms);
    CHECK(textOf(u8"rumble-log") == "Rumbling…");
    CHECK(fake.Strong == 1);
    CHECK(fake.Weak == 0.5f);
    frame(1'016ms);
    frame(1'032ms);
    CHECK(fake.Strong == 0);
    CHECK(textOf(u8"rumble-log") == "Pad 0: rumbled");

    // The pad going away leaves the readout empty again.
    fake.Slot = null;
    frame(2s);
    frame(2s + 16ms);
    CHECK(textOf(u8"gamepad-readout") == "Connect a pad and press a button on it to show it here.");
  }
}
