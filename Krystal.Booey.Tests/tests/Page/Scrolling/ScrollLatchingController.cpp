#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Animation/AnimationTestUtils.hpp"
#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Booey/DOM/Events/EventListener.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/Events/PlatformWheelEvent.hpp"
#include "Krystal.Core/Utils/ScopeExit.hpp"

namespace krys::boo::tests
{
  namespace
  {
    using Phase = PlatformWheelEventPhase;

    /// @brief A 100 pixel box fixed at the top left with 200 pixels to scroll, on a page with 900 pixels to
    /// scroll, and another element below the box.
    constexpr utf8_stringview LatchingPage =
      u8"<!DOCTYPE html><style>body{margin:0;height:1000px}"
      u8"#s{position:fixed;top:0;left:0;width:100px;height:100px;overflow:auto}</style>"
      u8"<div id=s><div id=a style=height:300px></div></div>"
      u8"<div id=other style=position:fixed;top:120px;left:0;width:100px;height:50px></div>";

    /// @brief A step of a touchpad's pan at @p position, with the content following @p deltaY.
    bool Pan(AnimationPageFixture &fixture, Phase phase, float deltaY, IntPoint position = {10, 10}) noexcept
    {
      return fixture.TestPage->MainFrame().GetEventHandler().HandleWheelEvent(
        PlatformWheelEvent(position, position, 0, deltaY, 0, 0,
                           PlatformWheelEventGranularity::ScrollByPixelWheelEvent, {}, {})
          .CopyAsTouchpadPan(phase, Phase::None));
    }

    KRYS_NODISCARD int32 ViewY(AnimationPageFixture &fixture) noexcept
    {
      return fixture.TestPage->MainFrame().View()->GetScrollPosition().Y();
    }

    struct WheelCounter : public dom::EventListenerCallback
    {
      int32 Count {0};

      void HandleEvent(dom::Event &) noexcept override
      {
        ++Count;
      }
    };

    /// @brief A listener that takes over the scrolling: it scrolls the box itself, and cancels the event.
    struct ScriptScroller : public dom::EventListenerCallback
    {
      dom::Element *Box {nullptr};

      void HandleEvent(dom::Event &event) noexcept override
      {
        Box->SetScrollTop(Box->ScrollTop() + 10);
        event.PreventDefault();
      }
    };
  }

  TEST_CASE("ScrollLatchingController: a touchpad's gesture keeps to the box it started over",
            "[Page][Scrolling][ScrollLatchingController]")
  {
    AnimationPageFixture fixture {utf8_string(LatchingPage)};
    auto &box = fixture.ById(u8"s");

    SECTION("it stops at the box's end, rather than going on to the view")
    {
      CHECK_FALSE(Pan(fixture, Phase::Began, 0));
      CHECK(Pan(fixture, Phase::Changed, -150));
      CHECK(Pan(fixture, Phase::Changed, -100));
      CHECK(box.ScrollTop() == 200);
      (void)Pan(fixture, Phase::Changed, -100);
      CHECK(ViewY(fixture) == 0);
      (void)Pan(fixture, Phase::Ended, 0);

      // The next gesture starts at the box's end, so the view takes it.
      (void)Pan(fixture, Phase::Began, 0);
      CHECK(Pan(fixture, Phase::Changed, -100));
      CHECK(box.ScrollTop() == 200);
      CHECK(ViewY(fixture) == 100);
    }

    SECTION("wherever the pointer goes")
    {
      (void)Pan(fixture, Phase::Began, 0);
      CHECK(Pan(fixture, Phase::Changed, -50, {150, 50}));
      CHECK(box.ScrollTop() == 50);
      CHECK(ViewY(fixture) == 0);
    }

    SECTION("a gesture that starts on the view stays there, though the box comes under the pointer")
    {
      box.SetScrollTop(200);
      (void)Pan(fixture, Phase::Began, 0);
      CHECK(Pan(fixture, Phase::Changed, -100));
      CHECK(ViewY(fixture) == 100);
      // Back up: the box could scroll that way, but the view keeps the gesture.
      CHECK(Pan(fixture, Phase::Changed, 50));
      CHECK(ViewY(fixture) == 50);
      CHECK(box.ScrollTop() == 200);
    }

    SECTION("its wheel events go to the element it started over")
    {
      auto counter = CreateRef<WheelCounter>();
      fixture.ById(u8"other").AddEventListener(dom::DOMStringAtom(u8"wheel"), counter, false);
      (void)Pan(fixture, Phase::Began, 0);
      (void)Pan(fixture, Phase::Changed, -50, {10, 150});
      CHECK(counter->Count == 0);
    }

    SECTION("a mouse wheel's notches don't latch")
    {
      // Where they go, so at once rather than smoothly.
      auto &settings = fixture.Document->GetSettings();
      settings.ScrollAnimatorEnabled = false;
      auto restoreSettings = MakeScopeExit([&] { settings.ScrollAnimatorEnabled = true; });
      box.SetScrollTop(200);
      auto &handler = fixture.TestPage->MainFrame().GetEventHandler();
      CHECK(handler.HandleWheelEvent(
        PlatformWheelEvent(IntPoint(10, 10), IntPoint(10, 10), 0, -100, 0, 1,
                           PlatformWheelEventGranularity::ScrollByPixelWheelEvent, {}, {})));
      CHECK(ViewY(fixture) == 100);
      CHECK(handler.HandleWheelEvent(
        PlatformWheelEvent(IntPoint(10, 10), IntPoint(10, 10), 0, 50, 0, -1,
                           PlatformWheelEventGranularity::ScrollByPixelWheelEvent, {}, {})));
      CHECK(box.ScrollTop() == 150);
      CHECK(ViewY(fixture) == 100);
    }
  }

  TEST_CASE("ScrollLatchingController: the latch ends", "[Page][Scrolling][ScrollLatchingController]")
  {
    AnimationPageFixture fixture {utf8_string(LatchingPage)};
    auto &box = fixture.ById(u8"s");
    (void)Pan(fixture, Phase::Began, 0);
    CHECK(Pan(fixture, Phase::Changed, -50));
    REQUIRE(box.ScrollTop() == 50);

    SECTION("when the element it started over leaves the document")
    {
      fixture.ById(u8"a").Remove();
      // Away from the box, the view scrolls.
      CHECK(Pan(fixture, Phase::Changed, -50, {150, 50}));
      CHECK(ViewY(fixture) == 50);
    }

    SECTION("when script scrolls the box and cancels the event")
    {
      auto scroller = CreateRef<ScriptScroller>();
      scroller->Box = &box;
      box.AddEventListener(dom::DOMStringAtom(u8"wheel"), scroller, false);
      CHECK(Pan(fixture, Phase::Changed, -50));
      CHECK(box.ScrollTop() == 60);
      box.RemoveEventListener(dom::DOMStringAtom(u8"wheel"), scroller, false);

      CHECK(Pan(fixture, Phase::Changed, -50, {150, 50}));
      CHECK(ViewY(fixture) == 50);
      CHECK(box.ScrollTop() == 60);
    }
  }
}
