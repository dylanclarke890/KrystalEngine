#pragma once

#include "Krystal.Booey/Graphics/DestinationColorSpace.hpp"
#include "Krystal.Booey/Graphics/PixelFormat.hpp"
#include "Krystal.Core/Colour/AlphaPremultiplication.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief How a pixel buffer's pixels are stored: premultiplied or not, their channels' layout and their
  /// colour space.
  struct PixelBufferFormat
  {
    AlphaPremultiplication AlphaFormat;
    PixelFormat Format;
    DestinationColorSpace ColorSpace;

    KRYS_NODISCARD bool operator==(const PixelBufferFormat &) const noexcept = default;
  };

  TextStream &operator<<(TextStream &, const PixelBufferFormat &) noexcept;
}
