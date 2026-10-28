#include "Krystal.Booey/HTML/ImageData.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::tests
{
  using html::ImageData;
  using html::ImageDataArray;
  using html::ImageDataPixelFormat;
  using html::PredefinedColorSpace;

  TEST_CASE("ImageData: made from a size", "[HTML][ImageData]")
  {
    auto imageData = ImageData::Create(3u, 2u, {});
    REQUIRE_FALSE(imageData.HasException());
    auto &data = *imageData.Value();
    CHECK(data.Width() == 3);
    CHECK(data.Height() == 2);
    CHECK(data.Data().Length() == 24u);
    CHECK(data.ColorSpace() == PredefinedColorSpace::SRGB);
    CHECK(data.PixelFormat() == ImageDataPixelFormat::RgbaUnorm8);
    CHECK(data.Data().AsUint8ClampedArray()->Item(23) == 0);

    CHECK(ImageData::Create(0u, 2u, {}).GetException().Code() == dom::ExceptionCode::IndexSizeError);
    CHECK(ImageData::Create(2u, 0u, {}).GetException().Code() == dom::ExceptionCode::IndexSizeError);
    CHECK(ImageData::Create(100'000u, 100'000u, {}).GetException().Code() == dom::ExceptionCode::RangeError);

    // The settings' colour space and format are kept.
    auto p3 = ImageData::Create(1u, 1u,
                                html::ImageDataSettings {.ColorSpace = PredefinedColorSpace::DisplayP3,
                                                         .Format = ImageDataPixelFormat::RgbaFloat16});
    REQUIRE_FALSE(p3.HasException());
    CHECK(p3.Value()->ColorSpace() == PredefinedColorSpace::DisplayP3);
    CHECK(p3.Value()->PixelFormat() == ImageDataPixelFormat::RgbaFloat16);
    // A default colour space is used when the settings have none.
    auto defaulted = ImageData::Create(1u, 1u, PredefinedColorSpace::DisplayP3, html::ImageDataSettings {});
    CHECK(defaulted.Value()->ColorSpace() == PredefinedColorSpace::DisplayP3);
  }

  TEST_CASE("ImageData: made from pixels", "[HTML][ImageData]")
  {
    auto bytes = dom::Uint8ClampedArray::Create(16u);
    bytes->Set(4, 200);

    auto imageData = ImageData::Create(ImageDataArray(Ref<dom::Uint8ClampedArray>(bytes)), 2u, {}, {});
    REQUIRE_FALSE(imageData.HasException());
    CHECK(imageData.Value()->Size() == IntSize(2, 2));
    // The pixels are shared, not copied.
    CHECK(imageData.Value()->Data().AsUint8ClampedArray().get() == bytes.get());

    CHECK(ImageData::Create(ImageDataArray(Ref<dom::Uint8ClampedArray>(bytes)), 2u, 2u, {}).Value()->Height()
          == 2);
    CHECK(
      ImageData::Create(ImageDataArray(Ref<dom::Uint8ClampedArray>(bytes)), 2u, 3u, {}).GetException().Code()
      == dom::ExceptionCode::IndexSizeError);
    CHECK(
      ImageData::Create(ImageDataArray(Ref<dom::Uint8ClampedArray>(bytes)), 3u, {}, {}).GetException().Code()
      == dom::ExceptionCode::IndexSizeError);
    CHECK(
      ImageData::Create(ImageDataArray(Ref<dom::Uint8ClampedArray>(bytes)), 0u, {}, {}).GetException().Code()
      == dom::ExceptionCode::IndexSizeError);
    CHECK(
      ImageData::Create(ImageDataArray(dom::Uint8ClampedArray::Create(6u)), 1u, {}, {}).GetException().Code()
      == dom::ExceptionCode::InvalidStateError);
    CHECK(
      ImageData::Create(ImageDataArray(dom::Uint8ClampedArray::Create(0u)), 1u, {}, {}).GetException().Code()
      == dom::ExceptionCode::InvalidStateError);

    // A copy of bytes.
    const uint8 source[] {9, 8, 7, 6};
    auto copied = ImageData::Create(1u, 1u, PredefinedColorSpace::SRGB, {}, source);
    CHECK(copied.Value()->Data().AsUint8ClampedArray()->Item(1) == 8);
  }

  TEST_CASE("ImageData: its pixel buffer shares its pixels", "[HTML][ImageData]")
  {
    auto imageData = ImageData::Create(IntSize(2, 1), PredefinedColorSpace::SRGB);
    REQUIRE(imageData);
    auto pixelBuffer = imageData->GetByteArrayPixelBuffer();
    CHECK(pixelBuffer->Format().AlphaFormat == AlphaPremultiplication::Unpremultiplied);
    CHECK(pixelBuffer->Format().Format == PixelFormat::RGBA8);
    CHECK(pixelBuffer->Size() == IntSize(2, 1));
    pixelBuffer->Set(5, 77);
    CHECK(imageData->Data().AsUint8ClampedArray()->Item(5) == 77);

    // Image data made from a pixel buffer shares its bytes too.
    auto fromBuffer = ImageData::Create(Ref<PixelBuffer>(pixelBuffer));
    REQUIRE(fromBuffer);
    CHECK(fromBuffer->Data().AsUint8ClampedArray().get() == &pixelBuffer->Data());
    CHECK(fromBuffer->ColorSpace() == PredefinedColorSpace::SRGB);

    CHECK_FALSE(ImageData::Create(IntSize(100'000, 100'000), PredefinedColorSpace::SRGB));
  }
}
