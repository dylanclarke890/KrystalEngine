#pragma once

#include "Krystal.Booey/DOM/Types/Uint8ClampedArray.hpp"
#include "Krystal.Booey/HTML/ImageDataPixelFormat.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::html
{
  /// @brief An `ImageData`'s pixels, four channels each, row after row.
  /// FIXME: A `Float16Array` for `rgba-float16`; the pixels are 8 bits a channel whichever format is asked
  /// for, and the format is only remembered.
  class ImageDataArray
  {
  private:
    Ref<dom::Uint8ClampedArray> _data;
    ImageDataPixelFormat _pixelFormat {ImageDataPixelFormat::RgbaUnorm8};

  public:
    explicit ImageDataArray(Ref<dom::Uint8ClampedArray> &&data,
                            ImageDataPixelFormat pixelFormat = ImageDataPixelFormat::RgbaUnorm8) noexcept
        : _data(krys::move(data)), _pixelFormat(pixelFormat)
    {
    }
    ImageDataArray(ImageDataArray &&original, Maybe<ImageDataPixelFormat> overridingPixelFormat) noexcept
        : _data(krys::move(original._data)),
          _pixelFormat(overridingPixelFormat.value_or(original._pixelFormat))
    {
    }

    /// @brief @p length channels of @p pixelFormat, a copy of @p optionalBytes or zero when it's empty.
    KRYS_NODISCARD static Maybe<ImageDataArray> TryCreate(size_t length, ImageDataPixelFormat pixelFormat,
                                                          Span<const uint8> optionalBytes = {}) noexcept;

    KRYS_NODISCARD ImageDataPixelFormat PixelFormat() const noexcept
    {
      return _pixelFormat;
    }
    KRYS_NODISCARD size_t Length() const noexcept
    {
      return _data->Length();
    }
    KRYS_NODISCARD size_t ByteLength() const noexcept
    {
      return _data->ByteLength();
    }
    /// @brief Whether the pixels were given away; never, as there are no scripts to transfer them.
    KRYS_NODISCARD bool IsDetached() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD Ref<dom::Uint8ClampedArray> AsUint8ClampedArray() const noexcept
    {
      return _data;
    }
  };
}
