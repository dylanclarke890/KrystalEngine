#include "Krystal.Booey/Style/Interpolation.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/Style/ChangedAnimatableProperties.hpp"
#include "Krystal.Booey/Style/CustomProperty.hpp"
#include "Krystal.Booey/Style/Extractor.hpp"
#include "Krystal.Booey/Style/InterpolationClient.hpp"
#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumeric.hpp"

namespace krys::boo::style::tests
{
  namespace
  {
    struct TestClient final : interpolation::Client
    {
      dom::Document *TestDocument;
      const RenderStyle *Style;

      KRYS_NODISCARD dom::Document *Document() const noexcept override
      {
        return TestDocument;
      }
      KRYS_NODISCARD RenderElement *Renderer() const noexcept override
      {
        return nullptr;
      }
      KRYS_NODISCARD const RenderStyle &CurrentStyle() const noexcept override
      {
        return *Style;
      }
      KRYS_NODISCARD Maybe<uint32> TransformFunctionListPrefix() const noexcept override
      {
        return null;
      }
    };

    /// @brief Resolves #a and #b, and serialises @p property interpolated from #a's value to #b's at @p
    /// progress.
    struct InterpolationFixture : ResolverFixture
    {
      UniquePtr<RenderStyle> From;
      UniquePtr<RenderStyle> To;

      explicit InterpolationFixture(utf8_string &&styles) noexcept
          : ResolverFixture(u8"<!DOCTYPE html><style>" + styles
                            + u8"</style><div id=a></div><div id=b></div>"),
            From(Resolve(ById(u8"a")).Style), To(Resolve(ById(u8"b")).Style)
      {
      }

      KRYS_NODISCARD std::string Value(const RenderStyle &style, css::PropertyId property) noexcept
      {
        Extractor extractor(&ById(u8"a"));
        return css::tests::ToNarrow(extractor.PropertyValueSerialisationInStyle(
          style, property, css::DefaultSerialisationContext(), css::ValuePool::MainThreadPool()));
      }

      KRYS_NODISCARD UniquePtr<RenderStyle>
        Interpolated(const AnimatableCSSProperty &property, double progress,
                     CompositeOperation operation = CompositeOperation::Replace) noexcept
      {
        auto destination = RenderStyle::ClonePtr(*From);
        TestClient client;
        client.TestDocument = Document.get();
        client.Style = From.get();
        interpolation::Interpolate(property, *destination, *From, *To, progress, operation, client);
        return destination;
      }

      KRYS_NODISCARD std::string Blended(css::PropertyId property, double progress) noexcept
      {
        return Value(*Interpolated(property, progress), property);
      }
    };
  }

  TEST_CASE("Interpolation: numbers, lengths, colours and transforms blend", "[Style][Interpolation]")
  {
    InterpolationFixture fixture(u8R"(
      #a { opacity: 0.2; width: 10px; color: rgb(0, 0, 0); transform: translateX(0px); font-weight: 100;
           z-index: 1 }
      #b { opacity: 0.6; width: 30px; color: rgb(200, 100, 50); transform: translateX(100px); font-weight: 900;
           z-index: 10 }
    )");

    CHECK(fixture.Blended(css::PropertyId::Opacity, 0.5) == "0.4");
    CHECK(fixture.Blended(css::PropertyId::Width, 0.5) == "20px");
    CHECK(fixture.Blended(css::PropertyId::Width, 1.5) == "40px");
    CHECK(fixture.Blended(css::PropertyId::Color, 0.5) == "rgb(100, 50, 25)");
    // Without a renderer, the extractor serialises transforms as `none`: check the computed value.
    auto transformed = fixture.Interpolated(css::PropertyId::Transform, 0.25);
    CHECK(ResolverFixture::Serialised(*transformed, transformed->GetTransform()) == "translateX(25px)");
    CHECK(fixture.Blended(css::PropertyId::FontWeight, 0.5) == "500");
    CHECK(fixture.Blended(css::PropertyId::ZIndex, 0.5) == "6");

    // The ends are the values themselves.
    CHECK(fixture.Blended(css::PropertyId::Opacity, 0) == "0.2");
    CHECK(fixture.Blended(css::PropertyId::Opacity, 1) == "0.6");
  }

  TEST_CASE("Interpolation: discrete properties swap at 50%, display and visibility at the ends",
            "[Style][Interpolation]")
  {
    InterpolationFixture fixture(u8R"(
      #a { text-align: left; display: none; visibility: hidden; width: auto }
      #b { text-align: right; display: inline; visibility: visible; width: 10px }
    )");

    CHECK(fixture.Blended(css::PropertyId::TextAlign, 0.4) == "left");
    CHECK(fixture.Blended(css::PropertyId::TextAlign, 0.6) == "right");

    // Between none and another value, display takes the other value except at the very start.
    CHECK(fixture.Blended(css::PropertyId::Display, 0) == "none");
    CHECK(fixture.Blended(css::PropertyId::Display, 0.1) == "inline");
    CHECK(fixture.Blended(css::PropertyId::Display, 1) == "inline");

    // Visibility is visible as soon as the progress isn't 0.
    CHECK(fixture.Blended(css::PropertyId::Visibility, 0) == "hidden");
    CHECK(fixture.Blended(css::PropertyId::Visibility, 0.1) == "visible");

    // `auto` and a length can't be interpolated, so they swap too.
    TestClient client;
    CHECK_FALSE(
      interpolation::CanInterpolate(css::PropertyId::Width, *fixture.From, *fixture.To, *fixture.Document));
    CHECK(fixture.Blended(css::PropertyId::Width, 0.4) == "auto");
    CHECK(fixture.Blended(css::PropertyId::Width, 0.6) == "10px");
  }

  TEST_CASE("Interpolation: lists, shorthands and custom properties", "[Style][Interpolation]")
  {
    InterpolationFixture fixture(u8R"(
      @property --length { syntax: '<length>'; inherits: false; initial-value: 0px }
      #a { background-image: linear-gradient(red, red), linear-gradient(red, red);
           background-position: 0px 0px, 10px 10px; padding: 0px; --length: 0px; --tokens: a;
           box-shadow: 0px 0px 0px black }
      #b { background-image: linear-gradient(red, red), linear-gradient(red, red);
           background-position: 20px 20px, 30px 30px; padding: 10px 20px; --length: 20px; --tokens: b;
           box-shadow: 10px 20px 0px black }
    )");

    CHECK(fixture.Blended(css::PropertyId::BackgroundPositionX, 0.5) == "10px, 20px");
    CHECK(fixture.Blended(css::PropertyId::BoxShadow, 0.5) == "rgb(0, 0, 0) 5px 10px 0px 0px");

    // A shorthand interpolates its longhands.
    auto padding = fixture.Interpolated(css::PropertyId::Padding, 0.5);
    CHECK(fixture.Value(*padding, css::PropertyId::PaddingTop) == "5px");
    CHECK(fixture.Value(*padding, css::PropertyId::PaddingLeft) == "10px");

    // Registered custom properties interpolate by their syntax; the others swap.
    const css::CSSOMStringAtom lengthName {u8"--length"};
    auto length = fixture.Interpolated(lengthName, 0.25);
    const auto *lengthValue = length->CustomPropertyValue(lengthName);
    REQUIRE(lengthValue);
    REQUIRE(std::holds_alternative<CustomProperty::Value>(lengthValue->GetValue()));
    const auto &syntaxValue = std::get<CustomProperty::Value>(lengthValue->GetValue());
    REQUIRE(std::holds_alternative<Length<>>(syntaxValue));
    CHECK(std::get<Length<>>(syntaxValue).UnresolvedValue() == 5);

    const css::CSSOMStringAtom tokensName {u8"--tokens"};
    auto tokens = fixture.Interpolated(tokensName, 0.6);
    CHECK(tokens->CustomPropertyValue(tokensName) == fixture.To->CustomPropertyValue(tokensName));
  }

  TEST_CASE("Interpolation: what can be interpolated and added", "[Style][Interpolation]")
  {
    CHECK(interpolation::CanInterpolate(css::PropertyId::Opacity));
    CHECK(interpolation::CanInterpolate(css::PropertyId::Display));
    CHECK(interpolation::CanInterpolate(css::PropertyId::Margin));
    CHECK_FALSE(interpolation::CanInterpolate(css::PropertyId::Direction));
    CHECK_FALSE(interpolation::CanInterpolate(css::PropertyId::AnimationName));
    CHECK(interpolation::CanInterpolate(css::CSSOMStringAtom(u8"--anything")));

    CHECK(interpolation::IsAdditiveOrCumulative(css::PropertyId::Opacity));
    CHECK_FALSE(interpolation::IsAdditiveOrCumulative(css::PropertyId::StrokeDasharray));
  }

  TEST_CASE("Interpolation: equality and the changed animatable properties", "[Style][Interpolation]")
  {
    InterpolationFixture fixture(u8R"(
      #a { opacity: 0.2; width: 10px; color: red }
      #b { opacity: 0.6; width: 10px; color: red }
    )");

    CHECK_FALSE(
      interpolation::Equals(css::PropertyId::Opacity, *fixture.From, *fixture.To, *fixture.Document));
    CHECK(interpolation::Equals(css::PropertyId::Width, *fixture.From, *fixture.To, *fixture.Document));

    CSSPropertiesBitSet changed;
    ConservativelyCollectChangedAnimatableProperties(*fixture.From, *fixture.To, changed);
    CHECK(changed.Get(css::PropertyId::Opacity));
    CHECK_FALSE(changed.Get(css::PropertyId::Width));
    CHECK_FALSE(changed.Get(css::PropertyId::Color));
  }
}
