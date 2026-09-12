#include "Krystal.Core/Text/Unicode.hpp"
#include <bit>
#include <catch_all.hpp>

namespace krys::text::tests
{
  TEST_CASE("Unicode::IsSurrogate", "[Text]")
  {
    REQUIRE(IsSurrogate(0xD800u) == true);  // High surrogate start
    REQUIRE(IsSurrogate(0xDBFFu) == true);  // High surrogate end
    REQUIRE(IsSurrogate(0xDC00u) == true);  // Low surrogate start
    REQUIRE(IsSurrogate(0xDFFFu) == true);  // Low surrogate end
    REQUIRE(IsSurrogate(0xD7FFu) == false); // Below surrogate range
    REQUIRE(IsSurrogate(0xE000u) == false); // Above surrogate range
  }

  TEST_CASE("Unicode::IsHighSurrogate", "[Text]")
  {
    REQUIRE(IsLeadSurrogate(0xD800u) == true);  // High surrogate start
    REQUIRE(IsLeadSurrogate(0xDBFFu) == true);  // High surrogate end
    REQUIRE(IsLeadSurrogate(0xDC00u) == false); // Low surrogate start
    REQUIRE(IsLeadSurrogate(0xD7FFu) == false); // Below surrogate range
    REQUIRE(IsLeadSurrogate(0xE000u) == false); // Above surrogate range
  }

  TEST_CASE("Unicode::IsLowSurrogate", "[Text]")
  {
    REQUIRE(IsTrailSurrogate(0xDC00u) == true);  // Low surrogate start
    REQUIRE(IsTrailSurrogate(0xDFFFu) == true);  // Low surrogate end
    REQUIRE(IsTrailSurrogate(0xD800u) == false); // High surrogate start
    REQUIRE(IsTrailSurrogate(0xD7FFu) == false); // Below surrogate range
    REQUIRE(IsTrailSurrogate(0xE000u) == false); // Above surrogate range
  }

  TEST_CASE("Unicode::UTF16CombineSurrogates", "[Text]")
  {
    constexpr char16 high = 0xD83D; // High surrogate for U+1F600 (GRINNING FACE)
    constexpr char16 low = 0xDE00;  // Low surrogate for U+1F600
    constexpr char32 expected = 0x1F600u;

    REQUIRE(UTF16CombineSurrogates(high, low) == expected);
  }

  TEST_CASE("Unicode::DecodeUTF8Length", "[Text]")
  {
    REQUIRE(DecodeUTF8Length(0x0041u) == 1u);   // U+0041 LATIN CAPITAL LETTER A
    REQUIRE(DecodeUTF8Length(0x00A9u) == 2u);   // U+00A9 COPYRIGHT SIGN
    REQUIRE(DecodeUTF8Length(0x20ACu) == 3u);   // U+20AC EURO SIGN
    REQUIRE(DecodeUTF8Length(0x1F600u) == 4u);  // U+1F600 GRINNING FACE
    REQUIRE(DecodeUTF8Length(0x10FFFFu) == 4u); // Maximum valid Unicode codepoint
  }
}