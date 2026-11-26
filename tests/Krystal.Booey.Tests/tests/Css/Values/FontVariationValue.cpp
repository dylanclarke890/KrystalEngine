#include "Krystal.Booey/Css/Values/FontVariationValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Booey/Css/Values/FontFeatureValue.hpp"
#include "Krystal.Booey/Css/Values/PrimitiveValue.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE_METHOD(ValueFixture, "FontVariationValue::Create", "[CSS][Values][FontVariationValue]")
  {
    auto value =
      FontVariationValue::Create(FontFeatureTag("wght"), PrimitiveValue::Create(700.0, UnitType::Number));
    CHECK(Is<FontVariationValue>(*value));
    CHECK(value->Tag() == FontTag {'w', 'g', 'h', 't'});
    CHECK(CssText(value->GetValue()) == "700");
  }

  TEST_CASE_METHOD(ValueFixture, "FontVariationValue serialisation", "[CSS][Values][FontVariationValue]")
  {
    CHECK(CssText(*FontVariationValue::Create(FontFeatureTag("wght"),
                                              PrimitiveValue::Create(700.0, UnitType::Number)))
          == "\"wght\" 700");

    // Unlike `font-feature-settings`, a value of 1 is always written.
    CHECK(CssText(*FontVariationValue::Create(FontFeatureTag("slnt"), PrimitiveValue::CreateInteger(1.0)))
          == "\"slnt\" 1");
    CHECK(CssText(*FontVariationValue::Create(FontFeatureTag("opsz"),
                                              PrimitiveValue::Create(1.5, UnitType::Number)))
          == "\"opsz\" 1.5");
  }

  TEST_CASE_METHOD(ValueFixture, "FontVariationValue equality", "[CSS][Values][FontVariationValue]")
  {
    auto a =
      FontVariationValue::Create(FontFeatureTag("wght"), PrimitiveValue::Create(700.0, UnitType::Number));
    auto b =
      FontVariationValue::Create(FontFeatureTag("wght"), PrimitiveValue::Create(700.0, UnitType::Number));
    auto otherTag =
      FontVariationValue::Create(FontFeatureTag("wdth"), PrimitiveValue::Create(700.0, UnitType::Number));
    auto otherValue =
      FontVariationValue::Create(FontFeatureTag("wght"), PrimitiveValue::Create(400.0, UnitType::Number));
    auto feature =
      FontFeatureValue::Create(FontFeatureTag("wght"), PrimitiveValue::Create(700.0, UnitType::Number));

    CHECK(*a == *b);
    CHECK_FALSE(*a == *otherTag);
    CHECK_FALSE(*a == *otherValue);
    CHECK_FALSE(*a == *feature);
  }

  TEST_CASE_METHOD(ValueFixture, "FontVariationValue visits its value", "[CSS][Values][FontVariationValue]")
  {
    auto value =
      FontVariationValue::Create(FontFeatureTag("wght"), PrimitiveValue::Create(700.0, UnitType::Number));
    auto children = Children(*value);
    REQUIRE(children.size() == 1uz);
    CHECK(children[0uz] == &value->GetValue());
  }
}
