#include "Krystal.Core/Text/ParsingUtilities.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::text::tests
{
  TEST_CASE("ParsingUtilities: skipping", "[Text][ParsingUtilities]")
  {
    utf8_stringview buffer = u8"svgView(viewBox(0 0 1 1))";

    CHECK_FALSE(SkipCharactersExactly(buffer, u8"svgview"));
    CHECK(SkipCharactersExactly(buffer, u8"svgView"));
    CHECK(buffer == u8"(viewBox(0 0 1 1))");

    CHECK_FALSE(SkipExactly(buffer, u8')'));
    CHECK(SkipExactly(buffer, u8'('));
    CHECK(buffer == u8"viewBox(0 0 1 1))");

    // Up to the delimiter, which isn't consumed.
    SkipUntil(buffer, u8'(');
    CHECK(buffer == u8"(0 0 1 1))");
    SkipUntil(buffer, u8')');
    CHECK(buffer == u8"))");

    // Without the delimiter, everything.
    SkipUntil(buffer, u8';');
    CHECK(buffer.empty());
    CHECK_FALSE(SkipExactly(buffer, u8';'));
  }
}
