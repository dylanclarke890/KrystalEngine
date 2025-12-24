#include "Krystal.Text/Encodings/ASCIIEncoding.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Text;

  TEST_CASE("ASCIIEncoding(Valid)", "[Text][Encodings][ASCII]")
  {
    ASCIIEncoding asciiEncoding;

    utf8_string text = u8"Hello, World! 123";
    List<byte> encoded = asciiEncoding.Encode(text);
    utf8_string decoded = asciiEncoding.Decode(encoded);
    REQUIRE(decoded == text);
  }

  TEST_CASE("ASCIIEncoding(Invalid)", "[Text][Encodings][ASCII]")
  {
    ASCIIEncoding asciiEncoding;

    utf8_string text = u8"Hello, 世界!";
    List<byte> encoded = asciiEncoding.Encode(text);
    utf8_string decoded = asciiEncoding.Decode(encoded);
    REQUIRE(decoded == u8"Hello, ??!");
  }
}