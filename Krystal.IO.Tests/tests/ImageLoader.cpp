#include "Krystal.IO/ImageLoader.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.IO/Streams/NativeFileStream.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::IO;

  TEST_CASE("LoadImage", "[IO][Images]")
  {
    auto stream = NativeFileReader(Path("data/24bpp-320x240.bmp"));
    REQUIRE(stream.IsOpen());

    ImageLoader loader;
    auto image = loader.Load(stream);
    REQUIRE(image.has_value());
    REQUIRE(image->Width == 320);
    REQUIRE(image->Height == 240);
    REQUIRE(image->Channels == 3);
  }
}
