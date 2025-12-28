#include "Krystal.Text/Encodings/ASCII.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys;

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

  TEST_CASE("ASCIIEncoding(GetBOM)", "[Text][Encodings][ASCII]")
  {
    ASCIIEncoding encoding;
    REQUIRE(encoding.GetBOM().empty());
  }

  TEST_CASE("ASCIIEncoding(IsSingleByte)", "[Text][Encodings][ASCII]")
  {
    ASCIIEncoding encoding;
    REQUIRE(encoding.IsSingleByte());
  }
}