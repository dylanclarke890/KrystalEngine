#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::pal
{
  /// @brief The widest standard colour gamut a display shows (near enough).
  enum class DisplayGamut : uint8
  {
    SRGB,
    DisplayP3,
    Rec2020
  };

  /// @brief A colour's chromaticity: its CIE 1931 x and y.
  struct Chromaticity
  {
    float X {0.f};
    float Y {0.f};
  };

  /// @brief What colours a display shows.
  struct DisplayColour
  {
    DisplayGamut Gamut {DisplayGamut::SRGB};
    /// @brief Whether the display is showing high dynamic range (Windows' HDR is on for it).
    bool HighDynamicRange {false};
    /// @brief The bits each colour component is sent to the display with.
    uint32 BitsPerComponent {8u};
    /// @brief The display's peak luminance, in nits; 0 when it isn't known.
    float MaxLuminance {0.f};
    /// @brief The luminance standard dynamic range white is shown at, in nits (80 is sRGB's reference).
    float SDRWhiteLevel {80.f};

    KRYS_NODISCARD constexpr bool operator==(const DisplayColour &) const noexcept = default;
  };

  namespace detail
  {
    KRYS_NODISCARD constexpr float TriangleArea(Chromaticity a, Chromaticity b, Chromaticity c) noexcept
    {
      const float area = ((b.X - a.X) * (c.Y - a.Y) - (c.X - a.X) * (b.Y - a.Y)) / 2.f;
      return area < 0.f ? -area : area;
    }
  }

  /// @brief The gamut of a display whose primaries are @p red, @p green and @p blue, by the area they span
  /// against sRGB's (as Chrome rates gamuts by their size): Display P3's is about 1.36 times sRGB's, and
  /// Rec. 2020's about 1.89 times; a display within a few percent of one counts as showing it.
  KRYS_NODISCARD constexpr DisplayGamut GamutOfPrimaries(Chromaticity red, Chromaticity green,
                                                         Chromaticity blue) noexcept
  {
    constexpr float srgbArea = detail::TriangleArea({0.64f, 0.33f}, {0.30f, 0.60f}, {0.15f, 0.06f});
    const float ratio = detail::TriangleArea(red, green, blue) / srgbArea;
    if (ratio >= 1.8f)
    {
      return DisplayGamut::Rec2020;
    }
    if (ratio >= 1.3f)
    {
      return DisplayGamut::DisplayP3;
    }
    return DisplayGamut::SRGB;
  }
}
