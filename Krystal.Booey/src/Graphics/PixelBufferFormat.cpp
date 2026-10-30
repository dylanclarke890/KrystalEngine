#include "Krystal.Booey/Graphics/PixelBufferFormat.hpp"
#include "Krystal.Core/Colour/ColourTextStream.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, const PixelBufferFormat &pixelBuffer) noexcept
  {
    return ts << pixelBuffer.AlphaFormat << ' ' << pixelBuffer.Format << ' ' << pixelBuffer.ColorSpace;
  }
}
