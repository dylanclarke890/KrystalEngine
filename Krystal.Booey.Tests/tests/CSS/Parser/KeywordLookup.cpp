#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("PropertyIdFromName", "[CSS][KeywordLookup]")
  {
    CHECK(PropertyIdFromName(u8"color") == PropertyId::Color);
    CHECK(PropertyIdFromName(u8"margin-top") == PropertyId::MarginTop);
    CHECK(PropertyIdFromName(u8"not-a-property") == PropertyId::Invalid);
  }

  TEST_CASE("PropertyIdFromName(case-insensitive)", "[CSS][KeywordLookup]")
  {
    CHECK(PropertyIdFromName(u8"COLOR") == PropertyId::Color);
    CHECK(PropertyIdFromName(u8"Margin-Top") == PropertyId::MarginTop);
  }

  TEST_CASE("PropertyIdFromName(invalid input)", "[CSS][KeywordLookup]")
  {
    CHECK(PropertyIdFromName(u8"") == PropertyId::Invalid);
    CHECK(PropertyIdFromName(CSSOMString(MaxPropertyNameLength + 1uz, u8'a')) == PropertyId::Invalid);
    CHECK(PropertyIdFromName(u8"colör") == PropertyId::Invalid);
    CHECK(PropertyIdFromName(CSSOMStringView(u8"col\0or", 6uz)) == PropertyId::Invalid);
  }

  TEST_CASE("ValueIdFromName", "[CSS][KeywordLookup]")
  {
    CHECK(ValueIdFromName(u8"red") == ValueId::Red);
    CHECK(ValueIdFromName(u8"auto") == ValueId::Auto);
    CHECK(ValueIdFromName(u8"-webkit-auto") == ValueId::WebkitAuto);
    CHECK(ValueIdFromName(u8"not-a-keyword") == ValueId::Invalid);
  }

  TEST_CASE("ValueIdFromName(case-insensitive)", "[CSS][KeywordLookup]")
  {
    CHECK(ValueIdFromName(u8"RED") == ValueId::Red);
    CHECK(ValueIdFromName(u8"Auto") == ValueId::Auto);
    CHECK(ValueIdFromName(u8"-WEBKIT-AUTO") == ValueId::WebkitAuto);
  }

  TEST_CASE("ValueIdFromName(invalid input)", "[CSS][KeywordLookup]")
  {
    CHECK(ValueIdFromName(u8"") == ValueId::Invalid);
    CHECK(ValueIdFromName(CSSOMString(MaxValueKeywordLength + 1uz, u8'a')) == ValueId::Invalid);
    CHECK(ValueIdFromName(u8"réd") == ValueId::Invalid);
    CHECK(ValueIdFromName(CSSOMStringView(u8"re\0d", 4uz)) == ValueId::Invalid);
  }

  TEST_CASE("ValueIdFromName(-apple- prefix)", "[CSS][KeywordLookup]")
  {
    // Legacy -apple- keywords map to their -webkit- equivalents.
    CHECK(ValueIdFromName(u8"-apple-auto") == ValueId::WebkitAuto);
    CHECK(ValueIdFromName(u8"-Apple-Auto") == ValueId::WebkitAuto);
    CHECK(ValueIdFromName(u8"-apple-linear-gradient") == ValueId::WebkitLinearGradient);

    // Except -apple-system, -apple-pay* and -apple-wireless*.
    CHECK(ValueIdFromName(u8"-apple-system") == ValueId::Invalid);
    CHECK(ValueIdFromName(u8"-apple-pay-button") == ValueId::Invalid);
    CHECK(ValueIdFromName(u8"-apple-wireless-playback") == ValueId::Invalid);

    // Too short to be rewritten.
    CHECK(ValueIdFromName(u8"-apple-") == ValueId::Invalid);
    CHECK(ValueIdFromName(u8"-apple") == ValueId::Invalid);
  }

  TEST_CASE("ValueIdFromName(-apple- prefix at max length)", "[CSS][KeywordLookup]")
  {
    // The rewrite adds a character, so a max-length name must still fit.
    CSSOMString name = u8"-apple-" + CSSOMString(MaxValueKeywordLength - 7uz, u8'a');
    REQUIRE(name.size() == MaxValueKeywordLength);
    CHECK(ValueIdFromName(name) == ValueId::Invalid);
  }
}
