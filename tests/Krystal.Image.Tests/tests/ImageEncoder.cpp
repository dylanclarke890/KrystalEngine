#include "Krystal.Image/ImageEncoder.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Types/SharedBuffer.hpp"
#include "Krystal.Image/ImageDecoder.hpp"

namespace krys::img::tests
{
  namespace
  {
    // Red, then green at half alpha.
    constexpr uint8 Pixels[] = {0xFF, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x80};

    /// @brief @p encoded's size and first frame's ARGB pixels, unpremultiplied.
    KRYS_NODISCARD Maybe<DecodedImage> Decode(const List<uint8> &encoded) noexcept
    {
      auto data = SharedBuffer::Create(Span<const uint8>(encoded));
      auto decoder =
        ImageDecoder::Create(*data, AlphaOption::NotPremultiplied, GammaAndColorProfileOption::Applied);
      if (!decoder)
      {
        return null;
      }
      decoder->SetData(*data, true);
      return decoder->CreateFrameImageAtIndex(0);
    }
  }

  TEST_CASE("EncodeImage: PNG keeps the pixels and their alpha", "[ImageEncoder]")
  {
    auto encoded = EncodeImage(Pixels, {2, 1}, EncodedImageFormat::PNG);
    auto image = Decode(encoded);
    REQUIRE(image);
    CHECK(image->Size == SizeI(2, 1));
    CHECK(image->Pixels->Pixels == List<uint32> {0xFF'FF'00'00u, 0x80'00'FF'00u});
  }

  TEST_CASE("EncodeImage: JPEG and WebP decode at the same size", "[ImageEncoder]")
  {
    for (auto format : {EncodedImageFormat::JPEG, EncodedImageFormat::WebP})
    {
      auto encoded = EncodeImage(Pixels, {2, 1}, format, 0.5);
      auto image = Decode(encoded);
      REQUIRE(image);
      CHECK(image->Size == SizeI(2, 1));
    }
  }

  TEST_CASE("EncodeImage: nothing for an empty size or too few pixels", "[ImageEncoder]")
  {
    CHECK(EncodeImage(Pixels, {0, 1}, EncodedImageFormat::PNG).empty());
    CHECK(EncodeImage(Pixels, {2, 2}, EncodedImageFormat::PNG).empty());
  }
}
