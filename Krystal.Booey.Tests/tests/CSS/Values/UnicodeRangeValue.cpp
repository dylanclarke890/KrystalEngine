#include "Krystal.Booey/CSS/Values/UnicodeRangeValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE_METHOD(ValueFixture, "UnicodeRangeValue::Create", "[CSS][Values][UnicodeRangeValue]")
  {
    auto value = UnicodeRangeValue::Create(U'\x0025', U'\x00FF');
    CHECK(value->IsUnicodeRangeValue());
    CHECK(value->From() == U'\x0025');
    CHECK(value->To() == U'\x00FF');
  }

  TEST_CASE_METHOD(ValueFixture, "UnicodeRangeValue serialisation", "[CSS][Values][UnicodeRangeValue]")
  {
    SECTION("A range is lowercase hex without leading zeros")
    {
      CHECK(CssText(*UnicodeRangeValue::Create(U'\x0025', U'\x00FF')) == "U+25-ff");
      CHECK(CssText(*UnicodeRangeValue::Create(U'\x0000', U'\x10FFFF')) == "U+0-10ffff");
    }

    SECTION("A single code point has no end")
    {
      CHECK(CssText(*UnicodeRangeValue::Create(U'\x4E00', U'\x4E00')) == "U+4e00");
      CHECK(CssText(*UnicodeRangeValue::Create(U'\x0000', U'\x0000')) == "U+0");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "UnicodeRangeValue equality", "[CSS][Values][UnicodeRangeValue]")
  {
    auto a = UnicodeRangeValue::Create(U'\x0025', U'\x00FF');
    CHECK(*a == *UnicodeRangeValue::Create(U'\x0025', U'\x00FF'));
    CHECK_FALSE(*a == *UnicodeRangeValue::Create(U'\x0026', U'\x00FF'));
    CHECK_FALSE(*a == *UnicodeRangeValue::Create(U'\x0025', U'\x00FE'));
  }
}
