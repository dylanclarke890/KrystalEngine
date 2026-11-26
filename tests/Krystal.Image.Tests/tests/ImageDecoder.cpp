#include "Krystal.Image/ImageDecoder.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Types/SharedBuffer.hpp"
#include "Krystal.Image.Tests/AnimatedImages.hpp"
#include "Krystal.Image.Tests/EXIFImages.hpp"
#include "Krystal.Image.Tests/GIFImages.hpp"
#include "Krystal.Image.Tests/ICCImages.hpp"
#include "Krystal.Image.Tests/JPEGImages.hpp"
#include "Krystal.Image.Tests/PNGImages.hpp"
#include "Krystal.Image.Tests/TestImages.hpp"
#include <algorithm>
#include <cstdlib>

namespace krys::img::tests
{
  namespace
  {
    constexpr uint32 Red = 0xFF'FF'00'00u;
    constexpr uint32 Green = 0xFF'00'FF'00u;
    constexpr uint32 Blue = 0xFF'00'00'FFu;
    constexpr uint32 White = 0xFF'FF'FF'FFu;
    constexpr uint32 Black = 0xFF'00'00'00u;

    KRYS_NODISCARD RefPtr<ImageDecoder>
      Decode(Span<const uint8> bytes, AlphaOption alpha = AlphaOption::Premultiplied,
             GammaAndColorProfileOption profile = GammaAndColorProfileOption::Applied) noexcept
    {
      auto data = SharedBuffer::Create(bytes);
      auto decoder = ImageDecoder::Create(*data, alpha, profile);
      if (decoder)
      {
        decoder->SetData(*data, true);
      }
      return decoder;
    }

    KRYS_NODISCARD List<uint32> Pixels(ImageDecoder &decoder, size_t index = 0) noexcept
    {
      auto image = decoder.CreateFrameImageAtIndex(index);
      if (!image)
      {
        return {};
      }
      return image->Pixels->Pixels;
    }

    /// @brief Decodes @p bytes as they'd arrive over a network, @p chunkSize bytes at a time, asking for
    /// every frame after each chunk. Sets @p sawPartialFrame if a frame was ever partly decoded.
    KRYS_NODISCARD RefPtr<ImageDecoder> DecodeInChunks(Span<const uint8> bytes, size_t chunkSize,
                                                       bool *sawPartialFrame = nullptr) noexcept
    {
      RefPtr<ImageDecoder> decoder;
      for (size_t size = std::min(chunkSize, bytes.size());; size = std::min(size + chunkSize, bytes.size()))
      {
        // A new buffer each time, so the data moves as it grows, as a network buffer's may.
        auto data = SharedBuffer::Create(bytes.first(size));
        auto allDataReceived = size == bytes.size();
        if (!decoder)
        {
          decoder =
            ImageDecoder::Create(*data, AlphaOption::Premultiplied, GammaAndColorProfileOption::Applied);
        }
        if (decoder)
        {
          decoder->SetData(*data, allDataReceived);
          for (size_t index = 0; index < decoder->FrameCount(); ++index)
          {
            auto *frame = decoder->FrameBufferAtIndex(index);
            if (sawPartialFrame && frame && frame->IsPartial())
            {
              *sawPartialFrame = true;
            }
          }
        }
        if (allDataReceived)
        {
          return decoder;
        }
      }
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

  TEST_CASE("ImageDecoder: sniffs the format from the data", "[ImageDecoder]")
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

  TEST_CASE("PNGImageDecoder: decodes pixels, premultiplied or not", "[ImageDecoder]")
  {
    SECTION("Premultiplied")
    {
      auto decoder = Decode(images::PNG);
      REQUIRE(decoder);
      CHECK(decoder->GetEncodedDataStatus() == EncodedDataStatus::Complete);
      CHECK(decoder->Size() == SizeI(3, 2));
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
        ImageDecoder::Create(*part, AlphaOption::Premultiplied, GammaAndColorProfileOption::Applied);
      REQUIRE(decoder);
      decoder->SetData(*part, false);
      CHECK(decoder->GetEncodedDataStatus() == EncodedDataStatus::SizeAvailable);
      CHECK(decoder->Size() == SizeI(3, 2));
      CHECK_FALSE(decoder->FrameIsCompleteAtIndex(0));

      decoder->SetData(*all, true);
      CHECK(decoder->GetEncodedDataStatus() == EncodedDataStatus::Complete);
      CHECK(Pixels(*decoder) == List<uint32> {Red, Green, Blue, White, Black, 0x80'80'00'00u});
    }

    SECTION("Truncated data fails once all of it has arrived")
    {
      auto part = SharedBuffer::Create(images::PNG.first(20));
      auto decoder =
        ImageDecoder::Create(*part, AlphaOption::Premultiplied, GammaAndColorProfileOption::Applied);
      REQUIRE(decoder);
      decoder->SetData(*part, true);
      CHECK(decoder->GetEncodedDataStatus() == EncodedDataStatus::Error);
    }
  }

  TEST_CASE("JPEGImageDecoder: decodes pixels and the EXIF orientation", "[ImageDecoder]")
  {
    auto decoder = Decode(images::JPEG);
    REQUIRE(decoder);
    CHECK(decoder->Size() == SizeI(16, 8));
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
    CHECK(rotated->Size() == SizeI(16, 8));
    CHECK(rotated->FrameOrientationAtIndex(0) == ImageOrientation::Orientation::OriginRightTop);
  }

  TEST_CASE("JPEGImageDecoder: decodes progressive and CMYK images", "[ImageDecoder]")
  {
    for (auto bytes : {images::ProgressiveJPEG, images::CMYKJPEG})
    {
      auto decoder = Decode(bytes);
      REQUIRE(decoder);
      CHECK(decoder->Size() == SizeI(16, 8));
      auto pixels = Pixels(*decoder);
      REQUIRE(pixels.size() == 16u * 8u);
      CHECK_FALSE(decoder->FrameHasAlphaAtIndex(0));
      CHECK(Near(pixels[3 * 16 + 2], Red));
      CHECK(Near(pixels[3 * 16 + 13], Blue));
    }
  }

  TEST_CASE("ImageDecoder: decodes data as it arrives", "[ImageDecoder]")
  {
    for (auto bytes :
         {images::JPEG, images::ProgressiveJPEG, images::CMYKJPEG, images::RotatedProfileJPEG, images::PNG,
          images::AnimatedPNG, images::InterlacedPNG, images::PalettePNG, images::GrayAlpha16PNG,
          images::BlendAPNG, images::InterlacedBlendAPNG, images::BrokenAPNG, images::GIF,
          images::AnimatedGIF, images::InterlacedGIF, images::NoiseGIF, images::DisposalGIF, images::WebP})
    {
      auto whole = Decode(bytes);
      REQUIRE(whole);
      INFO(reinterpret_cast<const char *>(whole->FilenameExtension().c_str())
           << ", " << bytes.size() << " bytes");
      for (size_t chunkSize : {1, 7, 64})
      {
        INFO("chunks of " << chunkSize);
        auto streamed = DecodeInChunks(bytes, chunkSize);
        REQUIRE(streamed);
        CHECK_FALSE(streamed->Failed());
        REQUIRE(streamed->FrameCount() == whole->FrameCount());
        for (size_t index = 0; index < whole->FrameCount(); ++index)
        {
          CHECK(Pixels(*streamed, index) == Pixels(*whole, index));
        }
      }
    }

    // A progressive JPEG shows its early scans before the rest arrives.
    auto sawPartialFrame = false;
    std::ignore = DecodeInChunks(images::ProgressiveJPEG, 32, &sawPartialFrame);
    CHECK(sawPartialFrame);
  }

  TEST_CASE("ImageDecoder: EXIF resolution gives the density-corrected size",
            "[Platform][ImageDecoders][html:prepare-an-image-for-presentation]")
  {
    SECTION("A JPEG at 144 dots per inch is half its size")
    {
      auto decoder = Decode(images::DensityJPEG);
      REQUIRE(decoder);
      CHECK(decoder->Size() == SizeI(16, 8));
      CHECK(decoder->FrameDensityCorrectedSizeAtIndex(0) == SizeI(8, 4));
      CHECK(Pixels(*decoder).size() == 16u * 8u);
    }

    SECTION("Without EXIF, or when its dimensions or unit don't match, there's none")
    {
      for (auto bytes : {images::JPEG, images::MismatchedDensityJPEG, images::CentimetreDensityJPEG})
      {
        auto decoder = Decode(bytes);
        REQUIRE(decoder);
        CHECK(decoder->Size() == SizeI(16, 8));
        CHECK_FALSE(decoder->FrameDensityCorrectedSizeAtIndex(0));
      }
    }

    SECTION("As in Chrome, PNG's eXIf chunk and WebP's EXIF chunk don't give it")
    {
      for (auto bytes : {images::DensityPNG, images::DensityWebP})
      {
        auto decoder = Decode(bytes, AlphaOption::NotPremultiplied);
        REQUIRE(decoder);
        CHECK(decoder->Size() == SizeI(3, 2));
        CHECK_FALSE(decoder->FrameDensityCorrectedSizeAtIndex(0));
        CHECK(Pixels(*decoder) == List<uint32> {Red, Green, Blue, White, Black, 0x80'FF'00'00u});
      }
    }
  }

  TEST_CASE("WEBPImageDecoder: decodes lossless pixels", "[ImageDecoder]")
  {
    auto decoder = Decode(images::WebP, AlphaOption::NotPremultiplied);
    REQUIRE(decoder);
    CHECK(decoder->Size() == SizeI(3, 2));
    CHECK(decoder->FrameCount() == 1u);
    CHECK(Pixels(*decoder) == List<uint32> {Red, Green, Blue, White, Black, 0x80'FF'00'00u});
  }

  TEST_CASE("GIFImageDecoder: decodes palette pixels and transparency", "[ImageDecoder]")
  {
    auto decoder = Decode(images::GIF);
    REQUIRE(decoder);
    CHECK(decoder->Size() == SizeI(3, 2));
    CHECK(decoder->FrameCount() == 1u);
    // Still GIFs loop once, as animated ones without a looping extension do.
    CHECK(decoder->GetRepetitionCount() == RepetitionCountOnce);
    CHECK(Pixels(*decoder) == List<uint32> {Red, Green, Blue, White, Black, 0u});
  }

  TEST_CASE("PNGImageDecoder: decodes interlaced, palette and 16-bit images", "[ImageDecoder]")
  {
    auto interlaced = Decode(images::InterlacedPNG);
    REQUIRE(interlaced);
    auto gradient = Pixels(*interlaced);
    REQUIRE(gradient.size() == 9u * 9u);
    auto mismatches = 0;
    for (uint32 y = 0; y < 9; ++y)
    {
      for (uint32 x = 0; x < 9; ++x)
      {
        mismatches += gradient[y * 9 + x] != (0xFF'00'00'80u | (x * 28) << 16 | (y * 28) << 8);
      }
    }
    CHECK(mismatches == 0);
    CHECK_FALSE(interlaced->FrameHasAlphaAtIndex(0));
    CHECK(interlaced->GetRepetitionCount() == RepetitionCountNone);

    auto palette = Decode(images::PalettePNG);
    REQUIRE(palette);
    CHECK(Pixels(*palette) == List<uint32> {Red, Green, 0});
    CHECK(palette->FrameHasAlphaAtIndex(0));

    auto grayAlpha = Decode(images::GrayAlpha16PNG);
    REQUIRE(grayAlpha);
    CHECK(Pixels(*grayAlpha) == List<uint32> {White, 0});
  }

  TEST_CASE("PNGImageDecoder: blends and disposes of APNG frames as they ask", "[ImageDecoder]")
  {
    constexpr uint32 T = 0;
    constexpr uint32 R = Red;
    constexpr uint32 G = Green;
    constexpr uint32 B = Blue;
    const List<uint32> expected[] = {
      // Red everywhere; the white default image isn't part of the animation.
      {R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R},
      // Green blended over the top left, its transparent pixels showing the red; then cleared.
      {G, R, R, R, G, R, R, R, R, R, R, R, R, R, R, R},
      // Blue over the bottom right, on the cleared top left; then restored to the previous frame.
      {T, T, R, R, T, T, R, R, R, R, B, B, R, R, B, B},
      // A transparent pixel replacing (3, 0), on frame 1 as it was disposed of.
      {T, T, R, T, T, T, R, R, R, R, R, R, R, R, R, R},
    };

    // Blending an interlaced frame waits for its last pass.
    for (auto bytes : {images::BlendAPNG, images::InterlacedBlendAPNG})
    {
      auto decoder = Decode(bytes);
      REQUIRE(decoder);
      REQUIRE(decoder->FrameCount() == 4u);
      CHECK(decoder->GetRepetitionCount() == 2);
      for (size_t index = 0; index < 4; ++index)
      {
        INFO("frame " << index);
        CHECK(Pixels(*decoder, index) == expected[index]);
        CHECK(decoder->FrameDurationAtIndex(index).count() == 0.1);
      }
      CHECK_FALSE(decoder->FrameHasAlphaAtIndex(0));
      CHECK(decoder->FrameHasAlphaAtIndex(3));

      // Cleared frames are decoded again from the frames they build on.
      decoder->ClearFrameBufferCache(3);
      for (size_t index : {2u, 1u, 3u})
      {
        INFO("frame " << index << " again");
        CHECK(Pixels(*decoder, index) == expected[index]);
      }
    }

    // Data out of sequence ends the animation, keeping the frames before it.
    auto broken = Decode(images::BrokenAPNG);
    REQUIRE(broken);
    CHECK_FALSE(broken->Failed());
    REQUIRE(broken->FrameCount() == 2u);
    CHECK(Pixels(*broken, 1) == expected[1]);
  }

  TEST_CASE("GIFImageDecoder: decodes interlaced rows and large dictionaries", "[ImageDecoder]")
  {
    auto opaque = [](uint32 r, uint32 g, uint32 b)
    {
      return 0xFF'00'00'00u | (r << 16) | (g << 8) | b;
    };

    auto interlaced = Decode(images::InterlacedGIF);
    REQUIRE(interlaced);
    auto rows = Pixels(*interlaced);
    REQUIRE(rows.size() == 5u * 9u);
    for (uint32 y = 0; y < 9; ++y)
    {
      INFO("row " << y);
      CHECK(rows[y * 5] == opaque(y * 28, 255 - y * 28, y * 10));
      CHECK(rows[y * 5 + 4] == opaque(y * 28, 255 - y * 28, y * 10));
    }

    // Enough codes that the LZW dictionary fills and is cleared.
    auto noise = Decode(images::NoiseGIF);
    REQUIRE(noise);
    auto pixels = Pixels(*noise);
    REQUIRE(pixels.size() == 64u * 64u);
    auto mismatches = 0;
    for (size_t index = 0; index < pixels.size(); ++index)
    {
      const auto *color = &images::NoiseGIFPalette[size_t {images::NoiseGIFIndices[index]} * 3];
      mismatches += pixels[index] != opaque(color[0], color[1], color[2]);
    }
    CHECK(mismatches == 0);
  }

  TEST_CASE("GIFImageDecoder: disposes of frames as they ask", "[ImageDecoder]")
  {
    constexpr uint32 T = 0;
    constexpr uint32 R = Red;
    constexpr uint32 G = Green;
    constexpr uint32 B = Blue;
    constexpr uint32 W = White;
    const List<uint32> expected[] = {
      // Red everywhere.
      {R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R},
      // Green over the top left, which is then restored to the background.
      {G, G, R, R, G, G, R, R, R, R, R, R, R, R, R, R},
      // Blue over the bottom right, on the cleared top left; then restored to the previous frame.
      {T, T, R, R, T, T, R, R, R, R, B, B, R, R, B, B},
      // White at (3, 0), on frame 1 as it was disposed of: the blue is gone.
      {T, T, R, W, T, T, R, R, R, R, R, R, R, R, R, R},
    };

    auto decoder = Decode(images::DisposalGIF);
    REQUIRE(decoder);
    REQUIRE(decoder->FrameCount() == 4u);
    CHECK(decoder->GetRepetitionCount() == RepetitionCountInfinite);
    for (size_t index = 0; index < 4; ++index)
    {
      INFO("frame " << index);
      CHECK(Pixels(*decoder, index) == expected[index]);
    }
    CHECK_FALSE(decoder->FrameHasAlphaAtIndex(0));
    CHECK(decoder->FrameHasAlphaAtIndex(2));

    // Cleared frames are decoded again from the frames they build on.
    decoder->ClearFrameBufferCache(3);
    for (size_t index : {2u, 1u, 3u})
    {
      INFO("frame " << index << " again");
      CHECK(Pixels(*decoder, index) == expected[index]);
    }
  }

  TEST_CASE("BMPImageDecoder: decodes 24-bit pixels", "[ImageDecoder]")
  {
    auto decoder = Decode(images::BMP);
    REQUIRE(decoder);
    CHECK(decoder->Size() == SizeI(3, 2));
    CHECK(Pixels(*decoder) == List<uint32> {Red, Green, Blue, White, Black, Red});
  }

  TEST_CASE("ICOImageDecoder: decodes PNG and BMP entries, largest first", "[ImageDecoder]")
  {
    SECTION("PNG entries")
    {
      auto decoder = Decode(images::PNGIcon);
      REQUIRE(decoder);
      CHECK(decoder->Size() == SizeI(32, 32));
      CHECK(decoder->FrameCount() == 2u);
      CHECK(decoder->FrameSizeAtIndex(1) == SizeI(16, 16));
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
      CHECK(decoder->Size() == SizeI(16, 16));
      auto pixels = Pixels(*decoder);
      REQUIRE(pixels.size() == 16u * 16u);
      CHECK(pixels[0] == Blue);
      CHECK(pixels[255] == Blue);
    }
  }

  TEST_CASE("ImageDecoder: animated GIF, APNG and WebP frames", "[ImageDecoder]")
  {
    for (auto bytes : {images::AnimatedGIF, images::AnimatedPNG, images::AnimatedWebP})
    {
      auto decoder = Decode(bytes);
      REQUIRE(decoder);
      INFO(reinterpret_cast<const char *>(decoder->FilenameExtension().c_str()));
      REQUIRE(decoder->FrameCount() == 3u);
      CHECK(decoder->GetRepetitionCount() == RepetitionCountInfinite);
      CHECK(Pixels(*decoder, 0) == List<uint32>(4, Red));
      CHECK(Pixels(*decoder, 1) == List<uint32>(4, Green));
      CHECK(Pixels(*decoder, 2) == List<uint32>(4, Blue));
      // A frame's duration is known once it's decoded.
      for (size_t index = 0; index < 3; ++index)
      {
        CHECK(decoder->FrameDurationAtIndex(index).count() == 0.1);
      }
    }

    auto once = Decode(images::OnceGIF);
    REQUIRE(once);
    CHECK(once->FrameCount() == 2u);
    CHECK(once->GetRepetitionCount() == RepetitionCountOnce);
  }

  // The rotated profile's red is sRGB's green, its green sRGB's blue and its blue sRGB's red.
  TEST_CASE("ImageDecoder: pixels are converted from an embedded ICC profile to sRGB",
            "[Platform][ImageDecoders][ColorManagement]")
  {
    SECTION("A PNG's iCCP profile, before alpha is premultiplied")
    {
      auto decoder = Decode(images::RotatedProfilePNG, AlphaOption::NotPremultiplied);
      REQUIRE(decoder);
      CHECK(Pixels(*decoder) == List<uint32> {Green, Blue, Red, White, Black, 0x80'00'FF'00u});

      auto premultiplied = Decode(images::RotatedProfilePNG);
      REQUIRE(premultiplied);
      CHECK(Pixels(*premultiplied) == List<uint32> {Green, Blue, Red, White, Black, 0x80'00'80'00u});
    }

    SECTION("A PNG arriving in parts")
    {
      auto all = SharedBuffer::Create(images::RotatedProfilePNG);
      auto decoder =
        ImageDecoder::Create(*all, AlphaOption::NotPremultiplied, GammaAndColorProfileOption::Applied);
      REQUIRE(decoder);
      auto part =
        SharedBuffer::Create(images::RotatedProfilePNG.first(images::RotatedProfilePNG.size() - 16));
      decoder->SetData(*part, false);
      decoder->SetData(*all, true);
      CHECK(Pixels(*decoder) == List<uint32> {Green, Blue, Red, White, Black, 0x80'00'FF'00u});
    }

    SECTION("sRGB's own profile changes nothing")
    {
      auto decoder = Decode(images::SRGBProfilePNG, AlphaOption::NotPremultiplied);
      REQUIRE(decoder);
      CHECK(Pixels(*decoder) == List<uint32> {Red, Green, Blue, White, Black, 0x80'FF'00'00u});
    }

    SECTION("A PNG's profile takes the place of its gAMA")
    {
      auto decoder = Decode(images::SRGBProfileLinearGammaPNG);
      REQUIRE(decoder);
      CHECK(Pixels(*decoder) == List<uint32> {0xFF'80'80'80u});
      // Without the profile, the gamma applies.
      auto withoutProfile = Decode(images::LinearGammaPNG);
      REQUIRE(withoutProfile);
      CHECK(Near(Pixels(*withoutProfile).at(0), 0xFF'BA'BA'BAu, 2));
    }

    SECTION("A WebP's ICCP chunk")
    {
      auto decoder = Decode(images::RotatedProfileWebP, AlphaOption::NotPremultiplied);
      REQUIRE(decoder);
      CHECK(decoder->Size() == SizeI(3, 2));
      CHECK(Pixels(*decoder) == List<uint32> {Green, Blue, Red, White, Black, 0x80'00'FF'00u});
    }

    SECTION("A JPEG's ICC APP2 blocks, whole or split")
    {
      for (auto bytes : {images::RotatedProfileJPEG, images::SplitRotatedProfileJPEG})
      {
        auto decoder = Decode(bytes);
        REQUIRE(decoder);
        auto pixels = Pixels(*decoder);
        REQUIRE(pixels.size() == 16u * 8u);
        CHECK(Near(pixels[3 * 16 + 2], Green));
        CHECK(Near(pixels[3 * 16 + 13], Red));
      }
    }

    SECTION("Not when the decoder ignores colour profiles")
    {
      auto png =
        Decode(images::RotatedProfilePNG, AlphaOption::NotPremultiplied, GammaAndColorProfileOption::Ignored);
      REQUIRE(png);
      CHECK(Pixels(*png) == List<uint32> {Red, Green, Blue, White, Black, 0x80'FF'00'00u});

      auto jpeg =
        Decode(images::RotatedProfileJPEG, AlphaOption::Premultiplied, GammaAndColorProfileOption::Ignored);
      REQUIRE(jpeg);
      auto pixels = Pixels(*jpeg);
      REQUIRE(pixels.size() == 16u * 8u);
      CHECK(Near(pixels[3 * 16 + 2], Red));

      auto webp = Decode(images::RotatedProfileWebP, AlphaOption::NotPremultiplied,
                         GammaAndColorProfileOption::Ignored);
      REQUIRE(webp);
      CHECK(Pixels(*webp) == List<uint32> {Red, Green, Blue, White, Black, 0x80'FF'00'00u});
    }
  }

  TEST_CASE("ImageDecoder::SupportsMimeType: the decoders' types, ignoring case", "[ImageDecoder]")
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
