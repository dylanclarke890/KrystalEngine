#include "Krystal.Text/Encodings/UTF8Encoding.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Text;

  TEST_CASE("UTF8Encoding(Valid)", "[Text][Encodings][ASCII]")
  {
    UTF8Encoding asciiEncoding;

    utf8_string text = u8"Hello, World! 123";
    List<byte> encoded = asciiEncoding.Encode(text);
    utf8_string decoded = asciiEncoding.Decode(encoded);
    REQUIRE(decoded == text);
  }
}