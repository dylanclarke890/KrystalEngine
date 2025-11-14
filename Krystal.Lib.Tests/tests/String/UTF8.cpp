#include "Krystal.Lib/String/UTF8.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  TEST_CASE("UTF8 Encoding and Decoding", "[Krystal.Lib][String][UTF8]")
  {
    using namespace Krys;

    auto codepoints = UTF8::Decode(u8"hé😊世界");
    auto encodedString = UTF8::Encode(codepoints);
    REQUIRE(encodedString == u8"hé😊世界");
  }
}