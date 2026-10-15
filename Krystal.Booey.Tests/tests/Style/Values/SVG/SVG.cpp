#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/ForVisitedLink.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGBaselineShift.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGGlyphOrientationHorizontal.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGGlyphOrientationVertical.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGPaint.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGPaintOrder.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGPathData.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGStrokeDasharray.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGStrokeDashoffset.hpp"

namespace krys::boo::style::tests
{
  using css::PropertyId;

  namespace
  {
    KRYS_NODISCARD SVGPaint ConvertPaint(BuilderFixture &fixture, utf8_stringview value) noexcept
    {
      auto parsed = fixture.Parse(PropertyId::Fill, value);
      fixture.CurrentProperty.Id = PropertyId::Fill;
      return ConvertCSSValueToStyle<SVGPaint>(*fixture.State, *parsed, ForVisitedLink::No);
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style SVG paint", "[Style][Values][SVG]")
  {
    CHECK(ConvertPaint(*this, u8"none").IsNone());

    auto red = ConvertPaint(*this, u8"red");
    REQUIRE(red.IsColor());
    CHECK(Serialised(red) == "rgb(255, 0, 0)");
    CHECK_FALSE(ContainsCurrentColor(red));

    auto current = ConvertPaint(*this, u8"currentcolor");
    CHECK(ContainsCurrentColor(current));
    CHECK(Style.DisallowsFastPathInheritance());

    auto url = ConvertPaint(*this, u8"url(#gradient)");
    CHECK(url.IsURL());
    CHECK(url.HasURL());
    CHECK_FALSE(url.HasColor());

    auto urlNone = ConvertPaint(*this, u8"url(#gradient) none");
    CHECK(urlNone.IsURLNone());

    auto urlColor = ConvertPaint(*this, u8"url(#gradient) blue");
    REQUIRE(urlColor.IsURLColor());
    CHECK(Serialised(*urlColor.TryAnyColor()) == "rgb(0, 0, 255)");

    SECTION("blending")
    {
      auto blue = ConvertPaint(*this, u8"blue");
      CHECK(Blending<SVGPaint> {}.CanBlend(red, blue));
      CHECK(Serialised(Blending<SVGPaint> {}.Blend(red, blue, Style, Style, BlendingContext {0.5}))
            == "rgb(128, 0, 128)");
      CHECK_FALSE(Blending<SVGPaint> {}.CanBlend(red, url));
      // Only colours compare for blending; everything else of the same type is equal.
      CHECK(Blending<SVGPaint> {}.Equals(url, ConvertPaint(*this, u8"url(#other)"), Style, Style));
      CHECK_FALSE(Blending<SVGPaint> {}.Equals(red, url, Style, Style));
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style SVG stroke dashes", "[Style][Values][SVG]")
  {
    CHECK(Convert<SVGStrokeDasharray>(PropertyId::StrokeDasharray, u8"none").IsNone());
    auto dashes = Convert<SVGStrokeDasharray>(PropertyId::StrokeDasharray, u8"1px, 2%, 3");
    CHECK(Serialised(dashes) == "1px, 2%, 3px");

    SECTION("blending repeats the shorter list to a common length")
    {
      auto from = Convert<SVGStrokeDasharray>(PropertyId::StrokeDasharray, u8"0px, 10px");
      auto to = Convert<SVGStrokeDasharray>(PropertyId::StrokeDasharray, u8"10px, 20px, 30px");
      CHECK(Serialised(Blending<SVGStrokeDasharray> {}.Blend(from, to, BlendingContext {0.5}))
            == "5px, 15px, 15px, 10px, 10px, 20px");

      // A multiple of the other's length is used as is.
      auto four = Convert<SVGStrokeDasharray>(PropertyId::StrokeDasharray, u8"10px, 20px, 30px, 40px");
      CHECK(Serialised(Blending<SVGStrokeDasharray> {}.Blend(from, four, BlendingContext {0.5}))
            == "5px, 15px, 15px, 25px");
    }

    CHECK(Serialised(Convert<SVGStrokeDashoffset>(PropertyId::StrokeDashoffset, u8"5")) == "5px");
    CHECK(Serialised(Convert<SVGStrokeDashoffset>(PropertyId::StrokeDashoffset, u8"10%")) == "10%");
  }

  TEST_CASE_METHOD(BuilderFixture, "Style SVG text and paint order", "[Style][Values][SVG]")
  {
    CHECK(Convert<SVGBaselineShift>(PropertyId::BaselineShift, u8"sub").IsSub());
    CHECK(Convert<SVGBaselineShift>(PropertyId::BaselineShift, u8"super").IsSuper());
    CHECK(Serialised(Convert<SVGBaselineShift>(PropertyId::BaselineShift, u8"10px")) == "10px");

    CHECK(Convert<SVGGlyphOrientationVertical>(PropertyId::GlyphOrientationVertical, u8"auto")
          == SVGGlyphOrientationVertical::Auto);
    CHECK(Convert<SVGGlyphOrientationVertical>(PropertyId::GlyphOrientationVertical, u8"90deg")
          == SVGGlyphOrientationVertical::Degrees90);
    CHECK(Convert<SVGGlyphOrientationHorizontal>(PropertyId::GlyphOrientationHorizontal, u8"180deg")
          == SVGGlyphOrientationHorizontal::Degrees180);

    CHECK(Convert<SVGPaintOrder>(PropertyId::PaintOrder, u8"normal").IsNormal());
    auto order = Convert<SVGPaintOrder>(PropertyId::PaintOrder, u8"markers stroke");
    CHECK(Serialised(order) == "markers stroke");
    auto types = order.PaintTypes();
    CHECK(types[0] == PaintType::Markers);
    CHECK(types[1] == PaintType::Stroke);
    CHECK(types[2] == PaintType::Fill);
  }

  TEST_CASE_METHOD(BuilderFixture, "Style SVG path data", "[Style][Values][SVG]")
  {
    CHECK(Convert<SVGPathData>(PropertyId::D, u8"none").IsNone());
    auto from = Convert<SVGPathData>(PropertyId::D, u8"path('M 0 0 L 10 10')");
    REQUIRE(from.IsPath());
    CHECK(Serialised(from) == "path(\"M 0 0 L 10 10\")");

    auto to = Convert<SVGPathData>(PropertyId::D, u8"path('M 10 10 L 30 50')");
    CHECK(Blending<SVGPathData> {}.CanBlend(from, to));
    CHECK(Serialised(Blending<SVGPathData> {}.Blend(from, to, BlendingContext {0.5}))
          == "path(\"M 5 5 L 20 30\")");
    CHECK_FALSE(Blending<SVGPathData> {}.CanBlend(from, Convert<SVGPathData>(PropertyId::D, u8"none")));
  }
}
