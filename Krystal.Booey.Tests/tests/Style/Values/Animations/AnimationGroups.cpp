#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/Style/Values/Animations/Animations.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Easing/EasingFunction.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/Style/Values/ScrollAnimations/ProgressTimelineAxes.hpp"
#include "Krystal.Booey/Style/Values/ScrollAnimations/ProgressTimelineName.hpp"
#include "Krystal.Booey/Style/Values/ScrollAnimations/ViewTimelineInsets.hpp"
#include "Krystal.Booey/Style/Values/Transitions/Transitions.hpp"

namespace krys::boo::style::tests
{
  using css::PropertyId;
  using namespace css::Literals;

  TEST_CASE_METHOD(BuilderFixture, "Style EasingFunction", "[Style][Values][Animations]")
  {
    auto easeIn = Convert<EasingFunction>(PropertyId::AnimationTimingFunction, u8"ease-in");
    REQUIRE(easeIn.Value->IsCubicBezierTimingFunction());
    CHECK(Downcast<CubicBezierTimingFunction>(*easeIn.Value).GetTimingFunctionPreset()
          == CubicBezierTimingFunction::TimingFunctionPreset::EaseIn);
    CHECK(Serialised(easeIn) == "ease-in");
    CHECK(Created(easeIn) == "ease-in");

    auto custom =
      Convert<EasingFunction>(PropertyId::AnimationTimingFunction, u8"cubic-bezier(0.1, 0.2, 0.3, 2)");
    const auto &bezier = Downcast<CubicBezierTimingFunction>(*custom.Value);
    CHECK(bezier.X1() == 0.1);
    CHECK(bezier.Y2() == 2);
    CHECK(Serialised(custom) == "cubic-bezier(0.1, 0.2, 0.3, 2)");

    auto steps = Convert<EasingFunction>(PropertyId::AnimationTimingFunction, u8"steps(3, jump-start)");
    CHECK(Downcast<StepsTimingFunction>(*steps.Value).GetStepPosition()
          == StepsTimingFunction::StepPosition::JumpStart);
    CHECK(Serialised(steps) == "steps(3, jump-start)");
    CHECK(Serialised(Convert<EasingFunction>(PropertyId::AnimationTimingFunction, u8"steps(2, end)"))
          == "steps(2)");
    auto stepStart = Convert<EasingFunction>(PropertyId::AnimationTimingFunction, u8"step-start");
    CHECK(Downcast<StepsTimingFunction>(*stepStart.Value).NumberOfSteps() == 1);

    // Points without an input are spread evenly between their neighbours.
    auto linear = Convert<EasingFunction>(PropertyId::AnimationTimingFunction, u8"linear(0, 0.25, 1 75%, 1)");
    const auto &points = Downcast<LinearTimingFunction>(*linear.Value).Points();
    REQUIRE(points.size() == 4);
    CHECK(points[1].Progress == 0.375);
    CHECK(points[2].Progress == 0.75);
    CHECK(points[3].Progress == 1);
    CHECK(Serialised(linear) == "linear(0 0%, 0.25 37.5%, 1 75%, 1 100%)");
    CHECK(Serialised(Convert<EasingFunction>(PropertyId::AnimationTimingFunction, u8"linear")) == "linear");

    auto spring = Convert<EasingFunction>(PropertyId::AnimationTimingFunction, u8"spring(1 100 10 0)");
    CHECK(spring.Value->IsSpringTimingFunction());
    CHECK(Serialised(spring) == "spring(1 100 10 0)");

    CHECK(easeIn == Convert<EasingFunction>(PropertyId::AnimationTimingFunction, u8"ease-in"));
    CHECK_FALSE(easeIn == custom);

    auto deprecated =
      CreateTimingFunctionDeprecated(*Parse(PropertyId::AnimationTimingFunction, u8"steps(4)"));
    REQUIRE(deprecated);
    CHECK(Downcast<StepsTimingFunction>(*deprecated).NumberOfSteps() == 4);
  }

  TEST_CASE_METHOD(BuilderFixture, "Style animation values", "[Style][Values][Animations]")
  {
    auto animationName = [&](utf8_stringview value)
    {
      Build(PropertyId::AnimationName, value);
      return Style.GetAnimations()[0].GetName();
    };

    auto name = animationName(u8"slide");
    REQUIRE(name.TryKeyframesName());
    CHECK(name.TryKeyframesName()->Name == css::CSSOMStringAtom {u8"slide"});
    CHECK(name.TryKeyframesName()->IsIdentifier);
    CHECK(Serialised(name) == "slide");
    CHECK(animationName(u8"none").IsNone());
    // A quoted name is an identifier unless it would read as a keyword.
    auto quoted = animationName(u8"\"two words\"");
    CHECK(quoted.TryKeyframesName()->IsIdentifier);
    auto keywordString = animationName(u8"\"none\"");
    REQUIRE(keywordString.TryKeyframesName());
    CHECK_FALSE(keywordString.TryKeyframesName()->IsIdentifier);
    CHECK(Serialised(keywordString) == "\"none\"");

    auto duration = Convert<SingleAnimationDuration>(PropertyId::AnimationDuration, u8"2s");
    CHECK(duration.TryTime()->Value == 2);
    CHECK(Convert<SingleAnimationDuration>(PropertyId::AnimationDuration, u8"auto").IsAuto());
    CHECK(
      Convert<SingleAnimationIterationCount>(PropertyId::AnimationIterationCount, u8"infinite").IsInfinite());
    CHECK(Serialised(Convert<SingleAnimationIterationCount>(PropertyId::AnimationIterationCount, u8"2.5"))
          == "2.5");

    SECTION("timeline")
    {
      CHECK(Convert<SingleAnimationTimeline>(PropertyId::AnimationTimeline, u8"auto").IsAuto());
      CHECK(Convert<SingleAnimationTimeline>(PropertyId::AnimationTimeline, u8"none").IsNone());
      auto named = Convert<SingleAnimationTimeline>(PropertyId::AnimationTimeline, u8"--scroller");
      CHECK(named.TryCustomIdentifier()->Value == css::CSSOMStringAtom {u8"--scroller"});

      auto scroll = Convert<SingleAnimationTimeline>(PropertyId::AnimationTimeline, u8"scroll(root inline)");
      REQUIRE(scroll.TryScrollFunction());
      CHECK(scroll.TryScrollFunction()->Parameters.Scroller == Scroller::Root);
      CHECK(scroll.TryScrollFunction()->Parameters.Axis == ScrollAxis::Inline);
      CHECK(Serialised(scroll) == "scroll(root inline)");
      CHECK(Created(scroll) == "scroll(root inline)");
      CHECK(Serialised(Convert<SingleAnimationTimeline>(PropertyId::AnimationTimeline, u8"scroll()"))
            == "scroll()");

      auto view = Convert<SingleAnimationTimeline>(PropertyId::AnimationTimeline, u8"view(x 10px auto)");
      REQUIRE(view.TryViewFunction());
      CHECK(view.TryViewFunction()->Parameters.Axis == ScrollAxis::X);
      CHECK(view.TryViewFunction()->Parameters.Insets.End().IsAuto());
      CHECK(Serialised(view) == "view(x 10px auto)");
      CHECK(Serialised(Convert<SingleAnimationTimeline>(PropertyId::AnimationTimeline, u8"view(20%)"))
            == "view(20%)");
    }

    SECTION("range")
    {
      auto cover = Convert<SingleAnimationRangeStart>(PropertyId::AnimationRangeStart, u8"cover 10%");
      CHECK(cover.GetName() == SingleAnimationRangeName::Cover);
      CHECK(Serialised(cover) == "cover 10%");
      // The default offset (0% for a start, 100% for an end) is omitted.
      CHECK(Serialised(Convert<SingleAnimationRangeStart>(PropertyId::AnimationRangeStart, u8"entry 0%"))
            == "entry");
      CHECK(Serialised(Convert<SingleAnimationRangeEnd>(PropertyId::AnimationRangeEnd, u8"exit 100%"))
            == "exit");
      CHECK(Serialised(Convert<SingleAnimationRangeEnd>(PropertyId::AnimationRangeEnd, u8"exit 0%"))
            == "exit 0%");
      CHECK(Serialised(Convert<SingleAnimationRangeStart>(PropertyId::AnimationRangeStart, u8"20px"))
            == "20px");
      CHECK(Convert<SingleAnimationRangeEnd>(PropertyId::AnimationRangeEnd, u8"normal").IsNormal());

      auto viewDefault = SingleAnimationRange::DefaultForViewTimeline();
      CHECK(viewDefault.Start.GetName() == SingleAnimationRangeName::Cover);
      CHECK(viewDefault.End.HasDefaultOffset());
      CHECK(SingleAnimationRange {css::keywords::Normal {}, css::keywords::Normal {}}.IsDefault());

      CHECK(ConvertSingleAnimationRangeNameToValueId(SingleAnimationRangeName::EntryCrossing)
            == css::ValueId::EntryCrossing);
      CHECK(ConvertValueIdToSingleAnimationRangeName(css::ValueId::Exit) == SingleAnimationRangeName::Exit);
      CHECK(ConvertRangeStringToSingleTimelineRangeName(u8"contain") == SingleAnimationRangeName::Contain);
      CHECK(ConvertSingleAnimationRangeNameToRangeString(SingleAnimationRangeName::ExitCrossing)
            == u8"exit-crossing");

      auto deprecated = DeprecatedConvertCSSValueToStyle<SingleAnimationRangeStart>(
        nullptr, *Parse(PropertyId::AnimationRangeStart, u8"contain 25%"));
      REQUIRE(deprecated);
      CHECK(deprecated->GetName() == SingleAnimationRangeName::Contain);
    }
  }

  TEST_CASE("Style Animation", "[Style][Values][Animations]")
  {
    Animation animation;
    CHECK(animation.IsInitial());
    CHECK(animation.GetName().IsNone());
    CHECK(animation.GetIterationCount().IsNumber());
    CHECK(animation.GetDuration().IsAuto());
    CHECK(animation.GetTimingFunction() == Animation::InitialTimingFunction());
    CHECK(animation.IsNameUnset());

    Animation named {SingleAnimationName {ScopedName {css::CSSOMStringAtom {u8"spin"}}}};
    CHECK_FALSE(named.IsInitial());
    CHECK(named.IsNameSet());

    named.SetDirection(AnimationDirection::Alternate);
    named.FillPlayState(AnimationPlayState::Paused);
    CHECK(named.GetDirection() == AnimationDirection::Alternate);
    CHECK(named.IsPlayStateFilled());

    named.SetRange(SingleAnimationRange {css::keywords::Cover {}, css::keywords::Contain {}});
    CHECK(named.IsRangeSet());
    CHECK(named.GetRange().End.GetName() == SingleAnimationRangeName::Contain);
    named.ClearRange();
    CHECK(named.IsRangeUnset());
    CHECK(named.GetRangeStart().IsNormal());

    auto clone = Animation::Clone(named);
    CHECK(clone == named);
    CHECK(clone.SortingIdentity() != named.SortingIdentity());
    clone.SetDelay(SingleAnimationDelay {1});
    CHECK_FALSE(clone == named);

    named.SetDefaultTimingFunctionForKeyframes(EasingFunction {LinearTimingFunction::Create()});
    CHECK(named.DefaultTimingFunctionForKeyframes());

    Animations list {Animation {}};
    CHECK(list.ComputedLength() == 1);
    CHECK(list.IsInitial());
  }

  TEST_CASE_METHOD(BuilderFixture, "Style Transition", "[Style][Values][Transitions]")
  {
    CHECK(Convert<SingleTransitionProperty>(PropertyId::TransitionProperty, u8"all").IsAll());
    CHECK(Convert<SingleTransitionProperty>(PropertyId::TransitionProperty, u8"none").IsNone());

    auto opacity = Convert<SingleTransitionProperty>(PropertyId::TransitionProperty, u8"opacity");
    CHECK(opacity.IsSingleProperty());
    CHECK(Serialised(opacity) == "opacity");
    auto custom = Convert<SingleTransitionProperty>(PropertyId::TransitionProperty, u8"--my-prop");
    CHECK(custom.IsSingleProperty());
    CHECK(Serialised(custom) == "--my-prop");
    auto unknown = Convert<SingleTransitionProperty>(PropertyId::TransitionProperty, u8"not-a-property");
    CHECK(unknown.IsUnknownProperty());
    CHECK(Serialised(unknown) == "not-a-property");

    Transition transition;
    CHECK(transition.IsInitial());
    CHECK(transition.GetBehavior() == TransitionBehavior::Normal);
    Transition width {SingleTransitionProperty {PropertyId::Width}};
    CHECK_FALSE(width.IsInitial());
    width.SetDuration(SingleTransitionDuration {0.5});
    width.SetBehavior(TransitionBehavior::AllowDiscrete);
    CHECK(width.IsDurationSet());
    CHECK(width.GetBehavior() == TransitionBehavior::AllowDiscrete);
    auto clone = Transition::Clone(width);
    CHECK(clone == width);
    clone.ClearDuration();
    CHECK(clone.IsDurationUnset());
    CHECK_FALSE(clone == width);

    Transitions list {Transition {}};
    CHECK(list.ComputedLength() == 1);
    CHECK(list.IsInitial());
  }

  TEST_CASE_METHOD(BuilderFixture, "Style scroll timelines", "[Style][Values][ScrollAnimations]")
  {
    auto name = Convert<ProgressTimelineName>(PropertyId::ScrollTimelineName, u8"--a");
    CHECK(name.Value.Value == css::CSSOMStringAtom {u8"--a"});
    CHECK(Serialised(name) == "--a");

    auto single = Convert<ViewTimelineInsets>(PropertyId::ViewTimelineInset, u8"10px 20%");
    const auto &inset = *single.begin();
    CHECK(inset.Start() != inset.End());
    CHECK(Serialised(single) == "10px 20%");
    // The pair serialises minimally.
    CHECK(Serialised(Convert<ViewTimelineInsets>(PropertyId::ViewTimelineInset, u8"auto auto")) == "auto");

    auto insets = Convert<ViewTimelineInsets>(PropertyId::ViewTimelineInset, u8"auto, 5px 10px");
    CHECK(Serialised(insets) == "auto, 5px 10px");
    CHECK(Serialised(ViewTimelineInsets {css::keywords::Auto {}}) == "auto");
    CHECK(Serialised(ProgressTimelineAxes {css::keywords::Block {}}) == "block");
  }
}
