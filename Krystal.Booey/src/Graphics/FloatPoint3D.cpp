#include "Krystal.Booey/Graphics/FloatPoint3D.hpp"

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
}
