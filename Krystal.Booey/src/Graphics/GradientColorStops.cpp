#include "Krystal.Booey/Graphics/GradientColorStops.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, const GradientColorStops &stops) noexcept
  {
    return StreamSizedContainer(ts, stops.Stops());
  }
}
