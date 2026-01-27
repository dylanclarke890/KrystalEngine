#include "Krystal.Text/StringConversion.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  TEST_CASE("StringConversion - UTF8 string to UTF32", "[Text][StringConversion]")
  {
    utf8_stringview input = u8"Hello, 世界!";
    utf32_string output = Text::ConvertToUTF32(input);

    utf32_string expected = U"Hello, 世界!";
    REQUIRE(output == expected);
  }

  TEST_CASE("StringConversion - const char8 span to UTF32", "[Text][StringConversion]")
  {
    utf8_stringview str = u8"Hello, 世界!";
    Span<const char8> input = Span(str.cbegin(), str.size());
    utf32_string output = Text::ConvertToUTF32(input);

    utf32_string expected = U"Hello, 世界!";
    REQUIRE(output == expected);
  }

  TEST_CASE("StringConversion - UTF32 to UTF8", "[Text][StringConversion]")
  {
    utf32_stringview input = U"Hello, 世界!";
    utf8_string output = Text::ConvertToUTF8(input);

    // Expected UTF-32 output
    utf8_string expected = u8"Hello, 世界!";
    REQUIRE(output == expected);
  }

  TEST_CASE("StringConversion - const char32 span to UTF8", "[Text][StringConversion]")
  {
    utf32_string str = U"Hello, 世界!";
    Span<const char32> input = Span(str.cbegin(), str.size());
    utf8_string output = Text::ConvertToUTF8(input);
    utf8_string expected = u8"Hello, 世界!";
    REQUIRE(output == expected);
  }
}