#include "Krystal.Text/Encodings/UTF/UTF8.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys;

  TEST_CASE("UTF8Encoding(Valid)", "[Text][Encodings][ASCII]")
  {
    UTF8Encoding encoding;

    utf8_string text = u8"Hello, World! 世界";
    List<byte> encoded = encoding.Encode(text);
    utf8_string decoded = encoding.Decode(encoded);
    REQUIRE(decoded == text);
  }
}