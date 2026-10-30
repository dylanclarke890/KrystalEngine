#include "Krystal.Booey/SVG/SVGLengthValue.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include <cmath>

namespace krys::boo::svg::tests
{
  using Catch::Approx;
  using style::tests::ResolverFixture;

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGLengthValue: parsing and serialising",
                   "[SVG][SVGLengthValue]")
  {
    SVGLengthValue length;
    CHECK(length.LengthType() == SVGLengthType::Number);
    CHECK(length.ValueAsString() == u8"0");
    CHECK(length.IsZero());

    auto check = [&](dom::DOMStringView string, SVGLengthType type, dom::DOMStringView serialised)
    {
      INFO(std::string(string.begin(), string.end()));
      REQUIRE_FALSE(length.SetValueAsString(string).HasException());
      CHECK(length.LengthType() == type);
      CHECK(length.ValueAsString() == serialised);
    };
    check(u8"10", SVGLengthType::Number, u8"10");
    check(u8" 2.5 ", SVGLengthType::Number, u8"2.5");
    check(u8"10px", SVGLengthType::Pixels, u8"10px");
    check(u8"50%", SVGLengthType::Percentage, u8"50%");
    check(u8"2em", SVGLengthType::Ems, u8"2em");
    check(u8"3ex", SVGLengthType::Exs, u8"3ex");
    check(u8"1cm", SVGLengthType::Centimeters, u8"1cm");
    check(u8"4mm", SVGLengthType::Millimeters, u8"4mm");
    check(u8"1in", SVGLengthType::Inches, u8"1in");
    check(u8"12pt", SVGLengthType::Points, u8"12pt");
    check(u8"1pc", SVGLengthType::Picas, u8"1pc");
    check(u8"2lh", SVGLengthType::Lh, u8"2lh");
    check(u8"2ch", SVGLengthType::Ch, u8"2ch");
    // Lengths SVG has no type for are kept, as unknown.
    check(u8"3rem", SVGLengthType::Unknown, u8"3rem");
    check(u8"1e2px", SVGLengthType::Pixels, u8"100px");
    // Numbers too big for a float become 0.
    check(u8"-45e58", SVGLengthType::Number, u8"0");

    REQUIRE_FALSE(length.SetValueAsString(u8"7px").HasException());
    CHECK(length.SetValueAsString(u8"") == dom::ExceptionCode::SyntaxError);
    CHECK(length.SetValueAsString(u8"   ") == dom::ExceptionCode::SyntaxError);
    CHECK(length.SetValueAsString(u8"10 px") == dom::ExceptionCode::SyntaxError);
    CHECK(length.SetValueAsString(u8"10px 2") == dom::ExceptionCode::SyntaxError);
    CHECK(length.SetValueAsString(u8"calc(1px + 2px)") == dom::ExceptionCode::SyntaxError);
    CHECK(length.SetValueAsString(u8"auto") == dom::ExceptionCode::SyntaxError);
    CHECK(length.SetValueAsString(u8"10deg") == dom::ExceptionCode::SyntaxError);
    CHECK(length.ValueAsString() == u8"7px");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGLengthValue: kinds and units", "[SVG][SVGLengthValue]")
  {
    CHECK_FALSE(SVGLengthValue(SVGLengthMode::Other, u8"5").IsRelative());
    CHECK_FALSE(SVGLengthValue(SVGLengthMode::Other, u8"5px").IsRelative());
    CHECK(SVGLengthValue(SVGLengthMode::Other, u8"5%").IsRelative());
    CHECK(SVGLengthValue(SVGLengthMode::Other, u8"5em").IsRelative());
    CHECK(SVGLengthValue(SVGLengthMode::Other, u8"5rem").IsRelative());
    CHECK(SVGLengthValue(SVGLengthMode::Other, u8"0px").IsZero());
    CHECK_FALSE(SVGLengthValue(SVGLengthMode::Other, u8"1px").IsZero());

    CHECK(SVGLengthValue(SVGLengthMode::Other, u8"50%").ValueAsPercentage() == 0.5f);
    CHECK(SVGLengthValue(SVGLengthMode::Other, u8"0.25").ValueAsPercentage() == 0.25f);

    SVGLengthValue pixels(3, SVGLengthType::Pixels);
    CHECK(pixels.ValueAsString() == u8"3px");
    pixels.SetValueInSpecifiedUnits(8);
    CHECK(pixels.ValueAsString() == u8"8px");
    CHECK(SVGLengthValue(3, SVGLengthType::Unknown).ValueAsString() == u8"3");
    CHECK(SVGLengthValue(SVGLengthMode::Width, u8"1px")
          == SVGLengthValue(1, SVGLengthType::Pixels, SVGLengthMode::Width));
    CHECK_FALSE(SVGLengthValue(SVGLengthMode::Width, u8"1px")
                == SVGLengthValue(SVGLengthMode::Height, u8"1px"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGLengthValue: construction from attributes",
                   "[SVG][SVGLengthValue]")
  {
    SVGParsingError error;
    auto length = SVGLengthValue::Construct(SVGLengthMode::Width, u8"20%", error);
    CHECK(error == SVGParsingError::None);
    CHECK(length.ValueAsString() == u8"20%");
    CHECK(length.LengthMode() == SVGLengthMode::Width);

    length =
      SVGLengthValue::Construct(SVGLengthMode::Width, u8"-1", error, SVGLengthNegativeValuesMode::Forbid);
    CHECK(error == SVGParsingError::ForbiddenNegativeValue);
    CHECK(length.ValueAsString() == u8"-1");

    length = SVGLengthValue::Construct(SVGLengthMode::Width, u8"bad", error);
    CHECK(error == SVGParsingError::ParsingFailed);

    // A fallback replaces a value that failed, or a missing (null) one.
    length = SVGLengthValue::Construct(SVGLengthMode::Width, u8"-1", error,
                                       SVGLengthNegativeValuesMode::Forbid, u8"100%");
    CHECK(error == SVGParsingError::ForbiddenNegativeValue);
    CHECK(length.ValueAsString() == u8"100%");
    length = SVGLengthValue::Construct(SVGLengthMode::Width, dom::DOMStringView {}, error,
                                       SVGLengthNegativeValuesMode::Allow, u8"100%");
    CHECK(length.ValueAsString() == u8"100%");
    length = SVGLengthValue::Construct(SVGLengthMode::Width, u8"5", error, SVGLengthNegativeValuesMode::Allow,
                                       u8"100%");
    CHECK(length.ValueAsString() == u8"5");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGLengthValue: user units", "[SVG][SVGLengthValue]")
  {
    // Without an element, percentages resolve against the given viewport, and absolute units by their ratios.
    SVGLengthContext context(nullptr, FloatSize(200, 100));
    CHECK(SVGLengthValue(SVGLengthMode::Width, u8"50%").Value(context) == 100);
    CHECK(SVGLengthValue(SVGLengthMode::Height, u8"50%").Value(context) == 50);
    CHECK(SVGLengthValue(SVGLengthMode::Other, u8"50%").Value(context)
          == Approx(0.5 * std::sqrt(200.0 * 200 + 100 * 100) / std::sqrt(2.0)));
    CHECK(SVGLengthValue(SVGLengthMode::Other, u8"12").Value(context) == 12);
    CHECK(SVGLengthValue(SVGLengthMode::Other, u8"1in").Value(context) == 96);
    CHECK(SVGLengthValue(SVGLengthMode::Other, u8"1cm").Value(context) == Approx(96 / 2.54));
    CHECK(SVGLengthValue(SVGLengthMode::Other, u8"3pt").Value(context) == 4);
    // Font-relative units need an element with a style.
    CHECK(SVGLengthValue(SVGLengthMode::Other, u8"2em").Value(context) == 0);
    CHECK(SVGLengthValue(SVGLengthMode::Other, u8"2ex").ValueForBindings(context)
          == dom::ExceptionCode::NotSupportedError);

    // Without a viewport, percentages don't resolve.
    SVGLengthContext noViewport(nullptr);
    CHECK(SVGLengthValue(SVGLengthMode::Width, u8"50%").ValueForBindings(noViewport)
          == dom::ExceptionCode::NotSupportedError);

    SECTION("Converting units")
    {
      SVGLengthValue length(SVGLengthMode::Width, u8"100");
      REQUIRE_FALSE(length.ConvertToSpecifiedUnits(context, SVGLengthType::Percentage).HasException());
      CHECK(length.ValueAsString() == u8"50%");
      REQUIRE_FALSE(length.ConvertToSpecifiedUnits(context, SVGLengthType::Inches).HasException());
      CHECK(length.ValueInSpecifiedUnits() == Approx(100.0 / 96));
      REQUIRE_FALSE(length.ConvertToSpecifiedUnits(context, SVGLengthType::Number).HasException());
      CHECK(length.ValueAsString() == u8"100");
      CHECK(length.ConvertToSpecifiedUnits(noViewport, SVGLengthType::Percentage)
            == dom::ExceptionCode::NotSupportedError);
    }

    SECTION("Setting user units")
    {
      SVGLengthValue length(SVGLengthMode::Width, u8"10%");
      REQUIRE_FALSE(length.SetValue(context, 50).HasException());
      CHECK(length.ValueAsString() == u8"25%");
      REQUIRE_FALSE(
        length.SetValue(context, 48, SVGLengthType::Inches, SVGLengthMode::Height).HasException());
      CHECK(length.ValueAsString() == u8"0.5in");
      CHECK(length.LengthMode() == SVGLengthMode::Height);
      CHECK(SVGLengthValue(context, 50, SVGLengthType::Percentage, SVGLengthMode::Height).ValueAsString()
            == u8"50%");
    }

    SECTION("Rectangles and points")
    {
      SVGLengthValue x(SVGLengthMode::Width, u8"10%");
      SVGLengthValue y(SVGLengthMode::Height, u8"20%");
      SVGLengthValue width(SVGLengthMode::Width, u8"50%");
      SVGLengthValue height(SVGLengthMode::Height, u8"5");
      // In the object bounding box, lengths are fractions of the box.
      CHECK(SVGLengthContext::ResolveRectangle(nullptr, SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX,
                                               FloatRect(10, 10, 100, 40), x, y, width, height)
            == FloatRect(20, 18, 50, 200));
      CHECK(SVGLengthContext::ResolveRectangle(nullptr, SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE,
                                               FloatRect(10, 10, 100, 40), x, y, width, height)
            == FloatRect(10, 8, 50, 5));
      CHECK(SVGLengthContext::ResolvePoint(nullptr, SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX, x, y)
            == FloatPoint(0.1f, 0.2f));
      CHECK(SVGLengthContext::ResolveLength(nullptr, SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX, height)
            == 5);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGLengthValue: font-relative units of an element",
                   "[SVG][SVGLengthValue]")
  {
    // Font-relative lengths resolve against the style of the nearest ancestor with a renderer.
    ResolverFixture fixture {u8"<!DOCTYPE html><body style='font-size:20px'><svg><rect id=r /></svg></body>"};
    (void)fixture.Document->UpdateLayout();
    auto &rect = Downcast<SVGElement>(fixture.ById(u8"r"));
    SVGLengthContext context(&rect);
    CHECK(SVGLengthValue(SVGLengthMode::Other, u8"2em").Value(context) == 40);
    CHECK(SVGLengthValue(SVGLengthMode::Other, u8"1rem").Value(context) == 16);

    SVGLengthValue length(SVGLengthMode::Other, u8"10px");
    REQUIRE_FALSE(length.ConvertToSpecifiedUnits(context, SVGLengthType::Ems).HasException());
    CHECK(length.ValueAsString() == u8"0.5em");
  }
}
