#include "Krystal.Booey/CSS/Values/CrossFadeValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    Ref<CrossFadeValue> CrossFade(utf8_stringview from, utf8_stringview to, double percentage)
    {
      return CrossFadeValue::Create(TestImage(from), TestImage(to),
                                    PrimitiveValue::Create(percentage, UnitType::Percentage));
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CrossFadeValue::Create", "[CSS][Values][CrossFadeValue]")
  {
    auto value = CrossFade(u8"a", u8"b", 50.0);
    CHECK(value->IsCrossFadeValue());
    CHECK(value->IsImageGeneratorValue());
    CHECK(value->IsImage());
  }

  TEST_CASE_METHOD(ValueFixture, "CrossFadeValue serialisation", "[CSS][Values][CrossFadeValue]")
  {
    CHECK(CssText(*CrossFade(u8"a", u8"b", 50.0))
          == "cross-fade(url(\"https://example.com/a.png\"), url(\"https://example.com/b.png\"), 50%)");

    // Either image can be `none`.
    auto withNone = CrossFadeValue::Create(PrimitiveValue::Create(ValueId::None), TestImage(u8"b"),
                                           PrimitiveValue::Create(0.5, UnitType::Number));
    CHECK(CssText(*withNone) == "cross-fade(none, url(\"https://example.com/b.png\"), 0.5)");
  }

  TEST_CASE_METHOD(ValueFixture, "CrossFadeValue equality", "[CSS][Values][CrossFadeValue]")
  {
    auto a = CrossFade(u8"a", u8"b", 50.0);
    CHECK(*a == *CrossFade(u8"a", u8"b", 50.0));
    CHECK_FALSE(*a == *CrossFade(u8"c", u8"b", 50.0));
    CHECK_FALSE(*a == *CrossFade(u8"a", u8"c", 50.0));
    CHECK_FALSE(*a == *CrossFade(u8"a", u8"b", 25.0));

    CHECK(a->EqualInputImages(*CrossFade(u8"a", u8"b", 25.0)));
    CHECK_FALSE(a->EqualInputImages(*CrossFade(u8"b", u8"a", 50.0)));

    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));
  }

  TEST_CASE_METHOD(ValueFixture, "CrossFadeValue children", "[CSS][Values][CrossFadeValue]")
  {
    auto value = CrossFade(u8"a", u8"b", 50.0);
    auto children = Children(*value);
    REQUIRE(children.size() == 3uz);
    CHECK(CssText(*children[0uz]) == "url(\"https://example.com/a.png\")");
    CHECK(CssText(*children[1uz]) == "url(\"https://example.com/b.png\")");
    CHECK(CssText(*children[2uz]) == "50%");

    CHECK_FALSE(value->MayDependOnBaseUrl());
  }
}
