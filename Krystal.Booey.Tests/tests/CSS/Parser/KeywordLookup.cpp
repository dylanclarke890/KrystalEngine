#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Hash.hpp"

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
    CHECK(ValueIdFromName(u8"-webkit-link") == ValueId::WebkitLink);
    CHECK(ValueIdFromName(u8"-webkit-auto") == ValueId::Invalid);
    CHECK(ValueIdFromName(u8"not-a-keyword") == ValueId::Invalid);
  }

  TEST_CASE("ValueIdFromName(case-insensitive)", "[CSS][KeywordLookup]")
  {
    CHECK(ValueIdFromName(u8"RED") == ValueId::Red);
    CHECK(ValueIdFromName(u8"Auto") == ValueId::Auto);
    CHECK(ValueIdFromName(u8"-WEBKIT-LINK") == ValueId::WebkitLink);
  }

  TEST_CASE("ValueIdFromName(invalid input)", "[CSS][KeywordLookup]")
  {
    CHECK(ValueIdFromName(u8"") == ValueId::Invalid);
    CHECK(ValueIdFromName(CSSOMString(MaxValueKeywordLength + 1uz, u8'a')) == ValueId::Invalid);
    CHECK(ValueIdFromName(u8"réd") == ValueId::Invalid);
    CHECK(ValueIdFromName(CSSOMStringView(u8"re\0d", 4uz)) == ValueId::Invalid);
  }

  TEST_CASE("IsCustomPropertyName", "[CSS][KeywordLookup]")
  {
    CHECK(IsCustomPropertyName(u8"--x"));
    CHECK(IsCustomPropertyName(u8"---"));
    CHECK_FALSE(IsCustomPropertyName(u8"--"));
    CHECK_FALSE(IsCustomPropertyName(u8"-x-"));
    CHECK_FALSE(IsCustomPropertyName(u8"x--"));
    CHECK_FALSE(IsCustomPropertyName(u8""));
  }

  TEST_CASE("ToString(PropertyId) round trips", "[CSS][KeywordLookup]")
  {
    // Regression: the generated name table had no commas between entries, so every name after the first was
    // empty.
    for (auto id : AllProperties())
    {
      auto name = ToString(id);
      INFO(static_cast<uint32>(id));
      REQUIRE_FALSE(name.empty());
      CHECK(FindProperty(name) == id);
    }

    CHECK(ToString(PropertyId::BackgroundColor) == u8"background-color");
    CHECK(ToString(PropertyId::Invalid).empty());
  }

  TEST_CASE("PropertyId and ValueId hashes", "[CSS][KeywordLookup]")
  {
    CHECK(Hash::Combine(PropertyId::Color) == Hash::Combine(PropertyId::Color));
    CHECK(Hash::Combine(PropertyId::Color) != Hash::Combine(PropertyId::MarginTop));
    CHECK(DefaultHash<PropertyId>::Equal(PropertyId::Color, PropertyId::Color));

    CHECK(Hash::Combine(ValueId::Auto) == Hash::Combine(ValueId::Auto));
    CHECK(Hash::Combine(ValueId::Auto) != Hash::Combine(ValueId::None));
    CHECK(!DefaultHash<ValueId>::Equal(ValueId::Auto, ValueId::None));
  }
}
