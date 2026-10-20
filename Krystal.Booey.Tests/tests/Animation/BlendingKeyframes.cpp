#include "Krystal.Booey/Animation/BlendingKeyframes.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleProperties+GettersInlines.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"

namespace krys::boo::tests
{
  namespace
  {
    using style::tests::ResolverFixture;

    /// @brief The keyframes of the `@keyframes` rule named @p name, resolved for #a.
    KRYS_NODISCARD BlendingKeyframes Keyframes(ResolverFixture &fixture, utf8_stringview name,
                                               bool *found = nullptr) noexcept
    {
      auto &element = fixture.ById(u8"a");
      auto style = fixture.Resolve(element).Style;
      auto rootStyle = fixture.Resolve(*fixture.Document->DocumentElement()).Style;
      BlendingKeyframes keyframes {KeyframesIdentifier {StringAtom {name}}};
      auto result = fixture.GetResolver().KeyframeStylesForAnimation(
        element, *style, {.ParentStyle = rootStyle.get(), .DocumentElementStyle = rootStyle.get()}, keyframes,
        nullptr);
      if (found)
      {
        *found = result;
      }
      return keyframes;
    }
  }

  TEST_CASE("BlendingKeyframes: keyframe styles from @keyframes", "[Animation][BlendingKeyframes]")
  {
    ResolverFixture fixture(u8R"(<!DOCTYPE html><style>
      @keyframes fade {
        from { opacity: 0 }
        50% { opacity: 0.5; animation-timing-function: steps(2); animation-composition: add }
        to { opacity: 1; margin-inline-start: 10px; --custom: 1 }
      }
      @keyframes merged { 0% { opacity: 0 } 0% { width: 5px } 100% { opacity: 1 } }
      #a { width: 40px; opacity: 0.3 }
    </style><div id=a></div>)");

    SECTION("each key gets a keyframe, in order, with its style and the properties it sets")
    {
      bool found = false;
      auto keyframes = Keyframes(fixture, u8"fade", &found);
      REQUIRE(found);
      REQUIRE(keyframes.Size() == 3);
      CHECK(keyframes.KeyframesName() == StringAtom {u8"fade"});
      CHECK(keyframes[0].Offset() == 0);
      CHECK(keyframes[1].Offset() == 0.5);
      CHECK(keyframes[2].Offset() == 1);

      REQUIRE(keyframes[1].Style());
      CHECK(keyframes[1].Style()->GetOpacity().Value.Value == 0.5f);
      // Properties the keyframe doesn't set keep the element's values.
      CHECK(keyframes[1].Style()->Width() == keyframes[0].Style()->Width());

      // The timing function and composite operation describe the keyframe; they aren't animated.
      CHECK(keyframes[1].GetTimingFunction());
      CHECK(keyframes[1].GetTimingFunction()->IsStepsTimingFunction());
      CHECK(keyframes[1].CompositeOperation() == CompositeOperation::Add);
      CHECK_FALSE(
        keyframes[1].AnimatesProperty(AnimatableCSSProperty {css::PropertyId::AnimationTimingFunction}));

      // Logical properties resolve to physical ones; custom properties are animated by name.
      CHECK(keyframes[2].AnimatesProperty(AnimatableCSSProperty {css::PropertyId::MarginLeft}));
      CHECK(keyframes[2].ContainsDirectionAwareProperty());
      CHECK(keyframes[2].AnimatesProperty(AnimatableCSSProperty {css::CSSOMStringAtom(u8"--custom")}));

      CHECK(keyframes.ContainsProperty(css::PropertyId::Opacity));
      CHECK(keyframes.ContainsAnimatableCSSProperty());
      CHECK_FALSE(keyframes.HasImplicitKeyframes());
      // With both 0% and 100% keyframes, no property has an implicit keyframe.
      CHECK_FALSE(keyframes.HasImplicitKeyframeForProperty(css::PropertyId::MarginLeft));
      CHECK_FALSE(keyframes.HasImplicitKeyframeForProperty(css::PropertyId::Opacity));
    }

    SECTION("keyframes with the same key, timing function and composite operation merge")
    {
      auto keyframes = Keyframes(fixture, u8"merged");
      REQUIRE(keyframes.Size() == 2);
      CHECK(keyframes[0].AnimatesProperty(AnimatableCSSProperty {css::PropertyId::Opacity}));
      CHECK(keyframes[0].AnimatesProperty(AnimatableCSSProperty {css::PropertyId::Width}));
    }

    SECTION("an unknown name has no keyframes")
    {
      bool found = true;
      auto keyframes = Keyframes(fixture, u8"missing", &found);
      CHECK_FALSE(found);
      CHECK(keyframes.IsEmpty());
    }
  }

  TEST_CASE("BlendingKeyframes: insertion, copies and metadata", "[Animation][BlendingKeyframes]")
  {
    BlendingKeyframes keyframes;
    CHECK(std::holds_alternative<uint64>(keyframes.Identifier()));
    CHECK(keyframes.KeyframesName() == StringAtom::Null());

    auto keyframe = [](double offset, css::PropertyId property)
    {
      BlendingKeyframe result(offset, RenderStyle::CreatePtr());
      result.AddProperty(property);
      return result;
    };

    keyframes.Insert(keyframe(1, css::PropertyId::Opacity));
    keyframes.Insert(keyframe(0.25, css::PropertyId::Width));
    keyframes.Insert(keyframe(1.5, css::PropertyId::Height)); // out of range: ignored
    REQUIRE(keyframes.Size() == 2);
    CHECK(keyframes[0].Offset() == 0.25);
    CHECK(keyframes.HasImplicitKeyframes());
    CHECK_FALSE(keyframes.ContainsProperty(css::PropertyId::Height));

    BlendingKeyframes copy;
    copy.CopyKeyframes(keyframes);
    CHECK(copy.Size() == 2);
    CHECK(copy[1].Style() != keyframes[1].Style());

    keyframes.UpdatedComputedOffsets([](const BlendingKeyframe::KeyframeOffset &offset)
                                     { return 1 - offset.Value; });
    CHECK(keyframes[0].Offset() == 0);
    CHECK(keyframes[1].Offset() == 0.75);
  }
}
