#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"

namespace krys::boo
{
  FloatPoint FloatPoint::ConstrainedWithin(const FloatRect &rect) const noexcept
  {
    return ConstrainedBetween(rect.MinXMinYCorner(), rect.MaxXMaxYCorner());
  }

  FloatPoint FloatPoint::MatrixTransform(const AffineTransform &transform) const noexcept
  {
    double newX, newY;
    transform.Map(static_cast<double>(_x), static_cast<double>(_y), newX, newY);
    return NarrowPrecision(newX, newY);
  }

  FloatPoint FloatPoint::MatrixTransform(const TransformationMatrix &transform) const noexcept
  {
    double newX, newY;
    transform.Map(static_cast<double>(_x), static_cast<double>(_y), newX, newY);
    return NarrowPrecision(newX, newY);
  }
}
