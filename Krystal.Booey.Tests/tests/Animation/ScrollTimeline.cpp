#include "Krystal.Booey/Animation/ScrollTimeline.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Animation/AnimationTestUtils.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Booey/Style/Values/Animations/SingleAnimationRange.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief A 100px square scroller (`#scroller`) whose content is 300px tall and wide, so it scrolls 200px
    /// either way, holding `#box`.
    constexpr const char8_t *ScrollerMarkup =
      u8"<!DOCTYPE html><style>body{margin:0}#scroller{overflow:scroll;width:100px;height:100px;"
      u8"scrollbar-width:none}#content{width:300px;height:300px}</style>"
      u8"<div id=scroller><div id=content><div id=box></div></div></div>";

    KRYS_NODISCARD double Percentage(const Maybe<WebAnimationTime> &time) noexcept
    {
      REQUIRE(time);
      REQUIRE(time->Percentage());
      return *time->Percentage();
    }

    KRYS_NODISCARD KeyframesInput Fade() noexcept
    {
      InputKeyframe from;
      from.Properties = {{u8"opacity", u8"0"}};
      InputKeyframe to;
      to.Properties = {{u8"opacity", u8"1"}};
      return List<InputKeyframe> {krys::move(from), krys::move(to)};
    }

    /// @brief Scrolls `#scroller` to @p top, and runs a rendering update so its timelines catch up.
    void ScrollTo(AnimationPageFixture &fixture, int32 top, int32 left = 0) noexcept
    {
      auto &scroller = fixture.ById(u8"scroller");
      scroller.SetScrollTop(top);
      scroller.SetScrollLeft(left);
      fixture.TestPage->UpdateRendering();
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ScrollTimeline: progress from a scroller",
                   "[Animation][ScrollTimeline]")
  {
    AnimationPageFixture fixture {ScrollerMarkup};
    auto &scroller = fixture.ById(u8"scroller");
    auto timeline = ScrollTimeline::Create(*fixture.Document, {.Source = ShareRefPtr(&scroller)});

    CHECK(timeline->Source().get() == &scroller);
    CHECK(timeline->Axis() == ScrollAxis::Block);
    CHECK(timeline->IsProgressBased());
    CHECK(Percentage(timeline->Duration()) == 100);
    CHECK(Percentage(timeline->CurrentTime()) == 0);

    ScrollTo(fixture, 50);
    CHECK(Percentage(timeline->CurrentTime()) == Catch::Approx(25));
    ScrollTo(fixture, 200);
    CHECK(Percentage(timeline->CurrentTime()) == Catch::Approx(100));

    SECTION("the other axes")
    {
      ScrollTo(fixture, 0, 150);
      timeline->SetAxis(ScrollAxis::Inline);
      CHECK(Percentage(timeline->CurrentTime(UseCachedCurrentTime::No)) == Catch::Approx(75));
      timeline->SetAxis(ScrollAxis::X);
      CHECK(Percentage(timeline->CurrentTime(UseCachedCurrentTime::No)) == Catch::Approx(75));
      timeline->SetAxis(ScrollAxis::Y);
      CHECK(Percentage(timeline->CurrentTime(UseCachedCurrentTime::No)) == 0);
    }

    SECTION("a source that doesn't scroll is inactive")
    {
      auto inactive =
        ScrollTimeline::Create(*fixture.Document, {.Source = ShareRefPtr(&fixture.ById(u8"box"))});
      CHECK_FALSE(inactive->CurrentTime());
      inactive->DetachFromDocument();
    }

    timeline->DetachFromDocument();
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ScrollTimeline: sources", "[Animation][ScrollTimeline]")
  {
    AnimationPageFixture fixture {ScrollerMarkup};
    auto &box = fixture.ById(u8"box");

    // Without a source, the timeline follows the document's scrolling element.
    auto rootTimeline = ScrollTimeline::Create(*fixture.Document);
    CHECK(rootTimeline->Source() == fixture.Document->ScrollingElement());

    // From style, the source is relative to an element.
    auto nearest = ScrollTimeline::Create(Scroller::Nearest, ScrollAxis::Block);
    nearest->SetSource(style::Styleable::FromElement(box));
    CHECK(nearest->Source().get() == &fixture.ById(u8"scroller"));
    auto root = ScrollTimeline::Create(Scroller::Root, ScrollAxis::Block);
    root->SetSource(style::Styleable::FromElement(box));
    CHECK(root->Source() == fixture.Document->ScrollingElement());
    auto self = ScrollTimeline::Create(Scroller::Self, ScrollAxis::Block);
    self->SetSource(style::Styleable::FromElement(box));
    CHECK(self->Source().get() == &box);

    // Removing the source detaches the timeline from the document's timelines.
    self->SetSource(nullptr);
    CHECK_FALSE(self->Source());
    CHECK_FALSE(self->Controller());

    rootTimeline->DetachFromDocument();
    nearest->DetachFromDocument();
    root->DetachFromDocument();
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ScrollTimeline: animations follow the scroller",
                   "[Animation][ScrollTimeline]")
  {
    AnimationPageFixture fixture {ScrollerMarkup};
    auto timeline = ScrollTimeline::Create(*fixture.Document,
                                           {.Source = ShareRefPtr(&fixture.ById(u8"scroller"))});

    KeyframeAnimationOptions options;
    options.Timeline = ShareRefPtr<AnimationTimeline>(timeline.get());
    auto result = fixture.ById(u8"box").Animate(Fade(), options);
    REQUIRE(result.HasValue());
    auto animation = result.ReleaseValue();

    // The animation's duration is the timeline's, and its start time is aligned with the scroller's start.
    fixture.TestPage->UpdateRendering();
    CHECK_FALSE(animation->Pending());
    CHECK(Percentage(animation->StartTime()) == 0);
    CHECK(fixture.Opacity(u8"box") == Catch::Approx(0));
    ScrollTo(fixture, 100);
    CHECK(Percentage(animation->CurrentTime()) == Catch::Approx(50));
    CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.5f));

    SECTION("an attachment range")
    {
      // Over the first half of the scroll range: 50px of 200px is halfway through it.
      animation->SetBindingsRangeEnd(dom::DOMString(u8"100px"));
      CHECK_FALSE(animation->Range().End.IsNormal());
      ScrollTo(fixture, 50);
      CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.5f));

      // A range edge is read back as an offset in a range name.
      auto rangeEnd = animation->BindingsRangeEnd();
      REQUIRE(std::holds_alternative<TimelineRangeOffset>(rangeEnd));
      auto &rangeEndOffset = std::get<TimelineRangeOffset>(rangeEnd);
      CHECK(rangeEndOffset.RangeName == u8"omitted");
      auto *offset = DynamicDowncast<css::CSSUnitValue>(rangeEndOffset.Offset.get());
      REQUIRE(offset);
      CHECK(offset->Value() == 100);
      CHECK(offset->UnitEnum() == css::UnitType::px);
      auto rangeStart = animation->BindingsRangeStart();
      REQUIRE(std::holds_alternative<dom::DOMString>(rangeStart));
      CHECK(std::get<dom::DOMString>(rangeStart) == u8"normal");
    }

    animation->Cancel(WebAnimation::Silently::Yes);
    timeline->DetachFromDocument();
  }
}
