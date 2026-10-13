#include "Krystal.Booey/CSS/Values/FontFeatureValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE_METHOD(ValueFixture, "FontFeatureValue::Create", "[CSS][Values][FontFeatureValue]")
  {
    auto value = FontFeatureValue::Create(FontFeatureTag("liga"), PrimitiveValue::CreateInteger(0.0));
    CHECK(value->IsFontFeatureValue());
    CHECK(value->Tag() == FontTag {'l', 'i', 'g', 'a'});
    CHECK(CssText(value->GetValue()) == "0");
  }

  TEST_CASE_METHOD(ValueFixture, "FontFeatureValue serialisation", "[CSS][Values][FontFeatureValue]")
  {
    SECTION("A value of 1 is implied, so it's omitted")
    {
      CHECK(CssText(*FontFeatureValue::Create(FontFeatureTag("liga"), PrimitiveValue::CreateInteger(1.0)))
            == "\"liga\"");
    }

    SECTION("Other values are written")
    {
      CHECK(CssText(*FontFeatureValue::Create(FontFeatureTag("liga"), PrimitiveValue::CreateInteger(0.0)))
            == "\"liga\" 0");
      CHECK(CssText(*FontFeatureValue::Create(FontFeatureTag("swsh"), PrimitiveValue::CreateInteger(2.0)))
            == "\"swsh\" 2");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "FontFeatureValue equality", "[CSS][Values][FontFeatureValue]")
  {
    auto a = FontFeatureValue::Create(FontFeatureTag("liga"), PrimitiveValue::CreateInteger(0.0));
    auto b = FontFeatureValue::Create(FontFeatureTag("liga"), PrimitiveValue::CreateInteger(0.0));
    auto otherTag = FontFeatureValue::Create(FontFeatureTag("kern"), PrimitiveValue::CreateInteger(0.0));
    auto otherValue = FontFeatureValue::Create(FontFeatureTag("liga"), PrimitiveValue::CreateInteger(1.0));

    CHECK(*a == *b);
    CHECK_FALSE(*a == *otherTag);
    CHECK_FALSE(*a == *otherValue);
  }

  TEST_CASE_METHOD(ValueFixture, "FontFeatureValue visits its value", "[CSS][Values][FontFeatureValue]")
  {
    auto value = FontFeatureValue::Create(FontFeatureTag("liga"), PrimitiveValue::CreateInteger(0.0));
    auto children = Children(*value);
    REQUIRE(children.size() == 1uz);
    CHECK(children[0uz] == &value->GetValue());
  }
}
