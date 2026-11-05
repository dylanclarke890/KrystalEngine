#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief The colour space an image's pixels are in.
  class DestinationColorSpace
  {
  public:
    enum class Space : uint8
    {
      SRGB,
      LinearSRGB
    };

  private:
    Space _space;

  public:
    KRYS_NODISCARD static const DestinationColorSpace &SRGB() noexcept;
    KRYS_NODISCARD static const DestinationColorSpace &LinearSRGB() noexcept;

    constexpr explicit DestinationColorSpace(Space space) noexcept : _space(space)
    {
    }

    KRYS_NODISCARD constexpr Space GetSpace() const noexcept
    {
      return _space;
    }

    KRYS_NODISCARD constexpr bool operator==(const DestinationColorSpace &) const noexcept = default;
  };

  TextStream &operator<<(TextStream &ts, const DestinationColorSpace &colorSpace) noexcept;
}
