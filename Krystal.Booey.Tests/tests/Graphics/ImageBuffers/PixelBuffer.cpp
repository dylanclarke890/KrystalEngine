#include "Krystal.Booey/Graphics/ImageBuffers/PixelBuffer.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Types/Uint8ClampedArray.hpp"
#include "Krystal.Booey/Graphics/ImageBuffers/ByteArrayPixelBuffer.hpp"
#include "Krystal.Booey/Graphics/ImageBuffers/PixelBufferConversion.hpp"
#include <cmath>
#include <limits>

namespace krys::boo::tests
{
  namespace
  {
    using Pixel = Array<uint8, 4>;

    KRYS_NODISCARD PixelBufferFormat Format(AlphaPremultiplication alpha, PixelFormat format) noexcept
    {
      return {alpha, format, DestinationColorSpace::SRGB()};
    }

    /// @brief @p pixel converted from @p from to @p to.
    KRYS_NODISCARD Pixel Convert(const Pixel &pixel, const PixelBufferFormat &from,
                                 const PixelBufferFormat &to) noexcept
    {
      Pixel converted {};
      ConvertImagePixels({from, 4u, {pixel.data(), 4u}}, {to, 4u, {converted.data(), 4u}}, IntSize(1, 1));
      return converted;
    }
  }

  TEST_CASE("Uint8ClampedArray: writes are clamped and rounded to even", "[Graphics][PixelBuffer]")
  {
    using dom::Uint8ClampedArray;
    CHECK(Uint8ClampedArray::ToNativeFromDouble(-1) == 0);
    CHECK(Uint8ClampedArray::ToNativeFromDouble(300) == 255);
    CHECK(Uint8ClampedArray::ToNativeFromDouble(std::numeric_limits<double>::quiet_NaN()) == 0);
    CHECK(Uint8ClampedArray::ToNativeFromDouble(0.5) == 0);
    CHECK(Uint8ClampedArray::ToNativeFromDouble(1.5) == 2);
    CHECK(Uint8ClampedArray::ToNativeFromDouble(2.5) == 2);
    CHECK(Uint8ClampedArray::ToNativeFromDouble(2.6) == 3);
    CHECK(Uint8ClampedArray::ToNativeFromDouble(254.5) == 254);

    auto array = Uint8ClampedArray::Create(4u);
    CHECK(array->Length() == 4u);
    array->Set(1, 99.7);
    CHECK(array->Item(0) == 0);
    CHECK(array->Item(1) == 100);
  }

  TEST_CASE("PixelBuffer: sizes, ranges and the byte array buffer", "[Graphics][PixelBuffer]")
  {
    CHECK(PixelBuffer::BytesPerPixel(PixelFormat::RGBA8) == 4u);
    CHECK(PixelBuffer::ComputeBufferSize(PixelFormat::RGBA8, IntSize(3, 2)) == 24u);
    CHECK(PixelBuffer::ComputePixelCount(IntSize(3, 2)) == 6u);
    // Sizes that don't fit in an int32 overflow.
    CHECK_FALSE(PixelBuffer::ComputeBufferSize(PixelFormat::RGBA8, IntSize(40'000, 40'000)));
    CHECK(PixelBuffer::SupportedPixelFormat(PixelFormat::BGRA8));
    CHECK_FALSE(PixelBuffer::SupportedPixelFormat(PixelFormat::BGRX8));

    const auto format = Format(AlphaPremultiplication::Unpremultiplied, PixelFormat::RGBA8);
    auto buffer = ByteArrayPixelBuffer::TryCreate(format, IntSize(2, 1));
    REQUIRE(buffer);
    CHECK(buffer->GetType() == PixelBuffer::Type::ByteArray);
    CHECK(buffer->Bytes().size() == 8u);
    CHECK(buffer->Item(0) == 0);

    const uint8 bytes[] {1, 2, 3, 4};
    CHECK(buffer->SetRange(bytes, 4u));
    CHECK(buffer->Item(4) == 1);
    CHECK(buffer->Item(7) == 4);
    CHECK_FALSE(buffer->SetRange(bytes, 5u));
    buffer->Set(0, 512.0);
    CHECK(buffer->Item(0) == 255);
    CHECK(buffer->ZeroRange(4u, 4u));
    CHECK(buffer->Item(4) == 0);
    CHECK_FALSE(buffer->ZeroRange(6u, 4u));

    // The bytes are shared with the array the buffer was made from.
    auto data = dom::Uint8ClampedArray::Create(8u);
    auto shared = ByteArrayPixelBuffer::TryCreate(format, IntSize(2, 1), Ref<dom::Uint8ClampedArray>(data));
    REQUIRE(shared);
    shared->Set(3, 7);
    CHECK(data->Item(3) == 7);
    CHECK(&shared->Data() == data.get());
    CHECK_FALSE(ByteArrayPixelBuffer::TryCreate(format, IntSize(3, 1), Ref<dom::Uint8ClampedArray>(data)));

    // A copy of the right length makes a buffer; the source view checks the length too.
    auto copied = ByteArrayPixelBuffer::Create(format, IntSize(1, 1), Span<const uint8>(bytes));
    REQUIRE(copied);
    CHECK((*copied)->Item(2) == 3);
    CHECK(PixelBufferSourceView::Create(format, IntSize(1, 1), bytes));
    CHECK_FALSE(PixelBufferSourceView::Create(format, IntSize(2, 1), bytes));
    CHECK(shared->CreateScratchPixelBuffer(IntSize(4, 4))->Size() == IntSize(4, 4));
  }

  TEST_CASE("PixelBufferConversion: converting between colour spaces",
            "[Graphics][PixelBuffer][ColorManagement]")
  {
    auto format =
      [](AlphaPremultiplication alpha, PixelFormat pixelFormat, const DestinationColorSpace &colorSpace)
    {
      return PixelBufferFormat {alpha, pixelFormat, colorSpace};
    };
    const auto srgb =
      format(AlphaPremultiplication::Unpremultiplied, PixelFormat::RGBA8, DestinationColorSpace::SRGB());
    const auto p3 =
      format(AlphaPremultiplication::Unpremultiplied, PixelFormat::RGBA8, DestinationColorSpace::DisplayP3());
    const auto linear = format(AlphaPremultiplication::Unpremultiplied, PixelFormat::RGBA8,
                               DestinationColorSpace::LinearSRGB());

    // sRGB's red is inside Display P3's gamut; Display P3's red is outside sRGB's, so it's clipped.
    CHECK(Convert({255, 0, 0, 255}, srgb, p3) == Pixel {234, 51, 35, 255});
    CHECK(Convert({255, 0, 0, 255}, p3, srgb) == Pixel {255, 0, 0, 255});
    CHECK(Convert({234, 51, 35, 255}, p3, srgb) == Pixel {255, 0, 0, 255});
    // Mid grey is about a fifth of white's light.
    CHECK(Convert({128, 128, 128, 255}, srgb, linear) == Pixel {55, 55, 55, 255});
    CHECK(Convert({55, 55, 55, 255}, linear, srgb) == Pixel {128, 128, 128, 255});

    // Colours are converted unpremultiplied, whatever the alpha formats and channel orders.
    const auto premultipliedBGRAP3 =
      format(AlphaPremultiplication::Premultiplied, PixelFormat::BGRA8, DestinationColorSpace::DisplayP3());
    CHECK(Convert({255, 0, 0, 128}, srgb, premultipliedBGRAP3) == Pixel {18, 26, 117, 128});
    CHECK(Convert({0, 0, 128, 128}, premultipliedBGRAP3, srgb) == Pixel {255, 0, 0, 128});
    CHECK(Convert({255, 0, 0, 0}, srgb, premultipliedBGRAP3) == Pixel {0, 0, 0, 0});
  }

  TEST_CASE("PixelBufferConversion: premultiplying, unpremultiplying and swapping channels",
            "[Graphics][PixelBuffer]")
  {
    const auto unpremultipliedRGBA = Format(AlphaPremultiplication::Unpremultiplied, PixelFormat::RGBA8);
    const auto premultipliedRGBA = Format(AlphaPremultiplication::Premultiplied, PixelFormat::RGBA8);
    const auto premultipliedBGRA = Format(AlphaPremultiplication::Premultiplied, PixelFormat::BGRA8);
    const auto unpremultipliedBGRA = Format(AlphaPremultiplication::Unpremultiplied, PixelFormat::BGRA8);

    // Premultiplying rounds up; unpremultiplying rounds down.
    CHECK(Convert({200, 100, 50, 128}, unpremultipliedRGBA, premultipliedRGBA) == Pixel {101, 51, 26, 128});
    CHECK(Convert({101, 51, 26, 128}, premultipliedRGBA, unpremultipliedRGBA) == Pixel {201, 101, 51, 128});
    // Opaque and transparent pixels are kept, except that a premultiplied transparent pixel is zero.
    CHECK(Convert({10, 20, 30, 255}, unpremultipliedRGBA, premultipliedRGBA) == Pixel {10, 20, 30, 255});
    CHECK(Convert({5, 6, 7, 0}, premultipliedRGBA, premultipliedRGBA) == Pixel {5, 6, 7, 0});
    CHECK(Convert({5, 6, 7, 0}, unpremultipliedRGBA, premultipliedBGRA) == Pixel {0, 0, 0, 0});
    CHECK(Convert({5, 6, 7, 0}, unpremultipliedRGBA, unpremultipliedRGBA) == Pixel {5, 6, 7, 0});

    // Red and blue swap between RGBA and BGRA.
    CHECK(Convert({10, 20, 30, 255}, premultipliedRGBA, premultipliedBGRA) == Pixel {30, 20, 10, 255});
    CHECK(Convert({10, 20, 30, 40}, unpremultipliedRGBA, unpremultipliedBGRA) == Pixel {30, 20, 10, 40});
    CHECK(Convert({200, 100, 50, 128}, unpremultipliedRGBA, premultipliedBGRA) == Pixel {26, 51, 101, 128});
    CHECK(Convert({26, 51, 101, 128}, premultipliedBGRA, unpremultipliedRGBA) == Pixel {201, 101, 51, 128});

    // Rows of different strides.
    const uint8 source[] {1, 2, 3, 4, 9, 9, 9, 9, 5, 6, 7, 8, 9, 9, 9, 9};
    uint8 destination[8] {};
    ConvertImagePixels({unpremultipliedRGBA, 8u, source}, {unpremultipliedRGBA, 4u, destination},
                       IntSize(1, 2));
    CHECK(destination[0] == 1);
    CHECK(destination[4] == 5);
    CHECK(destination[7] == 8);

    uint8 copied[8] {};
    CopyRows(8u, source, 4u, copied, 2u, 4u);
    CHECK(copied[3] == 4);
    CHECK(copied[4] == 5);
  }
}
