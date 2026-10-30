#pragma once

#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Graphics/ByteArrayPixelBuffer.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/HTML/Canvas/PredefinedColorSpace.hpp"
#include "Krystal.Booey/HTML/ImageDataArray.hpp"
#include "Krystal.Booey/HTML/ImageDataSettings.hpp"
#include "Krystal.Core/RefCounted.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo::html
{
  /// @brief A rectangle of unpremultiplied pixels a 2D context reads and writes.
  /// @see https://html.spec.whatwg.org/#imagedata
  class ImageData final : public RefCounted<ImageData>
  {
  private:
    IntSize _size;
    ImageDataArray _data;
    PredefinedColorSpace _colorSpace;

  public:
    /// @brief Image data sharing @p pixelBuffer's bytes.
    KRYS_NODISCARD static Ref<ImageData> Create(Ref<ByteArrayPixelBuffer> &&pixelBuffer,
                                                Maybe<ImageDataPixelFormat> = {}) noexcept;
    KRYS_NODISCARD static RefPtr<ImageData> Create(Ref<PixelBuffer> &&pixelBuffer,
                                                   Maybe<ImageDataPixelFormat> = {}) noexcept;
    KRYS_NODISCARD static RefPtr<ImageData> Create(RefPtr<ByteArrayPixelBuffer> &&pixelBuffer,
                                                   Maybe<ImageDataPixelFormat> = {}) noexcept;
    /// @brief Transparent black image data of @p size; null if it's too big.
    KRYS_NODISCARD static RefPtr<ImageData>
      Create(const IntSize &size, PredefinedColorSpace,
             ImageDataPixelFormat = ImageDataPixelFormat::RgbaUnorm8) noexcept;
    /// @brief Image data of @p size holding @p array; null if it's not as long as @p size's pixels.
    KRYS_NODISCARD static RefPtr<ImageData> Create(const IntSize &size, ImageDataArray &&array,
                                                   PredefinedColorSpace) noexcept;
    /// @brief `new ImageData(sw, sh, settings)` in @p defaultColorSpace, a copy of @p bytes or transparent
    /// black when it's empty.
    KRYS_NODISCARD static dom::ExceptionOr<Ref<ImageData>> Create(uint32 sw, uint32 sh,
                                                                  PredefinedColorSpace defaultColorSpace,
                                                                  Maybe<ImageDataSettings> = {},
                                                                  Span<const uint8> bytes = {}) noexcept;
    /// @see https://html.spec.whatwg.org/#dom-imagedata
    KRYS_NODISCARD static dom::ExceptionOr<Ref<ImageData>> Create(uint32 sw, uint32 sh,
                                                                  Maybe<ImageDataSettings>) noexcept;
    /// @see https://html.spec.whatwg.org/#dom-imagedata-with-data
    KRYS_NODISCARD static dom::ExceptionOr<Ref<ImageData>>
      Create(ImageDataArray &&, uint32 sw, Maybe<uint32> sh, Maybe<ImageDataSettings>) noexcept;

    ~ImageData() noexcept;

    KRYS_NODISCARD static PredefinedColorSpace
      ComputeColorSpace(Maybe<ImageDataSettings>,
                        PredefinedColorSpace defaultColorSpace = PredefinedColorSpace::SRGB) noexcept;

    KRYS_NODISCARD const IntSize &Size() const noexcept
    {
      return _size;
    }
    KRYS_NODISCARD int32 Width() const noexcept
    {
      return _size.Width();
    }
    KRYS_NODISCARD int32 Height() const noexcept
    {
      return _size.Height();
    }
    KRYS_NODISCARD const ImageDataArray &Data() const noexcept
    {
      return _data;
    }
    KRYS_NODISCARD PredefinedColorSpace ColorSpace() const noexcept
    {
      return _colorSpace;
    }
    KRYS_NODISCARD ImageDataPixelFormat PixelFormat() const noexcept
    {
      return _data.PixelFormat();
    }

    /// @brief A pixel buffer sharing the pixels.
    KRYS_NODISCARD Ref<ByteArrayPixelBuffer> GetByteArrayPixelBuffer() const noexcept;
    KRYS_NODISCARD Ref<PixelBuffer> GetPixelBuffer() const noexcept;

  private:
    ImageData(const IntSize &, ImageDataArray &&, PredefinedColorSpace) noexcept;
    ImageData(const IntSize &, ImageDataArray &&, PredefinedColorSpace, Maybe<ImageDataPixelFormat>) noexcept;
  };

  TextStream &operator<<(TextStream &, const ImageData &) noexcept;
}
