#include "Krystal.Core/Geometry/Point3.hpp"
#include "Krystal.Core/Strings/TextStream.hpp"

namespace krys
{
  void Point3F::Normalise() noexcept
  {
    if (auto length = Length(); length != 0)
    {
      *this = {_x / length, _y / length, _z / length};
    }
  }

  TextStream &operator<<(TextStream &ts, const Point3F &point) noexcept
  {
    return ts << point.X() << u8' ' << point.Y() << u8' ' << point.Z();
  }
}
