#include "Krystal.Core/Text/Encodings/Windows.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Text/Encodings/Decode.hpp"
#include "Krystal.Core/Text/Encodings/Encode.hpp"

namespace krys::text::tests
{
  TEST_CASE("Encodings::Windows1252: round trips through the table past ASCII", "[Text]")
  {
    auto decoded = Decode<Windows1252>(string("caf\xE9 \x80"));
    REQUIRE(decoded.Error == DecodeError::None);
    REQUIRE(decoded.Output == U"café €");

    // The bytes are the table's positions after ASCII.
    auto encoded = Encode<Windows1252>(decoded.Output);
    REQUIRE(encoded.Error == EncodeError::None);
    REQUIRE(encoded.Output == "caf\xE9 \x80");
  }
}
