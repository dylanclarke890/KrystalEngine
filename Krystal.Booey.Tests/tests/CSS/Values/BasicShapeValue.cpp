#include "Krystal.Booey/CSS/Values/BasicShapeValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    using LP = LengthPercentage<>;
    using NLP = LengthPercentage<NonNegative>;

    Position Centre()
    {
      return Position {TwoComponentPositionHorizontalVertical {{keywords::Center {}}, {keywords::Center {}}}};
    }
  }

  TEST_CASE_METHOD(ValueFixture, "BasicShapeValue::Create", "[CSS][Values][BasicShapeValue]")
  {
    BasicShape shape = CircleFunction {Circle {.Radius = NLP {1_css_px}}};
    auto value = BasicShapeValue::Create(shape);
    CHECK(value->IsBasicShapeValue());
    CHECK(value->GetShape() == shape);
  }

  TEST_CASE_METHOD(ValueFixture, "BasicShapeValue serialisation", "[CSS][Values][BasicShapeValue]")
  {
    CHECK(CssText(*BasicShapeValue::Create(BasicShape {CircleFunction {Circle {.Radius = NLP {1_css_px}}}}))
          == "circle(1px)");
    CHECK(CssText(*BasicShapeValue::Create(BasicShape {CircleFunction {
            Circle {.Radius = Circle::Extent {keywords::ClosestSide {}}, .Position = Centre()}}}))
          == "circle(at center center)");
    CHECK(CssText(*BasicShapeValue::Create(BasicShape {
            InsetFunction {Inset {.Insets = {LP {1_css_px}}, .Radii = BorderRadius::DefaultValue()}}}))
          == "inset(1px)");
    CHECK(CssText(*BasicShapeValue::Create(
            BasicShape {PathFunction {Path {.Data = {svg::SVGPathByteStream(u8"M 0 0")}}}}))
          == "path(\"M 0 0\")");
  }

  TEST_CASE_METHOD(ValueFixture, "BasicShapeValue equality and hashing", "[CSS][Values][BasicShapeValue]")
  {
    auto a = BasicShapeValue::Create(BasicShape {CircleFunction {Circle {.Radius = NLP {1_css_px}}}});
    auto b = BasicShapeValue::Create(BasicShape {CircleFunction {Circle {.Radius = NLP {1_css_px}}}});
    auto c = BasicShapeValue::Create(BasicShape {CircleFunction {Circle {.Radius = NLP {2_css_px}}}});
    auto ellipse = BasicShapeValue::Create(
      BasicShape {EllipseFunction {Ellipse {.Radii = {NLP {1_css_px}, NLP {1_css_px}}}}});

    CHECK(*a == *b);
    CHECK_FALSE(*a == *c);
    CHECK_FALSE(*a == *ellipse);

    // `BasicShapeValue` has no hash of its own.
    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));
  }

  TEST_CASE_METHOD(ValueFixture, "BasicShapeValue visitation", "[CSS][Values][BasicShapeValue]")
  {
    auto value = BasicShapeValue::Create(
      BasicShape {CircleFunction {Circle {.Radius = NLP {1_css_px}, .Position = Centre()}}});
    CHECK(Children(*value).empty());
    CHECK_FALSE(value->MayDependOnBaseUrl());
  }
}
