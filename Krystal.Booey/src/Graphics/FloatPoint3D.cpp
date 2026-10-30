#include "Krystal.Booey/Graphics/FloatPoint3D.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  void FloatPoint3D::Normalise() noexcept
  {
    float tempLength = Length();

    if (tempLength)
    {
      _x /= tempLength;
      _y /= tempLength;
      _z /= tempLength;
    }
  }

  TextStream &operator<<(TextStream &ts, const FloatPoint3D &point) noexcept
  {
    return ts << point.X() << u8' ' << point.Y() << u8' ' << point.Z();
  }
}
