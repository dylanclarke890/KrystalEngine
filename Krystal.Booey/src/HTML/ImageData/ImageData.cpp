#include "Krystal.Booey/HTML/ImageData/ImageData.hpp"
#include "Krystal.Booey/Graphics/DestinationColorSpace.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include <cstring>
#include <limits>

namespace krys::boo::html
{
  namespace
  {
    KRYS_NODISCARD Maybe<uint32> ComputeDataSize(const IntSize &size,
                                                 ImageDataPixelFormat pixelFormat) noexcept
    {
      return PixelBuffer::ComputePixelComponentCount(ToPixelFormat(pixelFormat), size);
    }

    KRYS_NODISCARD ImageDataPixelFormat
      ComputePixelFormat(Maybe<ImageDataSettings> settings,
                         ImageDataPixelFormat defaultPixelFormat = ImageDataPixelFormat::RgbaUnorm8) noexcept
    {
      return settings ? settings->Format : defaultPixelFormat;
    }
  }

  Maybe<ImageDataArray> ImageDataArray::TryCreate(size_t length, ImageDataPixelFormat pixelFormat,
                                                  Span<const uint8> optionalBytes) noexcept
  {
    if (optionalBytes.empty())
    {
      return ImageDataArray(dom::Uint8ClampedArray::Create(length), pixelFormat);
    }
    krys_assert_msg(optionalBytes.size() == length, "Caller should provide correctly-sized buffer to copy");
    return ImageDataArray(dom::Uint8ClampedArray::Create(optionalBytes), pixelFormat);
  }

  PredefinedColorSpace ImageData::ComputeColorSpace(Maybe<ImageDataSettings> settings,
                                                    PredefinedColorSpace defaultColorSpace) noexcept
  {
    if (settings && settings->ColorSpace)
    {
      return *settings->ColorSpace;
    }
    return defaultColorSpace;
  }

  Ref<ImageData> ImageData::Create(Ref<ByteArrayPixelBuffer> &&pixelBuffer,
                                   Maybe<ImageDataPixelFormat> overridingPixelFormat) noexcept
  {
    auto colorSpace = ToPredefinedColorSpace(pixelBuffer->Format().ColorSpace);
    return AdoptRef(*new ImageData(pixelBuffer->Size(), ImageDataArray(pixelBuffer->TakeData()),
                                   colorSpace.value_or(PredefinedColorSpace::SRGB), overridingPixelFormat));
  }

  RefPtr<ImageData> ImageData::Create(RefPtr<ByteArrayPixelBuffer> &&pixelBuffer,
                                      Maybe<ImageDataPixelFormat> overridingPixelFormat) noexcept
  {
    if (!pixelBuffer)
    {
      return nullptr;
    }
    return Create(Ref<ByteArrayPixelBuffer>(krys::move(pixelBuffer)), overridingPixelFormat);
  }

  RefPtr<ImageData> ImageData::Create(Ref<PixelBuffer> &&pixelBuffer,
                                      Maybe<ImageDataPixelFormat> overridingPixelFormat) noexcept
  {
    if (auto *byteArray = DynamicDowncast<ByteArrayPixelBuffer>(*pixelBuffer))
    {
      return Create(ShareRef(*byteArray), overridingPixelFormat);
    }
    return nullptr;
  }

  RefPtr<ImageData> ImageData::Create(const IntSize &size, PredefinedColorSpace colorSpace,
                                      ImageDataPixelFormat imageDataPixelFormat) noexcept
  {
    auto dataSize = ComputeDataSize(size, ImageDataPixelFormat::RgbaUnorm8);
    if (!dataSize)
    {
      return nullptr;
    }
    auto array = ImageDataArray::TryCreate(*dataSize, ImageDataPixelFormat::RgbaUnorm8);
    if (!array)
    {
      return nullptr;
    }
    return AdoptRef(*new ImageData(size, krys::move(*array), colorSpace, imageDataPixelFormat));
  }

  RefPtr<ImageData> ImageData::Create(const IntSize &size, ImageDataArray &&array,
                                      PredefinedColorSpace colorSpace) noexcept
  {
    auto dataSize = ComputeDataSize(size, array.PixelFormat());
    if (!dataSize || *dataSize != array.Length())
    {
      return nullptr;
    }
    return AdoptRef(*new ImageData(size, krys::move(array), colorSpace));
  }

  dom::ExceptionOr<Ref<ImageData>> ImageData::Create(uint32 sw, uint32 sh,
                                                     PredefinedColorSpace defaultColorSpace,
                                                     Maybe<ImageDataSettings> settings,
                                                     Span<const uint8> optionalBytes) noexcept
  {
    // https://html.spec.whatwg.org/#dom-imagedata
    // 1. If one or both of sw and sh are zero, then throw an "IndexSizeError" DOMException.
    if (!sw || !sh)
    {
      return dom::Exception {dom::ExceptionCode::IndexSizeError};
    }

    // 2. Initialize this given sw, sh, and settings.
    // 3. Initialize the image data of this to transparent black.
    // https://html.spec.whatwg.org/#initialize-an-imagedata-object
    // 2a.-2c. Initialize the data attribute of imageData to a new Uint8ClampedArray object [...] If the
    //         storage ArrayBuffer could not be allocated, then rethrow the RangeError thrown by JavaScript.
    // (Every format has 8 bits a channel; see the FIXME on ImageDataArray.)

    constexpr auto maximum = static_cast<uint32>(std::numeric_limits<int32>::max());
    if (sw > maximum || sh > maximum)
    {
      return dom::Exception {dom::ExceptionCode::RangeError, u8"Cannot allocate a buffer of this size"};
    }
    IntSize size(static_cast<int32>(sw), static_cast<int32>(sh));
    auto pixelFormat = ComputePixelFormat(settings);
    auto dataSize = ComputeDataSize(size, pixelFormat);
    if (!dataSize)
    {
      return dom::Exception {dom::ExceptionCode::RangeError, u8"Cannot allocate a buffer of this size"};
    }

    auto array = ImageDataArray::TryCreate(*dataSize, pixelFormat, optionalBytes);
    if (!array)
    {
      return dom::Exception {dom::ExceptionCode::RangeError, u8"Out of memory"};
    }

    // 3.-8. Initialize the width attribute of imageData to pixelsPerRow. [...] Otherwise, initialize the
    //       colorSpace attribute of imageData to "srgb".
    auto colorSpace = ImageData::ComputeColorSpace(settings, defaultColorSpace);
    return AdoptRef(*new ImageData(size, krys::move(*array), colorSpace));
  }

  dom::ExceptionOr<Ref<ImageData>> ImageData::Create(uint32 sw, uint32 sh,
                                                     Maybe<ImageDataSettings> settings) noexcept
  {
    return Create(sw, sh, PredefinedColorSpace::SRGB, settings);
  }

  dom::ExceptionOr<Ref<ImageData>> ImageData::Create(ImageDataArray &&array, uint32 sw, Maybe<uint32> sh,
                                                     Maybe<ImageDataSettings> settings) noexcept
  {
    // https://html.spec.whatwg.org/#dom-imagedata-with-data
    // 1. Let bytesPerPixel be 4 if settings["pixelFormat"] is "rgba-unorm8"; otherwise 8.
    // (Every format has 8 bits a channel; see the FIXME on ImageDataArray.)
    // 2. Let length be the buffer source byte length of data.
    // 3. If length is not a nonzero integral multiple of bytesPerPixel, then throw an "InvalidStateError"
    //    DOMException.
    auto length = array.Length();
    if (!length || length % 4)
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError,
                             u8"Length is not a non-zero multiple of 4"};
    }

    // 4. Let length be length divided by bytesPerPixel.
    // 5. If length is not an integral multiple of sw, then throw an "IndexSizeError" DOMException.
    auto pixels = length / 4;
    if (!sw || pixels % sw)
    {
      return dom::Exception {dom::ExceptionCode::IndexSizeError, u8"Length is not a multiple of sw"};
    }

    // 6. Let height be length divided by sw.
    auto height = pixels / sw;
    if (height > static_cast<size_t>(std::numeric_limits<int32>::max())
        || sw > static_cast<uint32>(std::numeric_limits<int32>::max()))
    {
      return dom::Exception {dom::ExceptionCode::IndexSizeError, u8"Computed height is too big"};
    }

    // 7. If sh was given and its value is not equal to height, then throw an "IndexSizeError" DOMException.
    if (sh && *sh != height)
    {
      return dom::Exception {dom::ExceptionCode::IndexSizeError, u8"sh value is not equal to height"};
    }

    // 8. Initialize this given sw, sh, settings, and source set to data.
    // https://html.spec.whatwg.org/#initialize-an-imagedata-object
    // 1a.-1b. If settings["pixelFormat"] equals "rgba-unorm8" and source is not a Uint8ClampedArray [...],
    //         then throw an "InvalidStateError" DOMException.
    // (There's no Float16Array, so a Uint8ClampedArray is taken for every format; see the FIXME on
    // ImageDataArray.)
    IntSize size(static_cast<int32>(sw), static_cast<int32>(height));
    auto dataSize = ComputeDataSize(size, ComputePixelFormat(settings));
    if (!dataSize || *dataSize != length)
    {
      return dom::Exception {dom::ExceptionCode::RangeError};
    }

    auto colorSpace = ComputeColorSpace(settings);
    return AdoptRef(*new ImageData(size, krys::move(array), colorSpace));
  }

  ImageData::ImageData(const IntSize &size, ImageDataArray &&data, PredefinedColorSpace colorSpace) noexcept
      : _size(size), _data(krys::move(data)), _colorSpace(colorSpace)
  {
  }

  ImageData::ImageData(const IntSize &size, ImageDataArray &&data, PredefinedColorSpace colorSpace,
                       Maybe<ImageDataPixelFormat> overridingPixelFormat) noexcept
      : _size(size), _data(krys::move(data), overridingPixelFormat), _colorSpace(colorSpace)
  {
  }

  ImageData::~ImageData() noexcept = default;

  Ref<ByteArrayPixelBuffer> ImageData::GetByteArrayPixelBuffer() const noexcept
  {
    auto uint8Data = _data.AsUint8ClampedArray();
    PixelBufferFormat format {AlphaPremultiplication::Unpremultiplied, boo::PixelFormat::RGBA8,
                              ToDestinationColorSpace(_colorSpace)};
    return ByteArrayPixelBuffer::Create(format, _size, *uint8Data);
  }

  Ref<PixelBuffer> ImageData::GetPixelBuffer() const noexcept
  {
    return GetByteArrayPixelBuffer();
  }

  TextStream &operator<<(TextStream &ts, const ImageData &imageData) noexcept
  {
    // Print out the address of the pixel data array
    return ts << static_cast<const void *>(&imageData.Data());
  }
}
