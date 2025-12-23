#include "Krystal.Text/Encodings/ASCIIEncoding.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Text;

  TEST_CASE("ASCIIEncoding - Valid ASCII", "[Text][Encodings][ASCII]")
  {
    ASCIIEncoding asciiEncoding;

    utf8_string text = u8"Hello, World! 123";
    List<byte> encoded = asciiEncoding.Encode(text);
    utf8_string decoded = asciiEncoding.Decode(encoded);
    REQUIRE(decoded == text);
  }

  TEST_CASE("ASCIIEncoding - Invalid ASCII", "[Text][Encodings][ASCII]")
  {
    ASCIIEncoding asciiEncoding;

    utf8_string text = u8"Hello, 世界!";
    List<byte> encodedWithFallback = asciiEncoding.Encode(text);
    utf8_string decodedWithFallback = asciiEncoding.Decode(encodedWithFallback);
    REQUIRE(decodedWithFallback == u8"Hello, ??!");
  }
}