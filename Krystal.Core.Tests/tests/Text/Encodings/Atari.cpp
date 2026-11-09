#include "Krystal.Core/Text/Encodings/Atari.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Text/Encodings/Decode.hpp"
#include "Krystal.Core/Text/Encodings/Encode.hpp"

namespace krys::text::tests
{
  TEST_CASE("Encodings::ATASCII: every byte is looked up in the table", "[Text]")
  {
    auto decoded = Decode<ATASCII>(string("\x01\xFC\xFE"));
    REQUIRE(decoded.Error == DecodeError::None);
    REQUIRE(decoded.Output == U"├-◀");

    // A code point that several bytes map to (here the inverse video copies) encodes to the first.
    auto encoded = Encode<ATASCII>(utf32_string(U"├-"));
    REQUIRE(encoded.Error == EncodeError::None);
    REQUIRE(encoded.Output == "\x01-");

    REQUIRE(
      Encode<ATASCII, EncodePolicy {.OnUnmappableCodePoint = EncodingErrorAction::Stop}>(utf32_string(U"é"))
        .Error
      == EncodeError::UnmappableCodePoint);
  }
}
