#include "Krystal.Text/Encodings/UTF/UTF16.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys;

  TEST_CASE("UTF16EncodingLE(Valid)", "[Text][Encodings][ASCII]")
  {
    UTF16EncodingLE encoding;

    utf8_string text = u8"Hello, World! 世界";

    List<byte> encoded;
    encoding.Encode(text, encoded);

    utf8_string decoded;
    encoding.Decode(encoded, decoded);

    REQUIRE(decoded == text);
  }

  TEST_CASE("UTF16EncodingBE(Valid)", "[Text][Encodings][ASCII]")
  {
    UTF16EncodingLE encoding;

    utf8_string text = u8"Hello, World! 世界";

    List<byte> encoded;
    encoding.Encode(text, encoded);

    utf8_string decoded;
    encoding.Decode(encoded, decoded);

    REQUIRE(decoded == text);
  }
}