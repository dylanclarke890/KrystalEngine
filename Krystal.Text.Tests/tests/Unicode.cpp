#include "Krystal.Text/Unicode.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  TEST_CASE("Unicode(IsValidCodepoint)", "[Unicode]")
  {
    REQUIRE(Unicode::IsValidCodepoint(0x0000u) == true);    // Valid BMP codepoint
    REQUIRE(Unicode::IsValidCodepoint(0xD7FFu) == true);    // Valid BMP codepoint below surrogate range
    REQUIRE(Unicode::IsValidCodepoint(0xE000u) == true);    // Valid BMP codepoint above surrogate range
    REQUIRE(Unicode::IsValidCodepoint(0x10FFFFu) == true);  // Valid supplementary plane codepoint
    REQUIRE(Unicode::IsValidCodepoint(0x110000u) == false); // Above valid Unicode range
    REQUIRE(Unicode::IsValidCodepoint(0xD800u) == false);   // High surrogate
    REQUIRE(Unicode::IsValidCodepoint(0xDFFFu) == false);   // Low surrogate
  }

  TEST_CASE("Unicode(IsSurrogateCodepoint)", "[Unicode]")
  {
    REQUIRE(Unicode::IsSurrogateCodepoint(0xD800u) == true);  // High surrogate start
    REQUIRE(Unicode::IsSurrogateCodepoint(0xDBFFu) == true);  // High surrogate end
    REQUIRE(Unicode::IsSurrogateCodepoint(0xDC00u) == true);  // Low surrogate start
    REQUIRE(Unicode::IsSurrogateCodepoint(0xDFFFu) == true);  // Low surrogate end
    REQUIRE(Unicode::IsSurrogateCodepoint(0xD7FFu) == false); // Below surrogate range
    REQUIRE(Unicode::IsSurrogateCodepoint(0xE000u) == false); // Above surrogate range
  }

  TEST_CASE("Unicode(IsHighSurrogate)", "[Unicode]")
  {
    REQUIRE(Unicode::IsHighSurrogate(0xD800u) == true);  // High surrogate start
    REQUIRE(Unicode::IsHighSurrogate(0xDBFFu) == true);  // High surrogate end
    REQUIRE(Unicode::IsHighSurrogate(0xDC00u) == false); // Low surrogate start
    REQUIRE(Unicode::IsHighSurrogate(0xD7FFu) == false); // Below surrogate range
    REQUIRE(Unicode::IsHighSurrogate(0xE000u) == false); // Above surrogate range
  }

  TEST_CASE("Unicode(IsLowSurrogate)", "[Unicode]")
  {
    REQUIRE(Unicode::IsLowSurrogate(0xDC00u) == true);  // Low surrogate start
    REQUIRE(Unicode::IsLowSurrogate(0xDFFFu) == true);  // Low surrogate end
    REQUIRE(Unicode::IsLowSurrogate(0xD800u) == false); // High surrogate start
    REQUIRE(Unicode::IsLowSurrogate(0xD7FFu) == false); // Below surrogate range
    REQUIRE(Unicode::IsLowSurrogate(0xE000u) == false); // Above surrogate range
  }

  TEST_CASE("Unicode(IsBasicMultilingualPlaneCodepoint)", "[Unicode]")
  {
    REQUIRE(Unicode::IsBasicMultilingualPlaneCodepoint(Rune(0x0000u)) == true); // Start of BMP
    REQUIRE(Unicode::IsBasicMultilingualPlaneCodepoint(Rune(0xFFFFu)) == true); // End of BMP
    REQUIRE(Unicode::IsBasicMultilingualPlaneCodepoint(Rune(0x10000u))
            == false); // Start of supplementary planes
    REQUIRE(Unicode::IsBasicMultilingualPlaneCodepoint(Rune(0x10FFFFu))
            == false); // End of valid Unicode range
  }

  TEST_CASE("Unicode(ConvertSurrogatePair)", "[Unicode]")
  {
    constexpr uint16 high = 0xD83D; // High surrogate for U+1F600 (GRINNING FACE)
    constexpr uint16 low = 0xDE00;  // Low surrogate for U+1F600
    constexpr Rune expected(0x1F600u);

    REQUIRE(Unicode::ConvertSurrogatePair(high, low) == expected);
  }

  TEST_CASE("Unicode(ToUTF8)", "[Unicode]")
  {
    utf8_string result;

    // U+0041 LATIN CAPITAL LETTER A
    Unicode::ToUTF8(Rune(0x0041u), result);
    REQUIRE(result == u8"A");
    result.clear();

    // U+00A9 COPYRIGHT SIGN
    Unicode::ToUTF8(Rune(0x00A9u), result);
    REQUIRE(result == u8"©");
    result.clear();

    // U+20AC EURO SIGN
    Unicode::ToUTF8(Rune(0x20ACu), result);
    REQUIRE(result == u8"€");
    result.clear();

    // U+1F600 GRINNING FACE
    Unicode::ToUTF8(Rune(0x1F600u), result);
    REQUIRE(result == u8"😀");
  }

  TEST_CASE("Unicode(ByteCount)", "[Unicode]")
  {
    REQUIRE(Unicode::ByteCount(Rune(0x0041u)) == 1u);   // U+0041 LATIN CAPITAL LETTER A
    REQUIRE(Unicode::ByteCount(Rune(0x00A9u)) == 2u);   // U+00A9 COPYRIGHT SIGN
    REQUIRE(Unicode::ByteCount(Rune(0x20ACu)) == 3u);   // U+20AC EURO SIGN
    REQUIRE(Unicode::ByteCount(Rune(0x1F600u)) == 4u);  // U+1F600 GRINNING FACE
    REQUIRE(Unicode::ByteCount(Rune(0x10FFFFu)) == 4u); // Maximum valid Unicode codepoint
  }

  TEST_CASE("Unicode(TryGetNextCodepoint)", "[Unicode]")
  {
    utf8_string input = u8"A©€😀";
    size_t offset = 0;
    Rune codepoint;

    // U+0041 LATIN CAPITAL LETTER A
    REQUIRE(Unicode::TryGetNextCodepoint(input, offset, codepoint) == true);
    REQUIRE(codepoint == Rune(0x0041u));

    // U+00A9 COPYRIGHT SIGN
    REQUIRE(Unicode::TryGetNextCodepoint(input, offset, codepoint) == true);
    REQUIRE(codepoint == Rune(0x00A9u));

    // U+20AC EURO SIGN
    REQUIRE(Unicode::TryGetNextCodepoint(input, offset, codepoint) == true);
    REQUIRE(codepoint == Rune(0x20ACu));

    // U+1F600 GRINNING FACE
    REQUIRE(Unicode::TryGetNextCodepoint(input, offset, codepoint) == true);
    REQUIRE(codepoint == Rune(0x1F600u));

    // No more codepoints
    REQUIRE(Unicode::TryGetNextCodepoint(input, offset, codepoint) == false);
  }

  TEST_CASE("Unicode(ForEachCodepoint)", "[Unicode]")
  {
    utf8_string input = u8"A©€😀";

    utf8_string output;
    const auto Append = [&](Rune rune)
    {
      Unicode::ToUTF8(rune, output);
    };

    Unicode::ForEachCodepoint(input, Append);

    REQUIRE(input == output);
  }
}