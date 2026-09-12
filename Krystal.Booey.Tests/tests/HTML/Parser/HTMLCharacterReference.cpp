#include "Krystal.Booey/HTML/Parser/HTMLCharacterReference.hpp"
#include <catch_all.hpp>

namespace krys::tests
{
  TEST_CASE("HTMLCharacterReference(SearchNumericCharacterReferences)", "[HTML][CharacterReference]")
  {
    char32 test = 0x84;
    char32 expected = 0x201E; // DOUBLE LOW-9 QUOTATION MARK
    REQUIRE(krys::boo::SearchNumericCharacterReferences(test) == expected);

    test = 0x94;
    expected = 0x201D; // RIGHT DOUBLE QUOTATION MARK
    REQUIRE(krys::boo::SearchNumericCharacterReferences(test) == expected);

    test = 0x9C;
    expected = 0x0153; // LATIN SMALL LIGATURE OE
    REQUIRE(krys::boo::SearchNumericCharacterReferences(test) == expected);

    test = 0xFF; // Not mapped
    expected = 0x0000;
    REQUIRE(krys::boo::SearchNumericCharacterReferences(test) == expected);
  }

  TEST_CASE("HTMLCharacterReference(SearchNamedCharacterReferences)", "[HTML][CharacterReference]")
  {
    utf32_stringview target = utf32_stringview(U"&LongLeftRightArrow;");

    auto entries = krys::boo::SearchNamedCharacterReferences(Span(target.begin(), target.begin() + 4));
    REQUIRE(entries.size() == 6uz);

    entries = krys::boo::SearchNamedCharacterReferences(Span(target.begin(), target.begin() + 8), entries);
    REQUIRE(entries.size() == 2uz);

    entries = krys::boo::SearchNamedCharacterReferences(Span(target.begin(), target.end()), entries);
    REQUIRE(entries.size() == 1uz);
    REQUIRE(entries[0].Name == "&LongLeftRightArrow;");

    target = utf32_stringview(U"&NonExistentReference;");
    entries = krys::boo::SearchNamedCharacterReferences(Span(target.begin(), target.end()));
    REQUIRE(entries.size() == 0uz);
  }
}