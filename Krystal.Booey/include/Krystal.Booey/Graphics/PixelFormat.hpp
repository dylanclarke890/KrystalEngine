#pragma once

#include "Krystal.Booey/Graphics/ContentsFormat.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief How a pixel's channels are laid out in memory, 8 bits each.
  // FIXME: RGB10, RGB10A8 and RGBA16F, with wide gamut and HDR backings.
  enum class PixelFormat : uint8
  {
    RGBA8,
    BGRX8,
    BGRA8,
  };

  enum class UseLosslessCompression : bool
  {
    No,
    Yes
  };

  KRYS_NODISCARD constexpr ContentsFormat ConvertToContentsFormat(PixelFormat format) noexcept
  {
    switch (format)
    {
      case PixelFormat::RGBA8:
      case PixelFormat::BGRX8:
      case PixelFormat::BGRA8: return ContentsFormat::RGBA8;
    }
    return ContentsFormat::RGBA8;
  }

  KRYS_NODISCARD constexpr bool PixelFormatIsOpaque(PixelFormat format) noexcept
  {
    switch (format)
    {
      case PixelFormat::BGRX8: return true;
      case PixelFormat::RGBA8:
      case PixelFormat::BGRA8: return false;
    }
    return false;
  }

  TextStream &operator<<(TextStream &, PixelFormat) noexcept;
}
