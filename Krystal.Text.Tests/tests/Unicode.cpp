#include "Krystal.Text/Unicode.hpp"
#include <bit>
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Text;

  TEST_CASE("Unicode(IsSurrogate)", "[Text][Unicode]")
  {
    REQUIRE(Unicode::IsSurrogate(0xD800u) == true);  // High surrogate start
    REQUIRE(Unicode::IsSurrogate(0xDBFFu) == true);  // High surrogate end
    REQUIRE(Unicode::IsSurrogate(0xDC00u) == true);  // Low surrogate start
    REQUIRE(Unicode::IsSurrogate(0xDFFFu) == true);  // Low surrogate end
    REQUIRE(Unicode::IsSurrogate(0xD7FFu) == false); // Below surrogate range
    REQUIRE(Unicode::IsSurrogate(0xE000u) == false); // Above surrogate range
  }

  TEST_CASE("Unicode(IsHighSurrogate)", "[Text][Unicode]")
  {
    REQUIRE(Unicode::IsLeadSurrogate(0xD800u) == true);  // High surrogate start
    REQUIRE(Unicode::IsLeadSurrogate(0xDBFFu) == true);  // High surrogate end
    REQUIRE(Unicode::IsLeadSurrogate(0xDC00u) == false); // Low surrogate start
    REQUIRE(Unicode::IsLeadSurrogate(0xD7FFu) == false); // Below surrogate range
    REQUIRE(Unicode::IsLeadSurrogate(0xE000u) == false); // Above surrogate range
  }

  TEST_CASE("Unicode(IsLowSurrogate)", "[Text][Unicode]")
  {
    REQUIRE(Unicode::IsTrailSurrogate(0xDC00u) == true);  // Low surrogate start
    REQUIRE(Unicode::IsTrailSurrogate(0xDFFFu) == true);  // Low surrogate end
    REQUIRE(Unicode::IsTrailSurrogate(0xD800u) == false); // High surrogate start
    REQUIRE(Unicode::IsTrailSurrogate(0xD7FFu) == false); // Below surrogate range
    REQUIRE(Unicode::IsTrailSurrogate(0xE000u) == false); // Above surrogate range
  }

  TEST_CASE("Unicode(UTF16CombineSurrogates)", "[Text][Unicode]")
  {
    constexpr char16 high = 0xD83D; // High surrogate for U+1F600 (GRINNING FACE)
    constexpr char16 low = 0xDE00;  // Low surrogate for U+1F600
    constexpr char32 expected = 0x1F600u;

    REQUIRE(Unicode::UTF16CombineSurrogates(high, low) == expected);
  }

  TEST_CASE("Unicode(UTF8DecodeLength)", "[Text][Unicode]")
  {
    REQUIRE(Unicode::UTF8DecodeLength(0x0041u) == 1u);   // U+0041 LATIN CAPITAL LETTER A
    REQUIRE(Unicode::UTF8DecodeLength(0x00A9u) == 2u);   // U+00A9 COPYRIGHT SIGN
    REQUIRE(Unicode::UTF8DecodeLength(0x20ACu) == 3u);   // U+20AC EURO SIGN
    REQUIRE(Unicode::UTF8DecodeLength(0x1F600u) == 4u);  // U+1F600 GRINNING FACE
    REQUIRE(Unicode::UTF8DecodeLength(0x10FFFFu) == 4u); // Maximum valid Unicode codepoint
  }
}