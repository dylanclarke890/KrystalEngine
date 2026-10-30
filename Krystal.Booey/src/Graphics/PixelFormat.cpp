#include "Krystal.Booey/Graphics/PixelFormat.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, PixelFormat pixelFormat) noexcept
  {
    switch (pixelFormat)
    {
      case PixelFormat::RGBA8: ts << "RGBA8"; break;
      case PixelFormat::BGRX8: ts << "BGRX8"; break;
      case PixelFormat::BGRA8: ts << "BGRA8"; break;
    }
    return ts;
  }
}
