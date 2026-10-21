#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  // FIXME: RGBA10 and RGBA16F (and their extended colour spaces, contentsFormatExtendedColorSpace), once
  // there are wide gamut and HDR backings.
  enum class ContentsFormat : uint8
  {
    RGBA8,
  };

  KRYS_NODISCARD constexpr uint32 ContentsFormatBytesPerPixel(ContentsFormat contentsFormat, bool) noexcept
  {
    switch (contentsFormat)
    {
      case ContentsFormat::RGBA8: return 4;
    }

    return 4;
  }

  TextStream &operator<<(TextStream &ts, ContentsFormat contentsFormat) noexcept;
}
