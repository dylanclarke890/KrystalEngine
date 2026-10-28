#include "Krystal.Booey/Graphics/ImageBufferUtilities.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/ByteArrayPixelBuffer.hpp"
#include "Krystal.Booey/Graphics/DestinationColorSpace.hpp"
#include "Krystal.Booey/Graphics/ImageFrame.hpp"
#include "Krystal.Booey/Platform/ImageDecoders/ScalableImageDecoder.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include "Krystal.Core/Text/Base64.hpp"
#include <cstdlib>

namespace krys::boo::tests
{
  namespace
  {
    /// @brief A 2 by 2 buffer: red, green, blue and half-transparent white, its alpha @p alphaFormat.
    KRYS_NODISCARD Ref<ByteArrayPixelBuffer> Pixels(AlphaPremultiplication alphaFormat) noexcept
    {
      const uint8 half = alphaFormat == AlphaPremultiplication::Premultiplied ? 128 : 255;
      const uint8 bytes[] = {255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, half, half, half, 128};
      auto buffer = ByteArrayPixelBuffer::Create(
        {alphaFormat, PixelFormat::RGBA8, DestinationColorSpace::SRGB()}, {2, 2}, Span<const uint8>(bytes));
      REQUIRE(buffer);
      return *buffer;
    }

    /// @brief The pixels of @p encoded, decoded, as unpremultiplied ARGB.
    KRYS_NODISCARD List<uint32> Decoded(const List<uint8> &encoded) noexcept
    {
      auto data = SharedBuffer::Create(Span<const uint8>(encoded));
      auto decoder = ScalableImageDecoder::Create(*data, AlphaOption::NotPremultiplied,
                                                  GammaAndColorProfileOption::Applied);
      REQUIRE(decoder);
      decoder->SetData(*data, true);
      auto image = decoder->CreateFrameImageAtIndex(0);
      REQUIRE(image);
      auto pixels = image->Pixels();
      return {pixels.begin(), pixels.end()};
    }

    /// @brief Whether each channel of @p actual is within @p tolerance of @p expected (lossy formats).
    KRYS_NODISCARD bool Near(uint32 actual, uint32 expected, int32 tolerance) noexcept
    {
      for (int32 shift = 0; shift < 32; shift += 8)
      {
        const auto a = static_cast<int32>((actual >> shift) & 0xFFu);
        const auto e = static_cast<int32>((expected >> shift) & 0xFFu);
        if (std::abs(a - e) > tolerance)
        {
          return false;
        }
      }
      return true;
    }
  }

  TEST_CASE("ImageBufferUtilities: encoding", "[Graphics][ImageBufferUtilities]")
  {
    SECTION("PNG keeps every pixel and its alpha")
    {
      auto png = EncodeData(*Pixels(AlphaPremultiplication::Unpremultiplied), u8"image/png");
      REQUIRE(png.size() > 8u);
      CHECK(png[0] == 0x89);
      CHECK(png[1] == 'P');
      auto pixels = Decoded(png);
      REQUIRE(pixels.size() == 4u);
      CHECK(pixels[0] == 0xFF'FF'00'00u);
      CHECK(pixels[1] == 0xFF'00'FF'00u);
      CHECK(pixels[2] == 0xFF'00'00'FFu);
      CHECK(pixels[3] == 0x80'FF'FF'FFu);
    }

    SECTION("JPEG has no alpha: a premultiplied buffer is drawn over black")
    {
      auto jpeg = EncodeData(*Pixels(AlphaPremultiplication::Premultiplied), u8"IMAGE/JPEG", 1.0);
      REQUIRE(jpeg.size() > 3u);
      CHECK(jpeg[0] == 0xFF);
      CHECK(jpeg[1] == 0xD8);
      auto pixels = Decoded(jpeg);
      REQUIRE(pixels.size() == 4u);
      // Chroma subsampling blurs the colours of a 2 by 2 image together, so only the luminance is close.
      CHECK((pixels[3] >> 24) == 0xFFu);
      CHECK(Near(pixels[3], 0xFF'80'80'80u, 80));

      // A lower quality is smaller.
      auto big = EncodeData(*Pixels(AlphaPremultiplication::Premultiplied), u8"image/jpeg", 1.0);
      CHECK(EncodeData(*Pixels(AlphaPremultiplication::Premultiplied), u8"image/jpg", 0.1).size()
            < big.size());
    }

    SECTION("WebP")
    {
      auto webp = EncodeData(*Pixels(AlphaPremultiplication::Unpremultiplied), u8"image/webp");
      REQUIRE(webp.size() > 12u);
      CHECK(webp[0] == 'R');
      CHECK(webp[8] == 'W');
      auto pixels = Decoded(webp);
      REQUIRE(pixels.size() == 4u);
      CHECK(Near(pixels[3] & 0xFF'00'00'00u, 0x80'00'00'00u, 4));
    }

    SECTION("other types and empty buffers encode nothing")
    {
      CHECK(EncodeData(*Pixels(AlphaPremultiplication::Unpremultiplied), u8"image/gif").empty());
      auto empty = ByteArrayPixelBuffer::TryCreate(
        {AlphaPremultiplication::Unpremultiplied, PixelFormat::RGBA8, DestinationColorSpace::SRGB()}, {0, 0});
      if (empty)
      {
        CHECK(EncodeData(*empty, u8"image/png").empty());
      }
    }

    SECTION("data URLs")
    {
      CHECK(DataURL({}, u8"image/png") == u8"data:,");
      const uint8 bytes[] = {1, 2, 3};
      CHECK(DataURL(bytes, u8"image/png") == u8"data:image/png;base64,AQID");
    }
  }
}
