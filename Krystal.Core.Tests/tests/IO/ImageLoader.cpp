#include "Krystal.Core/IO/ImageLoader.hpp"
#include "Krystal.Core/IO/Path.hpp"
#include "Krystal.Core/IO/Streams/NativeFileStream.hpp"
#include <catch_all.hpp>

namespace krys::io::tests
{
  TEST_CASE("LoadImage", "[IO][Images]")
  {
    auto stream = NativeFileReader(Path("data/io/24bpp-320x240.bmp"));
    REQUIRE(stream.IsOpen());

    ImageLoader loader;
    auto image = loader.Load(stream);
    REQUIRE(image.has_value());
    REQUIRE(image->Width == 320);
    REQUIRE(image->Height == 240);
    REQUIRE(image->Channels == 3);
  }
}
