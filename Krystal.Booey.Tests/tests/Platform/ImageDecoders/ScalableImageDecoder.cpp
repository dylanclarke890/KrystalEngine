#include "Krystal.Booey/Platform/ImageDecoders/ScalableImageDecoder.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Platform/ImageDecoders/TestImages.hpp"
#include "Krystal.Booey/Graphics/ImageFrame.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include <cstdlib>

namespace krys::boo::tests
{
  namespace
  {
    constexpr uint32 Red = 0xFF'FF'00'00u;
    constexpr uint32 Green = 0xFF'00'FF'00u;
    constexpr uint32 Blue = 0xFF'00'00'FFu;
    constexpr uint32 White = 0xFF'FF'FF'FFu;
    constexpr uint32 Black = 0xFF'00'00'00u;

    KRYS_NODISCARD RefPtr<ScalableImageDecoder>
      Decode(Span<const uint8> bytes, AlphaOption alpha = AlphaOption::Premultiplied) noexcept
    {
      auto data = SharedBuffer::Create(bytes);
      auto decoder = ScalableImageDecoder::Create(*data, alpha, GammaAndColorProfileOption::Applied);
      if (decoder)
      {
        decoder->SetData(*data, true);
      }
      return decoder;
    }

    KRYS_NODISCARD List<uint32> Pixels(ScalableImageDecoder &decoder, size_t index = 0) noexcept
    {
      auto image = decoder.CreateFrameImageAtIndex(index);
      if (!image)
      {
        return {};
      }
      auto pixels = image->Pixels();
      return {pixels.begin(), pixels.end()};
    }

    /// @brief Whether each channel of @p actual is within @p tolerance of @p expected (lossy formats).
    KRYS_NODISCARD bool Near(uint32 actual, uint32 expected, int tolerance = 12) noexcept
    {
      for (int shift = 0; shift < 32; shift += 8)
      {
        const int a = static_cast<int>((actual >> shift) & 0xFFu);
        const int e = static_cast<int>((expected >> shift) & 0xFFu);
        if (std::abs(a - e) > tolerance)
        {
          return false;
        }
      }
      return true;
    }
  }

  TEST_CASE("ScalableImageDecoder: sniffs the format from the data", "[Platform][ImageDecoders]")
  {
    CHECK(Decode(images::PNG)->FilenameExtension() == u8"png");
    CHECK(Decode(images::JPEG)->FilenameExtension() == u8"jpg");
    CHECK(Decode(images::WebP)->FilenameExtension() == u8"webp");
    CHECK(Decode(images::GIF)->FilenameExtension() == u8"gif");
    CHECK(Decode(images::BMP)->FilenameExtension() == u8"bmp");
    CHECK(Decode(images::PNGIcon)->FilenameExtension() == u8"ico");

    const uint8 text[] = "not an image at all";
    CHECK_FALSE(Decode(text));
  }

  TEST_CASE("PNGImageDecoder: decodes pixels, premultiplied or not", "[Platform][ImageDecoders]")
  {
    SECTION("Premultiplied")
    {
      auto decoder = Decode(images::PNG);
      REQUIRE(decoder);
      CHECK(decoder->GetEncodedDataStatus() == EncodedDataStatus::Complete);
      CHECK(decoder->Size() == IntSize(3, 2));
      CHECK(decoder->FrameCount() == 1u);
      CHECK(decoder->FrameHasAlphaAtIndex(0));
      CHECK(Pixels(*decoder) == List<uint32> {Red, Green, Blue, White, Black, 0x80'80'00'00u});
    }

    SECTION("Not premultiplied")
    {
      auto decoder = Decode(images::PNG, AlphaOption::NotPremultiplied);
      REQUIRE(decoder);
      CHECK(Pixels(*decoder) == List<uint32> {Red, Green, Blue, White, Black, 0x80'FF'00'00u});
    }

    SECTION("Data arriving in parts")
    {
      auto all = SharedBuffer::Create(images::PNG);
      // Up to the image data, cut short of its end.
      auto part = SharedBuffer::Create(images::PNG.first(images::PNG.size() - 16));
      auto decoder =
        ScalableImageDecoder::Create(*part, AlphaOption::Premultiplied, GammaAndColorProfileOption::Applied);
      REQUIRE(decoder);
      decoder->SetData(*part, false);
      CHECK(decoder->GetEncodedDataStatus() == EncodedDataStatus::SizeAvailable);
      CHECK(decoder->Size() == IntSize(3, 2));
      CHECK_FALSE(decoder->FrameIsCompleteAtIndex(0));

      decoder->SetData(*all, true);
      CHECK(decoder->GetEncodedDataStatus() == EncodedDataStatus::Complete);
      CHECK(Pixels(*decoder) == List<uint32> {Red, Green, Blue, White, Black, 0x80'80'00'00u});
    }

    SECTION("Truncated data fails once all of it has arrived")
    {
      auto part = SharedBuffer::Create(images::PNG.first(20));
      auto decoder =
        ScalableImageDecoder::Create(*part, AlphaOption::Premultiplied, GammaAndColorProfileOption::Applied);
      REQUIRE(decoder);
      decoder->SetData(*part, true);
      CHECK(decoder->GetEncodedDataStatus() == EncodedDataStatus::Error);
    }
  }

  TEST_CASE("JPEGImageDecoder: decodes pixels and the EXIF orientation", "[Platform][ImageDecoders]")
  {
    auto decoder = Decode(images::JPEG);
    REQUIRE(decoder);
    CHECK(decoder->Size() == IntSize(16, 8));
    CHECK(decoder->FrameOrientationAtIndex(0) == ImageOrientation::Orientation::None);
    // A frame has alpha until it's decoded.
    CHECK(decoder->FrameHasAlphaAtIndex(0));
    auto pixels = Pixels(*decoder);
    REQUIRE(pixels.size() == 16u * 8u);
    CHECK_FALSE(decoder->FrameHasAlphaAtIndex(0));
    CHECK(Near(pixels[3 * 16 + 2], Red));
    CHECK(Near(pixels[3 * 16 + 13], Blue));

    auto rotated = Decode(images::RotatedJPEG);
    REQUIRE(rotated);
    // The decoder reports the encoded size; the image applies the orientation.
    CHECK(rotated->Size() == IntSize(16, 8));
    CHECK(rotated->FrameOrientationAtIndex(0) == ImageOrientation::Orientation::OriginRightTop);
  }

  TEST_CASE("WEBPImageDecoder: decodes lossless pixels", "[Platform][ImageDecoders]")
  {
    auto decoder = Decode(images::WebP, AlphaOption::NotPremultiplied);
    REQUIRE(decoder);
    CHECK(decoder->Size() == IntSize(3, 2));
    CHECK(decoder->FrameCount() == 1u);
    CHECK(Pixels(*decoder) == List<uint32> {Red, Green, Blue, White, Black, 0x80'FF'00'00u});
  }

  TEST_CASE("GIFImageDecoder: decodes palette pixels and transparency", "[Platform][ImageDecoders]")
  {
    auto decoder = Decode(images::GIF);
    REQUIRE(decoder);
    CHECK(decoder->Size() == IntSize(3, 2));
    CHECK(decoder->FrameCount() == 1u);
    // Still GIFs loop once, as animated ones without a looping extension do.
    CHECK(decoder->GetRepetitionCount() == RepetitionCountOnce);
    CHECK(Pixels(*decoder) == List<uint32> {Red, Green, Blue, White, Black, 0u});
  }

  TEST_CASE("BMPImageDecoder: decodes 24-bit pixels", "[Platform][ImageDecoders]")
  {
    auto decoder = Decode(images::BMP);
    REQUIRE(decoder);
    CHECK(decoder->Size() == IntSize(3, 2));
    CHECK(Pixels(*decoder) == List<uint32> {Red, Green, Blue, White, Black, Red});
  }

  TEST_CASE("ICOImageDecoder: decodes PNG and BMP entries, largest first", "[Platform][ImageDecoders]")
  {
    SECTION("PNG entries")
    {
      auto decoder = Decode(images::PNGIcon);
      REQUIRE(decoder);
      CHECK(decoder->Size() == IntSize(32, 32));
      CHECK(decoder->FrameCount() == 2u);
      CHECK(decoder->FrameSizeAtIndex(1, SubsamplingLevel::Default) == IntSize(16, 16));
      CHECK_FALSE(decoder->HotSpot());
      auto pixels = Pixels(*decoder);
      REQUIRE(pixels.size() == 32u * 32u);
      CHECK(pixels[0] == Green);
      CHECK(Pixels(*decoder, 1).size() == 16u * 16u);
    }

    SECTION("A BMP entry")
    {
      auto decoder = Decode(images::BMPIcon);
      REQUIRE(decoder);
      CHECK(decoder->Size() == IntSize(16, 16));
      auto pixels = Pixels(*decoder);
      REQUIRE(pixels.size() == 16u * 16u);
      CHECK(pixels[0] == Blue);
      CHECK(pixels[255] == Blue);
    }
  }
}
