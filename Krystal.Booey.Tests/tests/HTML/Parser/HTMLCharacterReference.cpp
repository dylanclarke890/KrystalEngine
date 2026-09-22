#include "Krystal.Booey/HTML/Parser/HTMLCharacterReference.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::html::tests
{
  TEST_CASE("HTMLCharacterReference(SearchNumericCharacterReferences)", "[HTML][CharacterReference]")
  {
    char32 test = 0x84;
    char32 expected = 0x201E; // DOUBLE LOW-9 QUOTATION MARK
    REQUIRE(SearchNumericCharacterReferences(test) == expected);

    test = 0x94;
    expected = 0x201D; // RIGHT DOUBLE QUOTATION MARK
    REQUIRE(SearchNumericCharacterReferences(test) == expected);

    test = 0x9C;
    expected = 0x0153; // LATIN SMALL LIGATURE OE
    REQUIRE(SearchNumericCharacterReferences(test) == expected);

    test = 0xFF; // Not mapped
    expected = 0x0000;
    REQUIRE(SearchNumericCharacterReferences(test) == expected);
  }

  TEST_CASE("HTMLCharacterReference(SearchNamedCharacterReferences)", "[HTML][CharacterReference]")
  {
    utf32_stringview target = utf32_stringview(U"&LongLeftRightArrow;");

    auto entries = SearchNamedCharacterReferences(Span(target.begin(), target.begin() + 4));
    REQUIRE(entries.size() == 6uz);

    entries = SearchNamedCharacterReferences(Span(target.begin(), target.begin() + 8), entries);
    REQUIRE(entries.size() == 2uz);

    entries = SearchNamedCharacterReferences(Span(target.begin(), target.end()), entries);
    REQUIRE(entries.size() == 1uz);
    REQUIRE(entries[0].Name == "&LongLeftRightArrow;");

    target = utf32_stringview(U"&NonExistentReference;");
    entries = SearchNamedCharacterReferences(Span(target.begin(), target.end()));
    REQUIRE(entries.size() == 0uz);
  }
}