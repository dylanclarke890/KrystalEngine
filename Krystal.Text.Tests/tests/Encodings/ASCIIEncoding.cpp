#include "Krystal.Text/Encodings/ASCIIEncoding.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Text;

  TEST_CASE("ASCIIEncoding(Valid)", "[Text][Encodings][ASCII]")
  {
    ASCIIEncoding encoding;

    utf8_string text = u8"Hello, World! 123";
    List<byte> encoded = encoding.Encode(text);
    utf8_string decoded = encoding.Decode(encoded);
    REQUIRE(decoded == text);
  }

  TEST_CASE("ASCIIEncoding(Invalid)", "[Text][Encodings][ASCII]")
  {
    ASCIIEncoding encoding;

    utf8_string text = u8"Hello, 世界!";
    List<byte> encoded = encoding.Encode(text);
    utf8_string decoded = encoding.Decode(encoded);
    REQUIRE(decoded == u8"Hello, ??!");
  }

  TEST_CASE("ASCIIEncoding(Preamble)", "[Text][Encodings][ASCII]")
  {
    ASCIIEncoding encoding;
    REQUIRE(encoding.GetPreamble().empty());
  }

  TEST_CASE("ASCIIEncoding(IsSingleByte)", "[Text][Encodings][ASCII]")
  {
    ASCIIEncoding encoding;
    REQUIRE(encoding.IsSingleByte());
  }
}