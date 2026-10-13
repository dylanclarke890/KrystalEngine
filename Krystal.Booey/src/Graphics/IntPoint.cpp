#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Core/Maths/Base.hpp"

namespace krys::boo
{
  IntPoint::IntPoint(const FloatPoint &point) noexcept
      : _x(ClampToInteger(point.X())), _y(ClampToInteger(point.Y()))
  {
  }
}
