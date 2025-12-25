#include "Krystal.Text/Encodings/UTF8Encoding.hpp"
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

  TEST_CASE("UTF8Encoding(Preamble)", "[Text][Encodings][ASCII]")
  {
    UTF8Encoding encoding;
    REQUIRE(encoding.GetPreamble() == List<byte>{byte {0xEF}, byte {0xBB}, byte {0xBF}});
  }

  TEST_CASE("UTF8Encoding(IsSingleByte)", "[Text][Encodings][ASCII]")
  {
    UTF8Encoding encoding;
    REQUIRE(encoding.IsSingleByte());
  }
}