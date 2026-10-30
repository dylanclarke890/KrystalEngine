#include "Krystal.Booey/CSS/Values/RayValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    RayFunction MakeRay(Angle<> angle, RaySize size = keywords::ClosestSide {})
    {
      return RayFunction {.Parameters = Ray {.Angle = krys::move(angle), .Size = size}};
    }
  }

  TEST_CASE_METHOD(ValueFixture, "RayValue::Create", "[CSS][Values][RayValue]")
  {
    SECTION("Without a coordinate box")
    {
      auto value = RayValue::Create(MakeRay(45_css_deg));
      CHECK(value->IsRayValue());
      CHECK(value->GetRay() == MakeRay(45_css_deg));
      CHECK(value->CoordinateBox() == CSSBoxType::BoxMissing);
    }

    SECTION("With a coordinate box")
    {
      auto value = RayValue::Create(MakeRay(45_css_deg), CSSBoxType::PaddingBox);
      CHECK(value->CoordinateBox() == CSSBoxType::PaddingBox);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "RayValue serialisation", "[CSS][Values][RayValue]")
  {
    CHECK(CssText(*RayValue::Create(MakeRay(45_css_deg))) == "ray(45deg)");
    CHECK(CssText(*RayValue::Create(MakeRay(45_css_deg, keywords::Sides {}))) == "ray(45deg sides)");

    // The coordinate box follows the function.
    CHECK(CssText(*RayValue::Create(MakeRay(45_css_deg), CSSBoxType::MarginBox)) == "ray(45deg) margin-box");
    CHECK(CssText(*RayValue::Create(MakeRay(45_css_deg), CSSBoxType::BorderBox)) == "ray(45deg) border-box");
    CHECK(CssText(*RayValue::Create(MakeRay(45_css_deg), CSSBoxType::PaddingBox))
          == "ray(45deg) padding-box");
    CHECK(CssText(*RayValue::Create(MakeRay(45_css_deg), CSSBoxType::ContentBox))
          == "ray(45deg) content-box");
    CHECK(CssText(*RayValue::Create(MakeRay(45_css_deg), CSSBoxType::FillBox)) == "ray(45deg) fill-box");
    CHECK(CssText(*RayValue::Create(MakeRay(45_css_deg), CSSBoxType::StrokeBox)) == "ray(45deg) stroke-box");
    CHECK(CssText(*RayValue::Create(MakeRay(45_css_deg), CSSBoxType::ViewBox)) == "ray(45deg) view-box");
  }

  TEST_CASE_METHOD(ValueFixture, "RayValue equality and hashing", "[CSS][Values][RayValue]")
  {
    auto a = RayValue::Create(MakeRay(45_css_deg), CSSBoxType::BorderBox);
    auto b = RayValue::Create(MakeRay(45_css_deg), CSSBoxType::BorderBox);

    CHECK(*a == *b);
    CHECK_FALSE(*a == *RayValue::Create(MakeRay(90_css_deg), CSSBoxType::BorderBox));
    CHECK_FALSE(*a == *RayValue::Create(MakeRay(45_css_deg), CSSBoxType::ContentBox));
    CHECK_FALSE(*a == *RayValue::Create(MakeRay(45_css_deg)));

    // `RayValue` has no hash of its own.
    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));
  }

  TEST_CASE_METHOD(ValueFixture, "RayValue visitation", "[CSS][Values][RayValue]")
  {
    CHECK(Children(*RayValue::Create(MakeRay(45_css_deg), CSSBoxType::BorderBox)).empty());
  }
}
