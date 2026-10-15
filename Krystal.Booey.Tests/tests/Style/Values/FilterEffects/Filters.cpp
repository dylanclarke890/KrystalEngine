#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperations.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/AppleColorFilter.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/Filter.hpp"

namespace krys::boo::style::tests
{
  using Catch::Approx;
  using css::PropertyId;
  using Type = FilterOperation::Type;

  TEST_CASE_METHOD(BuilderFixture, "Style Filter", "[Style][Values][FilterEffects]")
  {
    CHECK(Convert<Filter>(PropertyId::Filter, u8"none").IsNone());
    CHECK(Serialised(Convert<Filter>(PropertyId::Filter, u8"none")) == "none");

    auto filter =
      Convert<Filter>(PropertyId::Filter, u8"blur(4px) grayscale(50%) drop-shadow(1px 2px 3px black)");
    CHECK(Serialised(filter) == "blur(4px) grayscale(0.5) drop-shadow(rgb(0, 0, 0) 1px 2px 3px)");
    CHECK(Created(filter) == "blur(4px) grayscale(0.5) drop-shadow(rgb(0, 0, 0) 1px 2px 3px)");
    CHECK(filter.HasFilterThatMovesPixels());
    CHECK(filter.HasFilterOfType<Type::Grayscale>());
    CHECK_FALSE(filter.HasReferenceFilter());

    auto platform = ConvertToPlatform(filter);
    REQUIRE(platform.size() == 3);
    CHECK(platform[0]->GetType() == Type::Blur);
    CHECK(Downcast<BasicColorMatrixFilterOperation>(*platform[1]).Amount() == Approx(0.5));
    CHECK(platform[2]->IsDropShadowBase());
    CHECK(filter.Outsets() == platform.Outsets());

    SECTION("functions")
    {
      CHECK(Serialised(Convert<Filter>(PropertyId::Filter, u8"brightness(2) contrast(150%) invert()"))
            == "brightness(2) contrast(1.5) invert(1)");
      CHECK(Serialised(Convert<Filter>(PropertyId::Filter, u8"hue-rotate(1turn) opacity(0.25)"))
            == "hue-rotate(360deg) opacity(0.25)");
      CHECK(Serialised(Convert<Filter>(PropertyId::Filter, u8"saturate(3) sepia(1)"))
            == "saturate(3) sepia(1)");
    }

    SECTION("reference")
    {
      auto reference = Convert<Filter>(PropertyId::Filter, u8"url(#a)");
      CHECK(reference.IsReferenceFilter());
      CHECK(reference.HasReferenceFilter());
      CHECK_FALSE(Blending<Filter> {}.CanBlend(reference, filter, CompositeOperation::Replace));
    }

    SECTION("blending")
    {
      auto from = Convert<Filter>(PropertyId::Filter, u8"blur(0px)");
      auto to = Convert<Filter>(PropertyId::Filter, u8"blur(10px) brightness(3)");
      CHECK(Blending<Filter> {}.CanBlend(from, to, CompositeOperation::Replace));
      CHECK(Serialised(Blending<Filter> {}.Blend(from, to, BlendingContext {0.5}))
            == "blur(5px) brightness(2)");
      CHECK(Serialised(Blending<Filter> {}.Blend(Filter {css::keywords::None {}}, to, BlendingContext {0.5}))
            == "blur(5px) brightness(2)");
    }

    SECTION("backdrop-filter")
    {
      CHECK(Serialised(Convert<Filter>(PropertyId::BackdropFilter, u8"blur(2px)")) == "blur(2px)");
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style AppleColorFilter", "[Style][Values][FilterEffects]")
  {
    CHECK(Convert<AppleColorFilter>(PropertyId::AppleColorFilter, u8"none").IsNone());

    auto filter =
      Convert<AppleColorFilter>(PropertyId::AppleColorFilter, u8"apple-invert-lightness() brightness(50%)");
    CHECK(Serialised(filter) == "apple-invert-lightness() brightness(0.5)");
    CHECK(Created(filter) == "apple-invert-lightness() brightness(0.5)");

    // Light grey becomes dark grey, then halves in brightness.
    boo::Color color {SRGBA<float> {0.8f, 0.8f, 0.8f, 1.0f}};
    CHECK(filter.TransformColor(color));
    auto transformed = color.ToColorTypeLossy<SRGBA<float>>().Resolved();
    CHECK(transformed.Red == Approx(0.18).margin(0.01));
    CHECK(transformed.Red == Approx(transformed.Blue).margin(0.01));
  }
}
