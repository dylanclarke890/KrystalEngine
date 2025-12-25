#include "Krystal.Text/Encodings/UTF32Encoding.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Text;

  TEST_CASE("UTF32EncodingLE(Valid)", "[Text][Encodings][ASCII]")
  {
    UTF32EncodingLE encoding;

    utf8_string text = u8"Hello, World! 世界";
    List<byte> encoded = encoding.Encode(text);
    utf8_string decoded = encoding.Decode(encoded);
    REQUIRE(decoded == text);
  }

  TEST_CASE("UTF32EncodingBE(Valid)", "[Text][Encodings][ASCII]")
  {
    UTF32EncodingBE encoding;

    utf8_string text = u8"Hello, World! 世界";
    List<byte> encoded = encoding.Encode(text);
    utf8_string decoded = encoding.Decode(encoded);
    REQUIRE(decoded == text);
  }
}