#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Animation/AnimationTestUtils.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"

namespace krys::boo::tests
{
  namespace
  {
    using namespace std::chrono_literals;

    KRYS_NODISCARD std::string Narrow(const dom::DOMString &string)
    {
      return std::string(string.begin(), string.end());
    }

    KRYS_NODISCARD InputKeyframe Keyframe(KeyframeOffset offset,
                                          List<Pair<dom::DOMString, dom::DOMString>> properties,
                                          dom::DOMString easing = u8"linear") noexcept
    {
      InputKeyframe keyframe;
      keyframe.Offset = krys::move(offset);
      keyframe.Easing = krys::move(easing);
      keyframe.Properties = krys::move(properties);
      return keyframe;
    }

    /// @brief A page with `#box`.
    struct BoxFixture : AnimationPageFixture
    {
      BoxFixture() noexcept : AnimationPageFixture(u8"<!DOCTYPE html><div id=box></div>")
      {
      }

      KRYS_NODISCARD dom::ExceptionOr<Ref<KeyframeEffect>>
        Effect(KeyframesInput &&keyframes,
               Maybe<Variant<double, KeyframeEffectOptions>> &&options = 1'000.0) noexcept
      {
        return KeyframeEffect::Create(*Document, &ById(u8"box"), krys::move(keyframes), krys::move(options));
      }
    };

    KRYS_NODISCARD Ref<KeyframeEffect> Valid(dom::ExceptionOr<Ref<KeyframeEffect>> &&result)
    {
      REQUIRE(result.HasValue());
      return result.ReleaseValue();
    }

    KRYS_NODISCARD dom::ExceptionCode Error(dom::ExceptionOr<Ref<KeyframeEffect>> &&result)
    {
      REQUIRE(result.HasException());
      return result.ReleaseException().Code();
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "KeyframeEffect: a list of keyframes",
                   "[Animation][KeyframeEffect]")
  {
    BoxFixture fixture;

    SECTION("offsets are computed and values serialised")
    {
      auto effect = Valid(fixture.Effect(List<InputKeyframe> {
        Keyframe(nullptr, {{u8"opacity", u8"0"}, {u8"marginLeft", u8"10px"}}),
        Keyframe(0.25, {{u8"opacity", u8"0.5"}}, u8"ease-in"),
        Keyframe(nullptr, {{u8"opacity", u8"0.75"}}),
        Keyframe(nullptr, {{u8"opacity", u8"1"}, {u8"marginLeft", u8"20px"}}),
      }));
      CHECK(effect->Target() == &fixture.ById(u8"box"));
      CHECK(effect->SpecifiedIterationDuration() == Seconds {1});

      auto keyframes = effect->GetKeyframes();
      REQUIRE(keyframes.size() == 4);
      CHECK(std::holds_alternative<std::nullptr_t>(keyframes[0].Offset));
      CHECK(keyframes[0].ComputedOffset == 0);
      CHECK(std::get<double>(keyframes[1].Offset) == 0.25);
      CHECK(keyframes[2].ComputedOffset == Catch::Approx(0.625));
      CHECK(keyframes[3].ComputedOffset == 1);
      CHECK(Narrow(keyframes[1].Easing) == "ease-in");
      CHECK(Narrow(keyframes[0].Easing) == "linear");
      CHECK(Narrow(keyframes[0].StyleStrings.at(css::PropertyId::MarginLeft)) == "10px");
      CHECK(Narrow(keyframes[3].StyleStrings.at(css::PropertyId::Opacity)) == "1");
      CHECK(keyframes[1].StyleStrings.size() == 1);
      CHECK(effect->AnimatesProperty(css::PropertyId::Opacity));
      CHECK_FALSE(effect->AnimatesProperty(css::PropertyId::Color));
    }

    SECTION("property names are IDL attributes")
    {
      auto effect = Valid(fixture.Effect(List<InputKeyframe> {
        Keyframe(nullptr, {{u8"backgroundColor", u8"red"},
                           {u8"background-color", u8"blue"},
                           {u8"cssFloat", u8"left"},
                           {u8"--size", u8"4px"},
                           {u8"lemon", u8"1"}}),
      }));
      auto keyframes = effect->GetKeyframes();
      REQUIRE(keyframes.size() == 1);
      CHECK(Narrow(keyframes[0].StyleStrings.at(css::PropertyId::BackgroundColor)) == "red");
      CHECK(Narrow(keyframes[0].StyleStrings.at(css::PropertyId::Float)) == "left");
      CHECK(keyframes[0].StyleStrings.size() == 2);
      CHECK(Narrow(keyframes[0].CustomStyleStrings.at(css::CSSOMStringAtom(u8"--size"))) == "4px");
      CHECK(Narrow(KeyframeEffect::PropertyIdToIDLAttributeName(css::PropertyId::BackgroundColor))
            == "backgroundColor");
      CHECK(Narrow(KeyframeEffect::PropertyIdToIDLAttributeName(css::PropertyId::Float)) == "cssFloat");
    }

    SECTION("a value that doesn't parse is dropped")
    {
      auto effect = Valid(fixture.Effect(List<InputKeyframe> {Keyframe(nullptr, {{u8"opacity", u8"lots"}})}));
      CHECK(effect->GetKeyframes()[0].StyleStrings.empty());
    }

    SECTION("invalid keyframes throw a TypeError")
    {
      CHECK(Error(fixture.Effect(List<InputKeyframe> {Keyframe(0.5, {}), Keyframe(0.25, {})}))
            == dom::ExceptionCode::TypeError);
      CHECK(Error(fixture.Effect(List<InputKeyframe> {Keyframe(1.5, {})})) == dom::ExceptionCode::TypeError);
      CHECK(Error(fixture.Effect(List<InputKeyframe> {Keyframe(nullptr, {}, u8"bounce")}))
            == dom::ExceptionCode::TypeError);
      CHECK(Error(fixture.Effect(List<InputKeyframe> {Keyframe(dom::DOMString(u8"lemon"), {})}))
            == dom::ExceptionCode::TypeError);
    }

    SECTION("string offsets")
    {
      auto effect = Valid(fixture.Effect(List<InputKeyframe> {
        Keyframe(dom::DOMString(u8"0.5"), {{u8"opacity", u8"0"}}),
        Keyframe(dom::DOMString(u8"cover 20%"), {{u8"opacity", u8"1"}}),
      }));
      auto keyframes = effect->GetKeyframes();
      REQUIRE(keyframes.size() == 2);
      CHECK(std::get<double>(keyframes[0].Offset) == 0.5);
      const auto &rangeOffset = std::get<TimelineRangeOffset>(keyframes[1].Offset);
      CHECK(Narrow(rangeOffset.RangeName) == "cover");
      REQUIRE(rangeOffset.Offset);
      CHECK(Narrow(rangeOffset.Offset->ToString()) == "20%");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "KeyframeEffect: property-indexed keyframes",
                   "[Animation][KeyframeEffect]")
  {
    BoxFixture fixture;

    InputPropertyIndexedKeyframes keyframes;
    keyframes.Properties = {{u8"opacity", {u8"0", u8"0.5", u8"1"}}, {u8"marginLeft", {u8"0px", u8"30px"}}};
    keyframes.Easing = List<dom::DOMString> {u8"ease-in", u8"ease-out"};
    auto effect = Valid(fixture.Effect(krys::move(keyframes)));

    // The opacities at 0, 0.5 and 1 and the margins at 0 and 1 merge into three keyframes, and the easings
    // repeat.
    auto computed = effect->GetKeyframes();
    REQUIRE(computed.size() == 3);
    CHECK(computed[0].ComputedOffset == 0);
    CHECK(computed[1].ComputedOffset == 0.5);
    CHECK(computed[2].ComputedOffset == 1);
    CHECK(computed[0].StyleStrings.size() == 2);
    CHECK(computed[1].StyleStrings.size() == 1);
    CHECK(Narrow(computed[2].StyleStrings.at(css::PropertyId::MarginLeft)) == "30px");
    CHECK(Narrow(computed[0].Easing) == "ease-in");
    CHECK(Narrow(computed[1].Easing) == "ease-out");
    CHECK(Narrow(computed[2].Easing) == "ease-in");

    // Composite operations other than `auto` apply to the keyframes in order.
    InputPropertyIndexedKeyframes composite;
    composite.Properties = {{u8"opacity", {u8"0", u8"1"}}};
    composite.Composite =
      List<CompositeOperationOrAuto> {CompositeOperationOrAuto::Add, CompositeOperationOrAuto::Auto};
    auto compositeKeyframes = Valid(fixture.Effect(krys::move(composite)))->GetKeyframes();
    CHECK(compositeKeyframes[0].Composite == CompositeOperationOrAuto::Add);
    CHECK(compositeKeyframes[1].Composite == CompositeOperationOrAuto::Auto);

    // An unused easing must still parse.
    InputPropertyIndexedKeyframes unusedEasing;
    unusedEasing.Properties = {{u8"opacity", {u8"1"}}};
    unusedEasing.Easing = List<dom::DOMString> {u8"linear", u8"bounce"};
    CHECK(Error(fixture.Effect(krys::move(unusedEasing))) == dom::ExceptionCode::TypeError);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "KeyframeEffect: options, copies and setKeyframes()",
                   "[Animation][KeyframeEffect]")
  {
    BoxFixture fixture;

    KeyframeEffectOptions options;
    options.Duration = 500.0;
    options.Iterations = 2;
    options.Composite = CompositeOperation::Add;
    options.PseudoElement = u8"::before";
    auto effect =
      Valid(fixture.Effect(List<InputKeyframe> {Keyframe(nullptr, {{u8"opacity", u8"0"}})}, options));
    CHECK(effect->SpecifiedIterationDuration() == Seconds {0.5});
    CHECK(effect->Iterations() == 2);
    CHECK(effect->Composite() == CompositeOperation::Add);
    CHECK(effect->BindingsComposite() == CompositeOperation::Add);
    CHECK(effect->TargetsPseudoElement());
    CHECK(Narrow(effect->PseudoElement()) == "::before");

    SECTION("pseudo-elements")
    {
      CHECK(effect->SetPseudoElement(u8"::lemon").GetException().Code() == dom::ExceptionCode::SyntaxError);
      CHECK_FALSE(effect->SetPseudoElement(u8"").HasException());
      CHECK_FALSE(effect->TargetsPseudoElement());

      options.PseudoElement = u8"lemon";
      CHECK(Error(fixture.Effect(nullptr, options)) == dom::ExceptionCode::SyntaxError);
    }

    SECTION("a copy")
    {
      auto copy = KeyframeEffect::Create(Ref<KeyframeEffect>(effect));
      CHECK(copy->Target() == effect->Target());
      CHECK(copy->Iterations() == 2);
      CHECK(copy->Composite() == CompositeOperation::Add);
      REQUIRE(copy->GetKeyframes().size() == 1);
      CHECK(copy->ParsedKeyframes()[0].Style.get() != effect->ParsedKeyframes()[0].Style.get());
    }

    SECTION("setKeyframes()")
    {
      REQUIRE_FALSE(effect->SetKeyframes(*fixture.Document, nullptr).HasException());
      CHECK(effect->GetKeyframes().size() == 1);
      REQUIRE_FALSE(
        effect
          ->SetKeyframes(*fixture.Document, List<InputKeyframe> {Keyframe(nullptr, {{u8"opacity", u8"0"}}),
                                                                 Keyframe(nullptr, {{u8"opacity", u8"1"}})})
          .HasException());
      CHECK(effect->GetKeyframes().size() == 2);
      CHECK(effect->SetKeyframes(*fixture.Document, List<InputKeyframe> {Keyframe(2.0, {})}).HasException());
      CHECK(effect->GetKeyframes().size() == 2);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "KeyframeEffect: keyframes from C++ animate the target",
                   "[Animation][KeyframeEffect]")
  {
    BoxFixture fixture;
    auto effect = Valid(fixture.Effect(List<InputKeyframe> {Keyframe(nullptr, {{u8"opacity", u8"0"}}),
                                                            Keyframe(nullptr, {{u8"opacity", u8"1"}})}));
    auto animation = WebAnimation::Create(*fixture.Document, effect.get());
    REQUIRE_FALSE(animation->Play().HasException());
    fixture.StartAnimations();
    CHECK(fixture.Opacity(u8"box") == 0.0f);
    CHECK(effect->GetBlendingKeyframes().Size() == 2);

    fixture.Frame(250ms);
    CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.25f));
    fixture.Frame(500ms);
    CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.75f));
    animation->Cancel(WebAnimation::Silently::Yes);
  }
}
