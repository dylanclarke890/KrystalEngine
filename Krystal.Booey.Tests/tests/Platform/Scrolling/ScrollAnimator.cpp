#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Animation/AnimationTestUtils.hpp"
#include "Krystal.Booey/DOM/Events/EventListener.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/ScrollOptions.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/Events/PlatformKeyboardEvent.hpp"
#include "Krystal.Booey/Platform/Events/PlatformWheelEvent.hpp"
#include "Krystal.Booey/Platform/Events/WindowsKeyboardCodes.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderListBox.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"

namespace krys::boo::tests
{
  namespace
  {
    using namespace std::chrono_literals;

    /// @brief A box 100 pixels tall, scrolling 1000 pixels of content.
    constexpr utf8_stringview Page =
      u8"<!DOCTYPE html><style>body{margin:0}#s{width:100px;height:100px;overflow:auto}#c{height:1000px}"
      u8".smooth{scroll-behavior:smooth}</style><div id=s tabindex=0><div id=c></div></div>";

    struct EventCounter final : dom::EventListenerCallback
    {
      int32 Count {0};

      void HandleEvent(dom::Event &) noexcept override
      {
        ++Count;
      }
    };

    KRYS_NODISCARD ScrollToOptions Smooth(double top) noexcept
    {
      ScrollToOptions options(0, top);
      options.Behavior = ScrollBehavior::Smooth;
      return options;
    }

    KRYS_NODISCARD ScrollableArea &AreaOf(dom::Element &element) noexcept
    {
      auto *area = Downcast<RenderBox>(*element.Renderer()).Layer()->GetScrollableArea();
      REQUIRE(area);
      return *area;
    }

    /// @brief Runs the frames that finish any smooth scroll.
    void Finish(AnimationPageFixture &fixture) noexcept
    {
      fixture.Frame();
      fixture.Frame(1'600ms);
    }

    bool PressKey(AnimationPageFixture &fixture, utf8_string key, utf8_string identifier,
                  int32 keyCode) noexcept
    {
      return fixture.TestPage->MainFrame().GetEventHandler().KeyEvent(
        PlatformKeyboardEvent(PlatformEvent::Type::RawKeyDown, u8"", u8"", key, key, krys::move(identifier),
                              keyCode, false, false, false, {}, {}));
    }
  }

  TEST_CASE("ScrollAnimator: a smooth scroll", "[Platform][Scrolling][ScrollAnimator][cssom-view:scrolling]")
  {
    AnimationPageFixture fixture {utf8_string(Page)};
    auto &scroller = fixture.ById(u8"s");
    auto scroll = CreateRef<EventCounter>();
    auto scrollEnd = CreateRef<EventCounter>();
    scroller.AddEventListener(dom::DOMStringAtom(EventNames::Scroll),
                              ShareRef<dom::EventListenerCallback>(*scroll), false);
    scroller.AddEventListener(dom::DOMStringAtom(EventNames::ScrollEnd),
                              ShareRef<dom::EventListenerCallback>(*scrollEnd), false);

    SECTION("moves over frames, and ends with one scrollend")
    {
      scroller.ScrollTo(Smooth(100));
      CHECK(scroller.ScrollTop() == 0);
      CHECK(AreaOf(scroller).GetScrollAnimationStatus() == ScrollAnimationStatus::Animating);

      fixture.Frame();
      CHECK(scroller.ScrollTop() == 0);

      fixture.Frame(50ms);
      CHECK(scroller.ScrollTop() > 0);
      CHECK(scroller.ScrollTop() < 100);
      CHECK(scroll->Count == 1);
      CHECK(scrollEnd->Count == 0);

      // 100 pixels take a sixth of a second.
      fixture.Frame(150ms);
      CHECK(scroller.ScrollTop() == 100);
      CHECK(AreaOf(scroller).GetScrollAnimationStatus() == ScrollAnimationStatus::NotAnimating);
      CHECK(scroll->Count == 2);
      CHECK(scrollEnd->Count == 1);

      fixture.Frame(100ms);
      CHECK(scrollEnd->Count == 1);
    }

    SECTION("a longer way takes longer, as the square root of the distance")
    {
      scroller.ScrollTo(Smooth(900));
      fixture.Frame();
      fixture.Frame(200ms);
      CHECK(scroller.ScrollTop() < 900);
      fixture.Frame(310ms);
      CHECK(scroller.ScrollTop() == 900);
    }

    SECTION("no way takes longer than a second and a half")
    {
      fixture.SetStyle(u8"c", u8"height:200000px");
      (void)fixture.Document->UpdateLayout();
      scroller.ScrollTo(Smooth(190'000));
      fixture.Frame();
      fixture.Frame(1'490ms);
      CHECK(scroller.ScrollTop() < 190'000);
      fixture.Frame(20ms);
      CHECK(scroller.ScrollTop() == 190'000);
    }

    SECTION("to where it already is doesn't animate")
    {
      scroller.ScrollTo(Smooth(0));
      CHECK(AreaOf(scroller).GetScrollAnimationStatus() == ScrollAnimationStatus::NotAnimating);
      Finish(fixture);
      CHECK(scroll->Count == 0);
      CHECK(scrollEnd->Count == 0);
    }

    SECTION("a behavior of auto follows scroll-behavior")
    {
      REQUIRE_FALSE(scroller.SetAttribute(u8"class", u8"smooth").HasException());
      (void)fixture.Document->UpdateLayout();

      scroller.SetScrollTop(100);
      CHECK(scroller.ScrollTop() == 0);
      Finish(fixture);
      CHECK(scroller.ScrollTop() == 100);

      scroller.ScrollTo(0, 50);
      CHECK(scroller.ScrollTop() == 100);
      Finish(fixture);
      CHECK(scroller.ScrollTop() == 50);

      SECTION("and instant overrides it")
      {
        ScrollToOptions options(0, 200);
        options.Behavior = ScrollBehavior::Instant;
        scroller.ScrollTo(options);
        CHECK(scroller.ScrollTop() == 200);
      }
    }

    SECTION("without scroll-behavior, auto is instant")
    {
      scroller.SetScrollTop(100);
      CHECK(scroller.ScrollTop() == 100);
    }

    SECTION("another retargets it, with one scrollend")
    {
      scroller.ScrollTo(Smooth(100));
      fixture.Frame();
      fixture.Frame(50ms);
      auto reached = scroller.ScrollTop();
      REQUIRE(reached > 0);

      scroller.ScrollTo(Smooth(300));
      CHECK(scroller.ScrollTop() == reached);
      Finish(fixture);
      CHECK(scroller.ScrollTop() == 300);
      CHECK(scrollEnd->Count == 1);
    }

    SECTION("an instant scroll stops it")
    {
      scroller.ScrollTo(Smooth(100));
      fixture.Frame();
      fixture.Frame(50ms);
      scroller.SetScrollTop(10);
      CHECK(scroller.ScrollTop() == 10);
      CHECK(AreaOf(scroller).GetScrollAnimationStatus() == ScrollAnimationStatus::NotAnimating);
      Finish(fixture);
      CHECK(scroller.ScrollTop() == 10);
      CHECK(scrollEnd->Count == 1);
    }

    SECTION("an instant scroll to where it has got to stops it, and scrollend fires")
    {
      scroller.ScrollTo(Smooth(100));
      fixture.Frame();
      fixture.Frame(50ms);
      auto reached = scroller.ScrollTop();
      scroller.SetScrollTop(reached);
      Finish(fixture);
      CHECK(scroller.ScrollTop() == reached);
      CHECK(scrollEnd->Count == 1);
    }

    SECTION("scrollTo(0, 0) stops one going elsewhere")
    {
      scroller.ScrollTo(Smooth(100));
      scroller.ScrollTo(0, 0);
      Finish(fixture);
      CHECK(scroller.ScrollTop() == 0);
    }

    SECTION("scrollTo(0, 0) goes back after an instant scroll")
    {
      scroller.SetScrollTop(100);
      scroller.ScrollTo(0, 0);
      CHECK(scroller.ScrollTop() == 0);
    }

    SECTION("stops at the end of content that shrinks")
    {
      scroller.ScrollTo(Smooth(800));
      fixture.Frame();
      fixture.SetStyle(u8"c", u8"height:150px");
      (void)fixture.Document->UpdateLayout();
      Finish(fixture);
      CHECK(scroller.ScrollTop() == 50);
    }

    SECTION("ends with the box")
    {
      scroller.ScrollTo(Smooth(100));
      fixture.Frame();
      fixture.SetStyle(u8"s", u8"display:none");
      Finish(fixture);
      CHECK(fixture.Document->View()->ScrollY() == 0);
    }

    SECTION("scrollBy goes on from the scroll position")
    {
      scroller.SetScrollTop(100);
      auto options = Smooth(50);
      scroller.ScrollBy(options);
      CHECK(scroller.ScrollTop() == 100);
      Finish(fixture);
      CHECK(scroller.ScrollTop() == 150);
    }
  }

  TEST_CASE("ScrollAnimator: scrolling an element into view smoothly",
            "[Platform][Scrolling][ScrollAnimator][cssom-view:scroll-an-element-into-view]")
  {
    AnimationPageFixture fixture {
      u8"<!DOCTYPE "
      u8"html><style>body{margin:0;height:2000px}#o{width:100px;height:80px;overflow:auto;margin-top:500px}"
      u8"#t{margin-top:400px;height:20px}</style><div id=o><div id=t></div></div>"};
    auto &outer = fixture.ById(u8"o");

    ScrollIntoViewOptions options;
    options.Behavior = ScrollBehavior::Smooth;
    fixture.ById(u8"t").ScrollIntoView(options);
    CHECK(outer.ScrollTop() == 0);
    CHECK(fixture.Document->View()->ScrollY() == 0);

    // The box scrolls as far as it can, leaving the target 60 pixels down it, and the view does the rest.
    Finish(fixture);
    CHECK(outer.ScrollTop() == 340);
    CHECK(fixture.Document->View()->ScrollY() == 560);
  }

  TEST_CASE("ScrollAnimator: the view's smooth scroll",
            "[Platform][Scrolling][ScrollAnimator][cssom-view:scrolling]")
  {
    AnimationPageFixture fixture {u8"<!DOCTYPE html><style>body{margin:0;height:2000px}</style><p>a"};
    auto &view = *fixture.Document->View();
    auto &window = *fixture.Document->Window();
    auto scrollEnd = CreateRef<EventCounter>();
    fixture.Document->AddEventListener(dom::DOMStringAtom(EventNames::ScrollEnd),
                                       ShareRef<dom::EventListenerCallback>(*scrollEnd), false);

    SECTION("scrollTo with smooth")
    {
      window.ScrollTo(Smooth(100));
      CHECK(view.ScrollY() == 0);
      CHECK(view.GetScrollAnimationStatus() == ScrollAnimationStatus::Animating);
      fixture.Frame();
      fixture.Frame(50ms);
      CHECK(view.ScrollY() > 0);
      CHECK(view.ScrollY() < 100);
      Finish(fixture);
      CHECK(view.ScrollY() == 100);
      CHECK(scrollEnd->Count == 1);
    }

    SECTION("an instant scroll stops it")
    {
      window.ScrollTo(Smooth(100));
      fixture.Frame();
      fixture.Frame(50ms);
      window.ScrollTo(0, 0);
      CHECK(view.ScrollY() == 0);
      Finish(fixture);
      CHECK(view.ScrollY() == 0);
      CHECK(scrollEnd->Count == 1);
    }
  }

  TEST_CASE("ScrollAnimator: the keyboard's scrolls are animated", "[Platform][Scrolling][ScrollAnimator]")
  {
    AnimationPageFixture fixture {utf8_string(Page)};
    auto &scroller = fixture.ById(u8"s");
    Downcast<html::HTMLElement>(scroller).Focus();

    SECTION("an arrow key")
    {
      CHECK(PressKey(fixture, u8"ArrowDown", u8"Down", vk::Down));
      CHECK(scroller.ScrollTop() == 0);
      Finish(fixture);
      CHECK(scroller.ScrollTop() == 40);
    }

    SECTION("presses in quick succession add up")
    {
      CHECK(PressKey(fixture, u8"ArrowDown", u8"Down", vk::Down));
      fixture.Frame();
      CHECK(PressKey(fixture, u8"ArrowDown", u8"Down", vk::Down));
      CHECK(PressKey(fixture, u8"ArrowDown", u8"Down", vk::Down));
      Finish(fixture);
      CHECK(scroller.ScrollTop() == 120);
    }

    SECTION("not if the user's preference turns them off")
    {
      auto &settings = fixture.Document->GetSettings();
      settings.ScrollAnimatorEnabled = false;
      CHECK(PressKey(fixture, u8"ArrowDown", u8"Down", vk::Down));
      CHECK(scroller.ScrollTop() == 40);
      settings.ScrollAnimatorEnabled = true;
    }
  }

  TEST_CASE("ScrollAnimator: a wheel's notches are animated", "[Platform][Scrolling][ScrollAnimator]")
  {
    AnimationPageFixture fixture {utf8_string(Page)};
    auto &scroller = fixture.ById(u8"s");
    auto &handler = fixture.TestPage->MainFrame().GetEventHandler();
    // A notch down (the content moves up), over the box.
    auto notch = [&](IntPoint position = {10, 10})
    {
      return handler.HandleWheelEvent(PlatformWheelEvent(
        position, position, 0, -100, 0, -1, PlatformWheelEventGranularity::ScrollByPixelWheelEvent, {}, {}));
    };

    SECTION("over frames")
    {
      CHECK(notch());
      CHECK(scroller.ScrollTop() == 0);
      CHECK(AreaOf(scroller).GetScrollAnimationStatus() == ScrollAnimationStatus::Animating);
      fixture.Frame();
      fixture.Frame(50ms);
      CHECK(scroller.ScrollTop() > 0);
      CHECK(scroller.ScrollTop() < 100);
      Finish(fixture);
      CHECK(scroller.ScrollTop() == 100);
    }

    SECTION("notches in quick succession add up")
    {
      CHECK(notch());
      fixture.Frame();
      CHECK(notch());
      CHECK(notch());
      Finish(fixture);
      CHECK(scroller.ScrollTop() == 300);
    }

    SECTION("the view's too")
    {
      fixture.SetStyle(u8"s", u8"margin-bottom:1000px");
      CHECK(notch({150, 50}));
      auto &view = *fixture.Document->View();
      CHECK(view.ScrollY() == 0);
      Finish(fixture);
      CHECK(view.ScrollY() == 100);
    }

    SECTION("but not a touchpad's pan, which follows the fingers")
    {
      CHECK(handler.HandleWheelEvent(
        PlatformWheelEvent(IntPoint(10, 10), IntPoint(10, 10), 0, -30, 0, 0,
                           PlatformWheelEventGranularity::ScrollByPixelWheelEvent, {}, {})
          .CopyAsTouchpadPan(PlatformWheelEventPhase::Changed, PlatformWheelEventPhase::None)));
      CHECK(scroller.ScrollTop() == 30);
    }

    SECTION("nor if the user's preference turns them off")
    {
      auto &settings = fixture.Document->GetSettings();
      settings.ScrollAnimatorEnabled = false;
      CHECK(notch());
      CHECK(scroller.ScrollTop() == 100);
      settings.ScrollAnimatorEnabled = true;
    }
  }

  TEST_CASE("ScrollAnimator: a list box's smooth scrolls",
            "[Platform][Scrolling][ScrollAnimator][RenderListBox]")
  {
    AnimationPageFixture fixture {utf8_string(
      u8"<!DOCTYPE html><style>body{margin:0}</style><select id=s size=3><option>0</option><option>1</option>"
      u8"<option>2</option><option>3</option><option>4</option><option>5</option><option>6</option>"
      u8"<option>7</option><option>8</option><option>9</option></select>")};
    auto &select = Downcast<html::HTMLSelectElement>(fixture.ById(u8"s"));
    auto &listBox = Downcast<RenderListBox>(*select.Renderer());
    auto &area = static_cast<ScrollableArea &>(listBox);
    int32 itemHeight =
      (listBox.ItemBoundingBoxRect({}, 1).Y() - listBox.ItemBoundingBoxRect({}, 0).Y()).ToInt();
    auto scrollEnd = CreateRef<EventCounter>();
    select.AddEventListener(dom::DOMStringAtom(EventNames::ScrollEnd),
                            ShareRef<dom::EventListenerCallback>(*scrollEnd), false);

    SECTION("scrollTop, with scroll-behavior: smooth, moves over frames and ends with one scrollend")
    {
      fixture.SetStyle(u8"s", u8"scroll-behavior:smooth");
      select.SetScrollTop(2 * itemHeight);
      CHECK(select.ScrollTop() == 0);
      CHECK(area.GetScrollAnimationStatus() == ScrollAnimationStatus::Animating);

      fixture.Frame();
      fixture.Frame(50ms);
      CHECK(select.ScrollTop() > 0);
      CHECK(select.ScrollTop() < 2 * itemHeight);
      CHECK(scrollEnd->Count == 0);

      Finish(fixture);
      CHECK(select.ScrollTop() == 2 * itemHeight);
      CHECK(area.GetScrollAnimationStatus() == ScrollAnimationStatus::NotAnimating);
      CHECK(scrollEnd->Count == 1);
    }

    SECTION("an instant one jumps")
    {
      select.ScrollTo(ScrollToOptions(0, itemHeight));
      CHECK(select.ScrollTop() == itemHeight);
      CHECK(area.GetScrollAnimationStatus() == ScrollAnimationStatus::NotAnimating);
    }

    SECTION("the keys reveal the selection smoothly")
    {
      select.Focus();
      CHECK(PressKey(fixture, u8"End", u8"End", vk::End));
      CHECK(select.SelectedIndex() == 9);
      CHECK(area.GetScrollAnimationStatus() == ScrollAnimationStatus::Animating);
      CHECK_FALSE(listBox.ListIndexIsVisible(9));

      Finish(fixture);
      CHECK(listBox.ListIndexIsVisible(9));
      CHECK(select.ScrollTop() == 7 * itemHeight);
    }

    SECTION("presses in quick succession go on from where the last is going")
    {
      select.Focus();
      for (int32 press = 0; press < 5; ++press)
      {
        CHECK(PressKey(fixture, u8"ArrowDown", u8"Down", vk::Down));
      }
      // Revealing the new selection doesn't cut the smooth scroll short.
      CHECK(select.SelectedIndex() == 4);
      CHECK(area.GetScrollAnimationStatus() == ScrollAnimationStatus::Animating);

      Finish(fixture);
      CHECK(select.ScrollTop() == 2 * itemHeight);
      CHECK(listBox.ListIndexIsVisible(4));
    }
  }
}
