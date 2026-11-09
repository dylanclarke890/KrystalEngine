#include "Krystal.Core/Text/Encodings/ISO-8859.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Text/Encodings/Decode.hpp"
#include "Krystal.Core/Text/Encodings/Encode.hpp"

namespace krys::text::tests
{
  TEST_CASE("Encodings::ISO_8859_7: bytes past ASCII are looked up in the table", "[Text]")
  {
    SECTION("Decoding")
    {
      auto result = Decode<ISO_8859_7>(string("A\xA1\xD3\xFE"));
      REQUIRE(result.Error == DecodeError::None);
      REQUIRE(result.Output == U"A‘Σώ");
    }

    SECTION("Encoding")
    {
      auto result = Encode<ISO_8859_7>(utf32_string(U"A‘Σώ"));
      REQUIRE(result.Error == EncodeError::None);
      REQUIRE(result.Output == "A\xA1\xD3\xFE");
    }
  }

  TEST_CASE("Encodings::ISO_8859_7: an unassigned byte is an invalid sequence", "[Text]")
  {
    // 0xAE, 0xD2 and 0xFF are unassigned.
    for (auto byte : {'\xAE', '\xD2', '\xFF'})
    {
      auto result =
        Decode<ISO_8859_7, DecodePolicy {.OnInvalidSequence = EncodingErrorAction::Stop}>(string(1uz, byte));
      CHECK(result.Error == DecodeError::InvalidSequence);
      CHECK(result.Output.empty());
    }
  }

  TEST_CASE("Encodings::ISO_8859_7: a code point outside the table is unmappable", "[Text]")
  {
    constexpr EncodePolicy stop {.OnUnmappableCodePoint = EncodingErrorAction::Stop};
    CHECK(Encode<ISO_8859_7, stop>(utf32_string(U"é")).Error == EncodeError::UnmappableCodePoint);
    // Past the last code point, which unassigned bytes hold.
    CHECK(Encode<ISO_8859_7, stop>(utf32_string(1uz, static_cast<char32_t>(0x110000))).Error
          == EncodeError::UnmappableCodePoint);
  }

  TEST_CASE("Encodings::ISO_8859_6: most of the upper half is unassigned", "[Text]")
  {
    REQUIRE(Decode<ISO_8859_6>(string("\xC1")).Output == U"ء");
    REQUIRE(
      Decode<ISO_8859_6, DecodePolicy {.OnInvalidSequence = EncodingErrorAction::Stop}>(string("\xA1")).Error
      == DecodeError::InvalidSequence);
    REQUIRE(Encode<ISO_8859_6>(utf32_string(U"ء")).Output == "\xC1");
  }
}
