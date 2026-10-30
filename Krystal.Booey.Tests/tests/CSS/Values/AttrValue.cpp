#include "Krystal.Booey/CSS/Values/AttrValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<PrimitiveValue> String(CSSOMStringView value) noexcept
    {
      return PrimitiveValue::Create(CSSOMString {value});
    }
  }

  TEST_CASE_METHOD(ValueFixture, "AttrValue::Create", "[CSS][Values][AttrValue]")
  {
    SECTION("Without a fallback")
    {
      auto attr = AttrValue::Create(CSSOMString {u8"data-label"});
      CHECK(attr->IsAttrValue());
      CHECK(ToNarrow(attr->AttributeName()) == "data-label");
      CHECK_FALSE(attr->Fallback());
    }

    SECTION("With a fallback")
    {
      auto fallback = String(u8"none");
      auto attr = AttrValue::Create(CSSOMString {u8"data-label"}, fallback);
      CHECK(attr->Fallback() == fallback.get());
    }
  }

  TEST_CASE_METHOD(ValueFixture, "AttrValue serialisation", "[CSS][Values][AttrValue]")
  {
    CHECK(CssText(*AttrValue::Create(CSSOMString {u8"data-label"})) == "attr(data-label)");
    CHECK(CssText(*AttrValue::Create(CSSOMString {u8"data-label"}, String(u8"none")))
          == "attr(data-label, \"none\")");

    // An empty fallback isn't written.
    CHECK(CssText(*AttrValue::Create(CSSOMString {u8"data-label"}, String(u8""))) == "attr(data-label)");
  }

  TEST_CASE_METHOD(ValueFixture, "AttrValue equality", "[CSS][Values][AttrValue]")
  {
    auto plain = AttrValue::Create(CSSOMString {u8"data-label"});
    CHECK(*plain == *AttrValue::Create(CSSOMString {u8"data-label"}));
    CHECK_FALSE(*plain == *AttrValue::Create(CSSOMString {u8"data-other"}));

    auto withFallback = AttrValue::Create(CSSOMString {u8"data-label"}, String(u8"none"));
    CHECK(*withFallback == *AttrValue::Create(CSSOMString {u8"data-label"}, String(u8"none")));
    CHECK_FALSE(*withFallback == *AttrValue::Create(CSSOMString {u8"data-label"}, String(u8"other")));
    CHECK_FALSE(*withFallback == *AttrValue::Create(CSSOMString {u8"data-other"}, String(u8"none")));
    CHECK_FALSE(*withFallback == *plain);
    CHECK_FALSE(*plain == *withFallback);
  }

  TEST_CASE_METHOD(ValueFixture, "AttrValue doesn't visit its fallback", "[CSS][Values][AttrValue]")
  {
    CHECK(Children(*AttrValue::Create(CSSOMString {u8"data-label"}, String(u8"none"))).empty());
  }
}
