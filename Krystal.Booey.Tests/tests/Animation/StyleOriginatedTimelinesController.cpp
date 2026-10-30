#include "Krystal.Booey/Animation/StyleOriginatedTimelinesController.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Animation/AnimationTestUtils.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/Animation/CSSAnimation.hpp"
#include "Krystal.Booey/Animation/ScrollTimeline.hpp"
#include "Krystal.Booey/Animation/ViewTimeline.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief A page whose `#scroller` scrolls 200px down, with @p style and @p content inside it.
    KRYS_NODISCARD utf8_string ScrollerPage(utf8_stringview style, utf8_stringview content,
                                            utf8_stringview outside = u8"") noexcept
    {
      utf8_string markup =
        u8"<!DOCTYPE html><style>body{margin:0}@keyframes fade{from{opacity:0}to{opacity:1}}"
        u8"#scroller{overflow:scroll;width:100px;height:100px;scrollbar-width:none}"
        u8"#content{height:300px}";
      markup += style;
      markup += u8"</style><div id=root><div id=scroller><div id=content>";
      markup += content;
      markup += u8"</div></div>";
      markup += outside;
      markup += u8"</div>";
      return markup;
    }

    void ScrollTo(AnimationPageFixture &fixture, int32 top) noexcept
    {
      fixture.ById(u8"scroller").SetScrollTop(top);
      fixture.TestPage->UpdateRendering();
    }

    KRYS_NODISCARD CSSAnimation &OnlyAnimation(dom::Element &element) noexcept
    {
      auto animations = element.GetAnimations();
      REQUIRE(animations.size() == 1);
      auto *cssAnimation = DynamicDowncast<CSSAnimation>(animations[0].get());
      REQUIRE(cssAnimation);
      return *cssAnimation;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "StyleOriginatedTimelinesController: scroll()",
                   "[Animation][StyleOriginatedTimelinesController]")
  {
    AnimationPageFixture fixture {
      ScrollerPage(u8"#box{animation:fade linear;animation-timeline:scroll()}", u8"<div id=box></div>")};
    fixture.StartAnimations();

    auto &animation = OnlyAnimation(fixture.ById(u8"box"));
    auto *timeline = DynamicDowncast<ScrollTimeline>(animation.Timeline());
    REQUIRE(timeline);
    // `scroll()` follows the nearest scroller.
    CHECK(timeline->Source().get() == &fixture.ById(u8"scroller"));

    ScrollTo(fixture, 100);
    CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.5f));
    ScrollTo(fixture, 50);
    CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.25f));

    SECTION("an animation range")
    {
      fixture.SetStyle(u8"box", u8"animation-range:0% 50%");
      fixture.Frame();
      ScrollTo(fixture, 50);
      CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.5f));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "StyleOriginatedTimelinesController: view()",
                   "[Animation][StyleOriginatedTimelinesController]")
  {
    // The subject's cover range is from 50px to 200px of scroll.
    AnimationPageFixture fixture {
      ScrollerPage(u8"#subject{margin-top:150px;height:50px;animation:fade linear;animation-timeline:view()}",
                   u8"<div id=subject></div>")};
    fixture.StartAnimations();

    auto &animation = OnlyAnimation(fixture.ById(u8"subject"));
    auto *timeline = DynamicDowncast<ViewTimeline>(animation.Timeline());
    REQUIRE(timeline);
    CHECK(timeline->Subject() == &fixture.ById(u8"subject"));

    ScrollTo(fixture, 125);
    CHECK(fixture.Opacity(u8"subject") == Catch::Approx(0.5f));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "StyleOriginatedTimelinesController: named timelines",
                   "[Animation][StyleOriginatedTimelinesController]")
  {
    AnimationPageFixture fixture {
      ScrollerPage(u8"#scroller{scroll-timeline:--page}#box{animation:fade linear;animation-timeline:--page}",
                   u8"<div id=box></div>")};
    fixture.StartAnimations();

    auto &box = fixture.ById(u8"box");
    auto &animation = OnlyAnimation(box);
    auto *timeline = DynamicDowncast<ScrollTimeline>(animation.Timeline());
    REQUIRE(timeline);
    CHECK(timeline->GetName().View() == u8"--page");
    CHECK(timeline->Source().get() == &fixture.ById(u8"scroller"));

    ScrollTo(fixture, 100);
    CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.5f));

    SECTION("a name no element declares leaves the animation without a timeline")
    {
      fixture.SetStyle(u8"box", u8"animation-timeline:--elsewhere");
      fixture.Frame();
      fixture.Frame();
      CHECK_FALSE(animation.Timeline());
      auto *controller = fixture.Document->GetStyleOriginatedTimelinesController();
      REQUIRE(controller);
      CHECK(controller->IsPendingTimelineAttachment(animation));
    }

    SECTION("removing the name detaches the animation")
    {
      fixture.SetStyle(u8"scroller", u8"scroll-timeline:none");
      fixture.Frame();
      fixture.Frame();
      CHECK_FALSE(animation.Timeline());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "StyleOriginatedTimelinesController: timeline-scope",
                   "[Animation][StyleOriginatedTimelinesController]")
  {
    // `#box` is outside the scroller, so it only sees the scroller's timeline through the scope on `#root`.
    AnimationPageFixture fixture {
      ScrollerPage(u8"#root{timeline-scope:--page}#scroller{scroll-timeline:--page}"
                   u8"#box{animation:fade linear;animation-timeline:--page}",
                   u8"", u8"<div id=box></div>")};
    fixture.StartAnimations();

    auto &animation = OnlyAnimation(fixture.ById(u8"box"));
    auto *timeline = DynamicDowncast<ScrollTimeline>(animation.Timeline());
    REQUIRE(timeline);
    CHECK(timeline->Source().get() == &fixture.ById(u8"scroller"));
    CHECK(timeline->TimelineScopeDeclaredElement().get() == &fixture.ById(u8"root"));

    ScrollTo(fixture, 100);
    CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.5f));

    SECTION("without the scope, the name is out of reach")
    {
      fixture.SetStyle(u8"root", u8"timeline-scope:none");
      fixture.Frame();
      fixture.Frame();
      CHECK_FALSE(animation.Timeline());
    }
  }
}
