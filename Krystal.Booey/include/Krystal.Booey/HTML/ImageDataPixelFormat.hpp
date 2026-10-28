#pragma once

#include "Krystal.Booey/Graphics/PixelFormat.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#imagedatapixelformat
  enum class ImageDataPixelFormat : bool
  {
    RgbaUnorm8,
    RgbaFloat16,
  };

  /// @brief The pixel buffer format @p pixelFormat's pixels are read into and written from.
  // FIXME: RGBA16F for `rgba-float16`; image data is 8 bits a channel whichever is asked for.
  KRYS_NODISCARD constexpr PixelFormat ToPixelFormat(ImageDataPixelFormat) noexcept
  {
    return PixelFormat::RGBA8;
  }
}
