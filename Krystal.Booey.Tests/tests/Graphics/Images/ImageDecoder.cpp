#include "Krystal.Booey/Graphics/Images/ImageDecoder.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::tests
{
  TEST_CASE("ImageDecoder::SupportsMimeType: the decoders' types, ignoring case", "[Graphics][ImageDecoder]")
  {
    CHECK(ImageDecoder::SupportsMimeType(u8"image/png"));
    CHECK(ImageDecoder::SupportsMimeType(u8"IMAGE/JPEG"));
    CHECK(ImageDecoder::SupportsMimeType(u8"image/gif"));
    CHECK(ImageDecoder::SupportsMimeType(u8"image/webp"));
    CHECK(ImageDecoder::SupportsMimeType(u8"image/bmp"));
    CHECK(ImageDecoder::SupportsMimeType(u8"image/vnd.microsoft.icon"));
    CHECK_FALSE(ImageDecoder::SupportsMimeType(u8"image/svg+xml"));
    CHECK_FALSE(ImageDecoder::SupportsMimeType(u8"image/avif"));
    CHECK_FALSE(ImageDecoder::SupportsMimeType(u8"text/html"));
  }
}
