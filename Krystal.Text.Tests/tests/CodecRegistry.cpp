#include "Krystal.Text/Codecs/CodecRegistry.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Text/Encodings/UTF8.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Text;

  TEST_CASE("CodecRegistry(Register)", "[Text][CodecRegistry]")
  {
    CodecRegistry registry;

    registry.Register<BasicCodec<basic_utf8<byte>>>();

    // Retrieve the encoding by its name

    ICodec *codec = registry.Find("utf-8"_s);

    REQUIRE(codec != nullptr);
    REQUIRE(string(codec->Name().Characters()) == string("UTF-8"));
  }
}