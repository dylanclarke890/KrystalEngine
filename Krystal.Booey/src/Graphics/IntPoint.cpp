#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Core/Maths/Base.hpp"

namespace krys::boo
{
  IntPoint::IntPoint(const FloatPoint &point) noexcept
      : _x(ClampToInteger(point.X())), _y(ClampToInteger(point.Y()))
  {
  }

  IntPoint IntPoint::ConstrainedWithin(const IntRect &rect) const noexcept
  {
    return ConstrainedBetween(rect.MinXMinYCorner(), rect.MaxXMaxYCorner());
  }
}
