#include "Krystal.Booey/Animation/ViewTimeline.hpp"
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
    /// @brief A 100px tall scroller (`#scroller`) with `#subject`, 50px tall, 200px down its 450px of
    /// content. The subject's cover range is from 100px to 250px of scroll.
    constexpr const char8_t *SubjectMarkup =
      u8"<!DOCTYPE html><style>body{margin:0}#scroller{overflow:scroll;width:100px;height:100px;"
      u8"scrollbar-width:none}.spacer{height:200px}#subject{height:50px}</style>"
      u8"<div id=scroller><div class=spacer></div><div id=subject></div><div class=spacer></div></div>";

    KRYS_NODISCARD double Percentage(const Maybe<WebAnimationTime> &time) noexcept
    {
      REQUIRE(time);
      REQUIRE(time->Percentage());
      return *time->Percentage();
    }

    KRYS_NODISCARD double Px(const css::CSSNumericValue &value) noexcept
    {
      const auto *unitValue = DynamicDowncast<css::CSSUnitValue>(value);
      REQUIRE(unitValue);
      CHECK(unitValue->UnitEnum() == css::UnitType::px);
      return unitValue->Value();
    }

    void ScrollTo(AnimationPageFixture &fixture, int32 top) noexcept
    {
      fixture.ById(u8"scroller").SetScrollTop(top);
      fixture.TestPage->UpdateRendering();
    }

    KRYS_NODISCARD Ref<ViewTimeline> CreateTimeline(AnimationPageFixture &fixture,
                                                    ViewTimelineInsetValue &&inset = dom::DOMString(u8"auto"))
    {
      auto result =
        ViewTimeline::Create(*fixture.Document, {.Subject = ShareRefPtr(&fixture.ById(u8"subject")),
                                                 .Inset = krys::move(inset)});
      REQUIRE(result.HasValue());
      return result.ReleaseValue();
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ViewTimeline: progress through the cover range",
                   "[Animation][ViewTimeline]")
  {
    AnimationPageFixture fixture {SubjectMarkup};
    auto timeline = CreateTimeline(fixture);

    CHECK(timeline->Subject() == &fixture.ById(u8"subject"));
    CHECK(timeline->Source().get() == &fixture.ById(u8"scroller"));
    CHECK(Px(*timeline->StartOffset()) == 100);
    CHECK(Px(*timeline->EndOffset()) == 250);

    // Before the subject enters the scrollport, the progress is negative.
    CHECK(Percentage(timeline->CurrentTime()) == Catch::Approx(-100.0 / 1.5));
    ScrollTo(fixture, 175);
    CHECK(Percentage(timeline->CurrentTime()) == Catch::Approx(50));
    ScrollTo(fixture, 250);
    CHECK(Percentage(timeline->CurrentTime()) == Catch::Approx(100));

    timeline->DetachFromDocument();
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ViewTimeline: named timeline ranges",
                   "[Animation][ViewTimeline]")
  {
    AnimationPageFixture fixture {SubjectMarkup};
    auto timeline = CreateTimeline(fixture);

    auto checkInterval = [&](style::SingleAnimationRangeName name, double start, double end)
    {
      auto [rangeStart, rangeEnd] = timeline->OffsetIntervalForTimelineRangeName(name);
      CHECK(rangeStart == Catch::Approx(start));
      CHECK(rangeEnd == Catch::Approx(end));
    };
    checkInterval(style::SingleAnimationRangeName::Cover, 0, 1);
    checkInterval(style::SingleAnimationRangeName::Contain, 1.0 / 3, 2.0 / 3);
    checkInterval(style::SingleAnimationRangeName::Entry, 0, 1.0 / 3);
    checkInterval(style::SingleAnimationRangeName::Exit, 2.0 / 3, 1);
    checkInterval(style::SingleAnimationRangeName::EntryCrossing, 0, 1.0 / 3);
    checkInterval(style::SingleAnimationRangeName::ExitCrossing, 2.0 / 3, 1);

    // The default attachment range is the cover range.
    auto [defaultStart, defaultEnd] = timeline->IntervalForAttachmentRange(
      {.Start = {css::keywords::Normal {}}, .End = {css::keywords::Normal {}}});
    CHECK(Percentage(defaultStart) == Catch::Approx(0));
    CHECK(Percentage(defaultEnd) == Catch::Approx(100));

    timeline->DetachFromDocument();
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ViewTimeline: insets", "[Animation][ViewTimeline]")
  {
    AnimationPageFixture fixture {SubjectMarkup};

    SECTION("as text")
    {
      auto timeline = CreateTimeline(fixture, dom::DOMString(u8"10px 20px"));
      CHECK(Px(*timeline->StartOffset()) == 120);
      CHECK(Px(*timeline->EndOffset()) == 240);
      timeline->DetachFromDocument();
    }

    SECTION("as a list")
    {
      auto timeline =
        CreateTimeline(fixture, List<ViewTimelineIndividualInset> {RefPtr<css::CSSNumericValue> {
                                  css::CSSUnitValue::Create(10, css::UnitType::Percentage)}});
      CHECK(Px(*timeline->StartOffset()) == 110);
      CHECK(Px(*timeline->EndOffset()) == 240);
      timeline->DetachFromDocument();
    }

    SECTION("invalid insets throw a TypeError")
    {
      auto expectTypeError = [&](ViewTimelineInsetValue &&inset)
      {
        auto result = ViewTimeline::Create(
          *fixture.Document,
          {.Subject = ShareRefPtr(&fixture.ById(u8"subject")), .Inset = krys::move(inset)});
        REQUIRE(result.HasException());
        CHECK(result.GetException().Code() == dom::ExceptionCode::TypeError);
      };
      expectTypeError(dom::DOMString());
      expectTypeError(dom::DOMString(u8"lemon"));
      expectTypeError(List<ViewTimelineIndividualInset> {});
      expectTypeError(List<ViewTimelineIndividualInset> {dom::DOMString(u8"auto"), dom::DOMString(u8"auto"),
                                                         dom::DOMString(u8"auto")});
      expectTypeError(List<ViewTimelineIndividualInset> {dom::DOMString(u8"none")});
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ViewTimeline: keyframes in named timeline ranges",
                   "[Animation][ViewTimeline]")
  {
    AnimationPageFixture fixture {SubjectMarkup};
    auto timeline = CreateTimeline(fixture);

    InputKeyframe from;
    from.Offset = dom::DOMString(u8"contain 0%");
    from.Properties = {{u8"opacity", u8"0"}};
    InputKeyframe to;
    to.Offset = dom::DOMString(u8"contain 100%");
    to.Properties = {{u8"opacity", u8"1"}};
    KeyframeAnimationOptions options;
    options.Timeline = ShareRefPtr<AnimationTimeline>(timeline.get());
    auto result =
      fixture.ById(u8"subject").Animate(List<InputKeyframe> {krys::move(from), krys::move(to)}, options);
    REQUIRE(result.HasValue());
    auto animation = result.ReleaseValue();
    fixture.TestPage->UpdateRendering();

    // The keyframes sit at the contain range's place in the cover range.
    auto keyframes = animation->GetKeyframeEffect()->GetKeyframes();
    REQUIRE(keyframes.size() == 2);
    CHECK(keyframes[0].ComputedOffset == Catch::Approx(1.0 / 3));
    CHECK(keyframes[1].ComputedOffset == Catch::Approx(2.0 / 3));

    // Halfway through the cover range is halfway through the contain range.
    ScrollTo(fixture, 175);
    CHECK(fixture.Opacity(u8"subject") == Catch::Approx(0.5f));

    SECTION("an attachment range in a named range")
    {
      // Over the entry range, the keyframes' range starts after it.
      animation->SetBindingsRangeStart(dom::DOMString(u8"entry"));
      animation->SetBindingsRangeEnd(dom::DOMString(u8"entry"));
      CHECK(animation->Range().Start.GetName() == style::SingleAnimationRangeName::Entry);
      auto [start, end] = timeline->IntervalForAttachmentRange(animation->Range());
      CHECK(Percentage(start) == Catch::Approx(0));
      CHECK(Percentage(end) == Catch::Approx(100.0 / 3));
    }

    animation->Cancel(WebAnimation::Silently::Yes);
    timeline->DetachFromDocument();
  }
}
