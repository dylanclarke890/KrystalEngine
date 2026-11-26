#include "Krystal.Text/CaseMapping.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::text::tests
{
  TEST_CASE("CaseMapping: simple mappings", "[Text][CaseMapping]")
  {
    CHECK(SimpleUppercase(U'a') == U'A');
    CHECK(SimpleUppercase(U'é') == U'É');
    CHECK(SimpleUppercase(U'1') == U'1');
    // A one-to-one mapping can't expand: ß has no simple uppercase.
    CHECK(SimpleUppercase(U'ß') == U'ß');

    // The digraph dž titlecases to Dž, not DŽ.
    CHECK(SimpleTitlecase(U'ǆ') == U'ǅ');
    CHECK(SimpleUppercase(U'ǆ') == U'Ǆ');

    CHECK(SimpleCaseFold(U'A') == U'a');
    CHECK(SimpleCaseFold(U'Σ') == U'σ');
  }

  TEST_CASE("CaseMapping: full mappings", "[Text][CaseMapping]")
  {
    CHECK(ToTitlecase(U'a') == U"A");
    CHECK(ToTitlecase(U'ß') == U"Ss");

    CHECK(ToUppercase(u8"straße", u8"") == u8"STRASSE");
    CHECK(ToLowercase(u8"ÉTÉ", u8"") == u8"été");

    // Turkish maps i to a dotted capital İ, and I to a dotless ı.
    CHECK(ToUppercase(u8"i", u8"tr") == u8"İ");
    CHECK(ToLowercase(u8"I", u8"tr") == u8"ı");
    CHECK(ToUppercase(u8"i", u8"en") == u8"I");
  }
}
