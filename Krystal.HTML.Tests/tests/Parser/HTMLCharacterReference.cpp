#include "Krystal.HTML/Parser/HTMLCharacterReference.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  TEST_CASE("HTMLCharacterReference(SearchNumericCharacterReferences)", "[HTML][CharacterReference]")
  {
    char32 test = 0x84;
    char32 expected = 0x201E; // DOUBLE LOW-9 QUOTATION MARK
    REQUIRE(Krys::HTML::SearchNumericCharacterReferences(test) == expected);

    test = 0x94;
    expected = 0x201D; // RIGHT DOUBLE QUOTATION MARK
    REQUIRE(Krys::HTML::SearchNumericCharacterReferences(test) == expected);

    test = 0x9C;
    expected = 0x0153; // LATIN SMALL LIGATURE OE
    REQUIRE(Krys::HTML::SearchNumericCharacterReferences(test) == expected);

    test = 0xFF; // Not mapped
    expected = 0x0000;
    REQUIRE(Krys::HTML::SearchNumericCharacterReferences(test) == expected);
  }

  TEST_CASE("HTMLCharacterReference(SearchNamedCharacterReferences)", "[HTML][CharacterReference]")
  {
    utf32_stringview target = utf32_stringview(U"&LongLeftRightArrow;");

    auto entries = Krys::HTML::SearchNamedCharacterReferences(Span(target.begin(), target.begin() + 4));
    REQUIRE(entries.size() == 6uz);

    entries = Krys::HTML::SearchNamedCharacterReferences(Span(target.begin(), target.begin() + 8), entries);
    REQUIRE(entries.size() == 2uz);

    entries = Krys::HTML::SearchNamedCharacterReferences(Span(target.begin(), target.end()), entries);
    REQUIRE(entries.size() == 1uz);
    REQUIRE(entries[0].Name == "&LongLeftRightArrow;");

    target = utf32_stringview(U"&NonExistentReference;");
    entries = Krys::HTML::SearchNamedCharacterReferences(Span(target.begin(), target.end()));
    REQUIRE(entries.size() == 0uz);
  }
}