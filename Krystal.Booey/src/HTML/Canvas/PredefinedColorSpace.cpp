#include "Krystal.Booey/HTML/Canvas/PredefinedColorSpace.hpp"
#include "Krystal.Booey/Graphics/DestinationColorSpace.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::html
{
  DestinationColorSpace ToDestinationColorSpace(PredefinedColorSpace colorSpace) noexcept
  {
    switch (colorSpace)
    {
      case PredefinedColorSpace::SRGB: return DestinationColorSpace::SRGB();
      case PredefinedColorSpace::DisplayP3:
        // FIXME: Display P3 destinations; canvases draw in sRGB until there are.
        return DestinationColorSpace::SRGB();
    }
    krys_debug_assert(false);
    return DestinationColorSpace::SRGB();
  }

  Maybe<PredefinedColorSpace> ToPredefinedColorSpace(const DestinationColorSpace &colorSpace) noexcept
  {
    if (colorSpace == DestinationColorSpace::SRGB())
    {
      return PredefinedColorSpace::SRGB;
    }
    return null;
  }
}
