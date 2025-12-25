#include "Krystal.Text/Encodings/UTF/UTF16Encoding.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Text;

  TEST_CASE("UTF16EncodingLE(Valid)", "[Text][Encodings][ASCII]")
  {
    UTF16EncodingLE encoding;

    utf8_string text = u8"Hello, World! 123";
    List<byte> encoded = encoding.Encode(text);
    utf8_string decoded = encoding.Decode(encoded);
    REQUIRE(decoded == text);
  }

  TEST_CASE("UTF16EncodingBE(Valid)", "[Text][Encodings][ASCII]")
  {
    UTF16EncodingLE encoding;

    utf8_string text = u8"Hello, World! 123";
    List<byte> encoded = encoding.Encode(text);
    utf8_string decoded = encoding.Decode(encoded);
    REQUIRE(decoded == text);
  }
}