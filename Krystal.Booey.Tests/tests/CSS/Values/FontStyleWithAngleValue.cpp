#include "Krystal.Booey/CSS/Values/FontStyleWithAngleValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    using ObliqueAngle = FontStyleWithAngleValue::ObliqueAngle;
  }

  TEST_CASE_METHOD(ValueFixture, "FontStyleWithAngleValue::Create", "[CSS][Values][FontStyleWithAngleValue]")
  {
    auto value = FontStyleWithAngleValue::Create(ObliqueAngle {14_css_deg});
    CHECK(value->IsFontStyleWithAngleValue());
    CHECK(value->GetObliqueAngle() == ObliqueAngle {14_css_deg});
  }

  TEST_CASE_METHOD(ValueFixture, "FontStyleWithAngleValue serialisation",
                   "[CSS][Values][FontStyleWithAngleValue]")
  {
    SECTION("A zero angle is normal")
    {
      CHECK(CssText(*FontStyleWithAngleValue::Create(ObliqueAngle {0_css_deg})) == "normal");
    }

    SECTION("Other angles are oblique")
    {
      CHECK(CssText(*FontStyleWithAngleValue::Create(ObliqueAngle {14_css_deg})) == "oblique 14deg");
      CHECK(CssText(*FontStyleWithAngleValue::Create(ObliqueAngle {-20_css_deg})) == "oblique -20deg");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "FontStyleWithAngleValue equality", "[CSS][Values][FontStyleWithAngleValue]")
  {
    auto a = FontStyleWithAngleValue::Create(ObliqueAngle {14_css_deg});
    auto b = FontStyleWithAngleValue::Create(ObliqueAngle {14_css_deg});
    auto c = FontStyleWithAngleValue::Create(ObliqueAngle {20_css_deg});

    CHECK(*a == *b);
    CHECK_FALSE(*a == *c);
  }

  TEST_CASE_METHOD(ValueFixture, "FontStyleWithAngleValue has no value children",
                   "[CSS][Values][FontStyleWithAngleValue]")
  {
    CHECK(Children(*FontStyleWithAngleValue::Create(ObliqueAngle {14_css_deg})).empty());
  }
}
