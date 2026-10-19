#include "Krystal.Booey/Graphics/GradientColorStop.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, const GradientColorStop &stop) noexcept
  {
    return ts << stop.Color << " at " << stop.Offset;
  }
}
