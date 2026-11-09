#include "Krystal.Core/Text/ASCII.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::text::tests
{
  TEST_CASE("IsASCIICaselessEqual: strings, views and literals, ignoring ASCII case", "[Text][ASCII]")
  {
    // Literals need no view around them.
    STATIC_CHECK(IsASCIICaselessEqual(u8"Important", u8"important"));
    STATIC_CHECK(IsASCIICaselessEqual(utf8_stringview(u8"URL"), u8"url"));
    STATIC_CHECK(IsASCIICaselessEqual(u8"url", utf8_stringview(u8"uRl")));
    STATIC_CHECK_FALSE(IsASCIICaselessEqual(u8"url", u8"urls"));
    STATIC_CHECK_FALSE(IsASCIICaselessEqual(u8"url", u8"uri"));

    // Strings and code units of different types.
    CHECK(IsASCIICaselessEqual(utf8_string(u8"LAYER"), "layer"));
    CHECK(IsASCIICaselessEqual(string("Supports"), u8"supports"));

    // Only ASCII letters fold: not other characters a bit apart, nor non-ASCII.
    STATIC_CHECK_FALSE(IsASCIICaselessEqual(u8"@", u8"`"));
    STATIC_CHECK_FALSE(IsASCIICaselessEqual(u8"[", u8"{"));
    STATIC_CHECK_FALSE(IsASCIICaselessEqual(u8"text/css", u8"text\x0F"
                                                          u8"css"));
    STATIC_CHECK_FALSE(IsASCIICaselessEqual(u8"É", u8"é"));
  }
}
