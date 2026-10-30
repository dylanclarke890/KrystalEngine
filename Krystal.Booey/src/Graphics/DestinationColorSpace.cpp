#include "Krystal.Booey/Graphics/DestinationColorSpace.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  const DestinationColorSpace &DestinationColorSpace::SRGB() noexcept
  {
    static constexpr DestinationColorSpace srgb(Space::SRGB);
    return srgb;
  }

  const DestinationColorSpace &DestinationColorSpace::LinearSRGB() noexcept
  {
    static constexpr DestinationColorSpace linearSRGB(Space::LinearSRGB);
    return linearSRGB;
  }

  TextStream &operator<<(TextStream &ts, const DestinationColorSpace &colorSpace) noexcept
  {
    if (colorSpace == DestinationColorSpace::SRGB())
    {
      ts << u8"sRGB";
    }
    else if (colorSpace == DestinationColorSpace::LinearSRGB())
    {
      ts << u8"LinearSRGB";
    }
    return ts;
  }
}
