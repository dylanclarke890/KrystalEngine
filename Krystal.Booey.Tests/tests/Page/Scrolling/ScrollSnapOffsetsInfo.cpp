#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Animation/AnimationTestUtils.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/ScrollOptions.hpp"
#include "Krystal.Booey/Platform/Events/PlatformKeyboardEvent.hpp"
#include "Krystal.Booey/Platform/Events/PlatformWheelEvent.hpp"
#include "Krystal.Booey/Platform/Events/WindowsKeyboardCodes.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"

namespace krys::boo::tests
{
  namespace
  {
    using namespace std::chrono_literals;

    /// @brief A 100 pixel tall box with five 100 pixel items, the markup after its style.
    KRYS_NODISCARD utf8_string Scroller(utf8_stringview scrollerStyle,
                                        utf8_stringview itemStyle = u8"") noexcept
    {
      return utf8_string(u8"<!DOCTYPE html><style>body{margin:0}#s{width:100px;height:100px;overflow:auto;")
             + utf8_string(scrollerStyle) + u8"}.i{height:100px;scroll-snap-align:start;"
             + utf8_string(itemStyle)
             + u8"}</style><div id=s tabindex=0><div class=i id=a></div><div class=i id=b></div>"
               u8"<div class=i id=c></div><div class=i id=d></div><div class=i id=e></div></div>";
    }

    /// @brief A 100 pixel wide box with a row of five 100 pixel items.
    KRYS_NODISCARD utf8_string Row(utf8_stringview scrollerStyle) noexcept
    {
      utf8_string item = u8"<div class=i style=display:inline-block;width:100px></div>";
      return utf8_string(u8"<!DOCTYPE html><style>body{margin:0}#s{width:100px;height:100px;overflow:auto;"
                         u8"white-space:nowrap;")
             + utf8_string(scrollerStyle) + u8"}.i{height:100px;scroll-snap-align:start}</style><div id=s>"
             + item + item + item + item + item + u8"</div>";
    }

    KRYS_NODISCARD ScrollableArea &AreaOf(dom::Element &element) noexcept
    {
      auto *area = Downcast<RenderBox>(*element.Renderer()).Layer()->GetScrollableArea();
      REQUIRE(area);
      return *area;
    }

    void Finish(AnimationPageFixture &fixture) noexcept
    {
      fixture.Frame();
      fixture.Frame(1'600ms);
    }

    bool Wheel(AnimationPageFixture &fixture, float deltaY) noexcept
    {
      return fixture.TestPage->MainFrame().GetEventHandler().HandleWheelEvent(
        PlatformWheelEvent(IntPoint(10, 10), IntPoint(10, 10), 0, deltaY, 0, deltaY > 0 ? 1.f : -1.f,
                           PlatformWheelEventGranularity::ScrollByPixelWheelEvent, {}, {}));
    }

    bool TouchpadPan(AnimationPageFixture &fixture, PlatformWheelEventPhase phase,
                     PlatformWheelEventPhase momentumPhase, float deltaY) noexcept
    {
      return fixture.TestPage->MainFrame().GetEventHandler().HandleWheelEvent(
        PlatformWheelEvent(IntPoint(10, 10), IntPoint(10, 10), 0, deltaY, 0, 0,
                           PlatformWheelEventGranularity::ScrollByPixelWheelEvent, {}, {})
          .CopyAsTouchpadPan(phase, momentumPhase));
    }

    bool TouchPan(AnimationPageFixture &fixture, float deltaY) noexcept
    {
      return fixture.TestPage->MainFrame().GetEventHandler().HandleTouchScroll(
        PlatformWheelEvent(IntPoint(10, 10), IntPoint(10, 10), 0, deltaY, 0, 0,
                           PlatformWheelEventGranularity::ScrollByPixelWheelEvent, {}, {}));
    }
  }

  TEST_CASE("ScrollSnapOffsetsInfo: a mandatory container",
            "[Page][Scrolling][ScrollSnap][css-scroll-snap:choosing]")
  {
    AnimationPageFixture fixture {Scroller(u8"scroll-snap-type:y mandatory")};
    auto &scroller = fixture.ById(u8"s");

    SECTION("a scroll goes to the closest position")
    {
      scroller.SetScrollTop(130);
      CHECK(scroller.ScrollTop() == 100);
      scroller.SetScrollTop(170);
      CHECK(scroller.ScrollTop() == 200);
    }

    SECTION("a smooth scroll goes to it too")
    {
      ScrollToOptions options(0, 130);
      options.Behavior = ScrollBehavior::Smooth;
      scroller.ScrollTo(options);
      CHECK(scroller.ScrollTop() == 0);
      Finish(fixture);
      CHECK(scroller.ScrollTop() == 100);
    }

    SECTION("scrollBy goes that way, however little")
    {
      scroller.ScrollBy(0, 10);
      CHECK(scroller.ScrollTop() == 100);
      scroller.ScrollBy(0, -10);
      CHECK(scroller.ScrollTop() == 0);
    }

    SECTION("a wheel notch goes to the next position, animated, and notches add up")
    {
      CHECK(Wheel(fixture, -10));
      CHECK(scroller.ScrollTop() == 0);
      fixture.Frame();
      CHECK(Wheel(fixture, -10));
      Finish(fixture);
      CHECK(scroller.ScrollTop() == 200);
    }

    SECTION("an arrow key goes to the next position")
    {
      Downcast<html::HTMLElement>(scroller).Focus();
      auto &handler = fixture.TestPage->MainFrame().GetEventHandler();
      CHECK(handler.KeyEvent(PlatformKeyboardEvent(PlatformEvent::Type::RawKeyDown, u8"", u8"", u8"ArrowDown",
                                                   u8"ArrowDown", u8"Down", vk::Down, false, false, false, {},
                                                   {})));
      Finish(fixture);
      CHECK(scroller.ScrollTop() == 100);
    }

    SECTION("a touch moves freely and snaps when it ends")
    {
      CHECK(TouchPan(fixture, -60));
      CHECK(scroller.ScrollTop() == 60);
      CHECK(TouchPan(fixture, -20));
      CHECK(scroller.ScrollTop() == 80);

      SECTION("layout doesn't re-snap it in the meantime")
      {
        fixture.SetStyle(u8"a", u8"width:50px");
        (void)fixture.Document->UpdateLayout();
        CHECK(scroller.ScrollTop() == 80);
      }

      fixture.TestPage->MainFrame().GetEventHandler().DidEndGestureScroll();
      CHECK(scroller.ScrollTop() == 80);
      Finish(fixture);
      CHECK(scroller.ScrollTop() == 100);
    }

    SECTION("a touchpad's pan moves freely and snaps when its momentum ends")
    {
      using Phase = PlatformWheelEventPhase;
      CHECK_FALSE(TouchpadPan(fixture, Phase::Began, Phase::None, 0));
      CHECK(TouchpadPan(fixture, Phase::Changed, Phase::None, -30));
      CHECK(TouchpadPan(fixture, Phase::Changed, Phase::None, -30));
      CHECK(scroller.ScrollTop() == 60);

      // The fingers flicked: the content coasts on, still free.
      CHECK_FALSE(TouchpadPan(fixture, Phase::None, Phase::Began, 0));
      CHECK(TouchpadPan(fixture, Phase::None, Phase::Changed, -20));
      Finish(fixture);
      CHECK(scroller.ScrollTop() == 80);

      CHECK_FALSE(TouchpadPan(fixture, Phase::None, Phase::Ended, 0));
      Finish(fixture);
      CHECK(scroller.ScrollTop() == 100);
    }

    SECTION("a touchpad's pan without momentum snaps when the fingers lift")
    {
      using Phase = PlatformWheelEventPhase;
      CHECK(TouchpadPan(fixture, Phase::Began, Phase::None, -30));
      CHECK(scroller.ScrollTop() == 30);
      CHECK_FALSE(TouchpadPan(fixture, Phase::Ended, Phase::None, 0));
      Finish(fixture);
      CHECK(scroller.ScrollTop() == 0);
    }

    SECTION("letting go of a scrollbar's thumb snaps")
    {
      auto &area = AreaOf(scroller);
      area.ScrollToOffsetWithoutAnimation(ScrollOffset(0, 130));
      CHECK(scroller.ScrollTop() == 130);
      area.DoPostThumbMoveSnapping(ScrollbarOrientation::Vertical);
      Finish(fixture);
      CHECK(scroller.ScrollTop() == 100);
    }

    SECTION("an autoscroll isn't held at a position")
    {
      Downcast<RenderBox>(*scroller.Renderer())
        .Layer()
        ->GetScrollableArea()
        ->ScrollByRecursively(IntSize(0, 5));
      CHECK(scroller.ScrollTop() == 5);
    }
  }

  TEST_CASE("ScrollSnapOffsetsInfo: a proximity container", "[Page][Scrolling][ScrollSnap]")
  {
    AnimationPageFixture fixture {Scroller(u8"scroll-snap-type:y proximity")};
    auto &scroller = fixture.ById(u8"s");

    // Within 30 pixels (of the 100 pixel snapport) it snaps; beyond, it stays.
    scroller.SetScrollTop(120);
    CHECK(scroller.ScrollTop() == 100);
    scroller.SetScrollTop(150);
    CHECK(scroller.ScrollTop() == 150);
  }

  TEST_CASE("ScrollSnapOffsetsInfo: what makes a snap position",
            "[Page][Scrolling][ScrollSnap][css-scroll-snap:snap-positions]")
  {
    SECTION("no scroll-snap-type")
    {
      AnimationPageFixture fixture {Scroller(u8"")};
      auto &scroller = fixture.ById(u8"s");
      scroller.SetScrollTop(130);
      CHECK(scroller.ScrollTop() == 130);
    }

    SECTION("an axis it doesn't snap in")
    {
      AnimationPageFixture fixture {Scroller(u8"scroll-snap-type:x mandatory")};
      auto &scroller = fixture.ById(u8"s");
      scroller.SetScrollTop(130);
      CHECK(scroller.ScrollTop() == 130);
    }

    SECTION("scroll-padding moves the snapport")
    {
      AnimationPageFixture fixture {
        Scroller(u8"scroll-snap-type:y mandatory;scroll-padding-top:10px", u8"height:80px")};
      auto &scroller = fixture.ById(u8"s");
      scroller.SetScrollTop(100);
      CHECK(scroller.ScrollTop() == 70);
    }

    SECTION("scroll-margin moves the snap area")
    {
      AnimationPageFixture fixture {
        Scroller(u8"scroll-snap-type:y mandatory", u8"height:80px;scroll-margin-top:10px")};
      auto &scroller = fixture.ById(u8"s");
      scroller.SetScrollTop(100);
      CHECK(scroller.ScrollTop() == 70);
    }

    SECTION("end alignment")
    {
      AnimationPageFixture fixture {
        Scroller(u8"scroll-snap-type:y mandatory", u8"height:60px;scroll-snap-align:end")};
      auto &scroller = fixture.ById(u8"s");
      scroller.SetScrollTop(85);
      CHECK(scroller.ScrollTop() == 80);
    }

    SECTION("center alignment")
    {
      AnimationPageFixture fixture {
        Scroller(u8"scroll-snap-type:y mandatory", u8"height:60px;scroll-snap-align:center")};
      auto &scroller = fixture.ById(u8"s");
      // The items' centres, less half the snapport: 0, 40, 100 and so on.
      scroller.SetScrollTop(55);
      CHECK(scroller.ScrollTop() == 40);
    }

    SECTION("scroll-snap-stop stops a scroll passing it")
    {
      AnimationPageFixture fixture {Scroller(u8"scroll-snap-type:y mandatory")};
      fixture.SetStyle(u8"b", u8"scroll-snap-stop:always");
      (void)fixture.Document->UpdateLayout();
      auto &scroller = fixture.ById(u8"s");
      scroller.ScrollBy(0, 350);
      CHECK(scroller.ScrollTop() == 100);
    }

    SECTION("a box of a nested scroller isn't one of its ancestor's")
    {
      AnimationPageFixture fixture {
        u8"<!DOCTYPE html><style>#s{height:100px;overflow:auto;scroll-snap-type:y mandatory}"
        u8"#n{height:100px;overflow:auto}#i{height:300px;margin-top:70px;scroll-snap-align:start}#t{height:"
        u8"500px}"
        u8"</style><div id=s><div id=n><div id=i></div></div><div id=t></div></div>"};
      auto &scroller = fixture.ById(u8"s");
      scroller.SetScrollTop(70);
      CHECK(scroller.ScrollTop() == 70);
    }

    SECTION("a snap area larger than the snapport can be scrolled within")
    {
      AnimationPageFixture fixture {Scroller(u8"scroll-snap-type:y mandatory", u8"height:300px")};
      auto &scroller = fixture.ById(u8"s");
      scroller.SetScrollTop(130);
      CHECK(scroller.ScrollTop() == 130);
    }

    SECTION("right to left")
    {
      AnimationPageFixture fixture {utf8_string(Row(u8"scroll-snap-type:x mandatory;direction:rtl"))};
      auto &scroller = fixture.ById(u8"s");
      scroller.SetScrollLeft(-130);
      CHECK(scroller.ScrollLeft() == -100);
    }

    SECTION("the inline axis")
    {
      AnimationPageFixture fixture {utf8_string(Row(u8"scroll-snap-type:inline mandatory"))};
      auto &scroller = fixture.ById(u8"s");
      scroller.SetScrollLeft(130);
      CHECK(scroller.ScrollLeft() == 100);
    }

    SECTION("the block axis of a vertical writing mode")
    {
      AnimationPageFixture fixture {
        Scroller(u8"scroll-snap-type:block mandatory;writing-mode:vertical-rl", u8"height:auto;width:100px")};
      auto &scroller = fixture.ById(u8"s");
      scroller.SetScrollLeft(-130);
      CHECK(scroller.ScrollLeft() == -100);
    }

    SECTION("the view, from the root's scroll-snap-type")
    {
      AnimationPageFixture fixture {u8"<!DOCTYPE html><style>html{scroll-snap-type:y mandatory}body{margin:0}"
                                    u8"div{height:100px;scroll-snap-align:start}</style>"
                                    u8"<div></div><div></div><div></div><div></div><div></div>"};
      auto &window = *fixture.Document->Window();
      window.ScrollTo(0, 130);
      CHECK(fixture.Document->View()->ScrollY() == 100);
      window.ScrollBy(0, 10);
      CHECK(fixture.Document->View()->ScrollY() == 200);
    }
  }

  TEST_CASE("ScrollSnapOffsetsInfo: re-snapping after layout",
            "[Page][Scrolling][ScrollSnap][css-scroll-snap:re-snap]")
  {
    AnimationPageFixture fixture {Scroller(u8"scroll-snap-type:y mandatory")};
    auto &scroller = fixture.ById(u8"s");

    SECTION("it follows what it's snapped to")
    {
      scroller.SetScrollTop(200);
      REQUIRE(scroller.ScrollTop() == 200);
      fixture.SetStyle(u8"a", u8"height:150px");
      (void)fixture.Document->UpdateLayout();
      CHECK(scroller.ScrollTop() == 250);
    }

    SECTION("if that's gone, it takes the closest")
    {
      scroller.SetScrollTop(200);
      fixture.SetStyle(u8"c", u8"display:none");
      (void)fixture.Document->UpdateLayout();
      CHECK(scroller.ScrollTop() == 200);
    }

    SECTION("when a position is first made, it snaps to it")
    {
      AnimationPageFixture padded {
        Scroller(u8"scroll-snap-type:y mandatory;scroll-padding-top:0", u8"margin-top:40px")};
      (void)padded.Document->UpdateLayout();
      CHECK(padded.ById(u8"s").ScrollTop() == 40);
    }

    SECTION("a change of what snaps takes effect")
    {
      scroller.SetScrollTop(100);
      fixture.SetStyle(u8"b", u8"scroll-snap-align:none");
      (void)fixture.Document->UpdateLayout();
      scroller.SetScrollTop(130);
      CHECK(scroller.ScrollTop() == 200);
    }

    SECTION("no longer a container, it stays and scrolls freely")
    {
      scroller.SetScrollTop(100);
      fixture.SetStyle(u8"s", u8"scroll-snap-type:none");
      (void)fixture.Document->UpdateLayout();
      CHECK(scroller.ScrollTop() == 100);
      scroller.SetScrollTop(130);
      CHECK(scroller.ScrollTop() == 130);
    }
  }
}
