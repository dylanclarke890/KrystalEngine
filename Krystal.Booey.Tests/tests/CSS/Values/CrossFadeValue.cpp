#include "Krystal.Booey/CSS/Values/CrossFadeValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/NamedImageValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    Ref<CrossFadeValue> CrossFade(utf8_stringview from, utf8_stringview to, double percentage,
                                  bool isPrefixed = false)
    {
      return CrossFadeValue::Create(NamedImageValue::Create(CSSOMString {from}),
                                    NamedImageValue::Create(CSSOMString {to}),
                                    PrimitiveValue::Create(percentage, UnitType::Percentage), isPrefixed);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CrossFadeValue::Create", "[CSS][Values][CrossFadeValue]")
  {
    auto value = CrossFade(u8"a", u8"b", 50.0);
    CHECK(value->IsCrossFadeValue());
    CHECK(value->IsImageGeneratorValue());
    CHECK(value->IsImage());
    CHECK_FALSE(value->IsPrefixed());
    CHECK(CrossFade(u8"a", u8"b", 50.0, true)->IsPrefixed());
  }

  TEST_CASE_METHOD(ValueFixture, "CrossFadeValue serialisation", "[CSS][Values][CrossFadeValue]")
  {
    CHECK(CssText(*CrossFade(u8"a", u8"b", 50.0))
          == "cross-fade(-webkit-named-image(a), -webkit-named-image(b), 50%)");
    CHECK(CssText(*CrossFade(u8"a", u8"b", 25.0, true))
          == "-webkit-cross-fade(-webkit-named-image(a), -webkit-named-image(b), 25%)");

    // Either image can be `none`.
    auto withNone =
      CrossFadeValue::Create(PrimitiveValue::Create(ValueId::None), NamedImageValue::Create(u8"b"),
                             PrimitiveValue::Create(0.5, UnitType::Number));
    CHECK(CssText(*withNone) == "cross-fade(none, -webkit-named-image(b), 0.5)");
  }

  TEST_CASE_METHOD(ValueFixture, "CrossFadeValue equality", "[CSS][Values][CrossFadeValue]")
  {
    auto a = CrossFade(u8"a", u8"b", 50.0);
    CHECK(*a == *CrossFade(u8"a", u8"b", 50.0));
    CHECK_FALSE(*a == *CrossFade(u8"c", u8"b", 50.0));
    CHECK_FALSE(*a == *CrossFade(u8"a", u8"c", 50.0));
    CHECK_FALSE(*a == *CrossFade(u8"a", u8"b", 25.0));

    // The prefix isn't compared.
    CHECK(*a == *CrossFade(u8"a", u8"b", 50.0, true));

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
    CHECK(CssText(*children[0uz]) == "-webkit-named-image(a)");
    CHECK(CssText(*children[1uz]) == "-webkit-named-image(b)");
    CHECK(CssText(*children[2uz]) == "50%");

    CHECK_FALSE(value->MayDependOnBaseUrl());
  }
}
