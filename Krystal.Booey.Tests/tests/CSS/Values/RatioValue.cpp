#include "Krystal.Booey/CSS/Values/RatioValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE_METHOD(ValueFixture, "RatioValue::Create", "[CSS][Values][RatioValue]")
  {
    auto ratio = RatioValue::Create(Ratio {16_css_number, 9_css_number});
    CHECK(ratio->IsRatioValue());
    CHECK(ratio->GetRatio() == Ratio {16_css_number, 9_css_number});
    CHECK(CssText(*ratio) == "16 / 9");
  }

  TEST_CASE_METHOD(ValueFixture, "RatioValue serialisation", "[CSS][Values][RatioValue]")
  {
    CHECK(CssText(*RatioValue::Create(Ratio {1.5_css_number, 0.5_css_number})) == "1.5 / 0.5");
    // The denominator is always serialised, even when it was omitted.
    CHECK(CssText(*RatioValue::Create(Ratio {2_css_number})) == "2 / 1");
  }

  TEST_CASE_METHOD(ValueFixture, "RatioValue equality", "[CSS][Values][RatioValue]")
  {
    auto a = RatioValue::Create(Ratio {16_css_number, 9_css_number});
    CHECK(*a == *RatioValue::Create(Ratio {16_css_number, 9_css_number}));
    CHECK(*RatioValue::Create(Ratio {2_css_number})
          == *RatioValue::Create(Ratio {2_css_number, 1_css_number}));
    CHECK_FALSE(*a == *RatioValue::Create(Ratio {4_css_number, 9_css_number}));
    CHECK_FALSE(*a == *RatioValue::Create(Ratio {16_css_number, 10_css_number}));
  }

  TEST_CASE_METHOD(ValueFixture, "RatioValue has no child values without calc", "[CSS][Values][RatioValue]")
  {
    CHECK(Children(*RatioValue::Create(Ratio {16_css_number, 9_css_number})).empty());
  }
}
