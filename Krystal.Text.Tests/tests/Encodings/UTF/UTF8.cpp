#include "Krystal.Text/Encodings/UTF/UTF8.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Text;

  TEST_CASE("UTF8Encoding(Valid)", "[Text][Encodings][ASCII]")
  {
    UTF8Encoding encoding;

    utf8_string text = u8"Hello, World! 世界";
    List<byte> encoded = encoding.Encode(text);
    utf8_string decoded = encoding.Decode(encoded);
    REQUIRE(decoded == text);
  }

  TEST_CASE("UTF8Encoding(GetBOM)", "[Text][Encodings][ASCII]")
  {
    UTF8Encoding encoding;
    auto bom = encoding.GetBOM();

    REQUIRE(bom.size() == 3u);
    REQUIRE(bom[0] == byte {0xEF});
    REQUIRE(bom[1] == byte {0xBB});
    REQUIRE(bom[2] == byte {0xBF});
  }

  TEST_CASE("UTF8Encoding(IsSingleByte)", "[Text][Encodings][ASCII]")
  {
    UTF8Encoding encoding;
    REQUIRE(encoding.IsSingleByte());
  }
}