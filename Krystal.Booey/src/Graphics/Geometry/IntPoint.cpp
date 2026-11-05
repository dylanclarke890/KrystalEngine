#include "Krystal.Booey/Graphics/Geometry/IntPoint.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/Geometry/IntRect.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

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

  TextStream &operator<<(TextStream &ts, const IntPoint &p) noexcept
  {
    return ts << '(' << p.X() << ',' << p.Y() << ')';
  }
}
