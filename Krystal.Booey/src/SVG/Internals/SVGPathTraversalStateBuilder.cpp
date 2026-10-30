#include "Krystal.Booey/SVG/Internals/SVGPathTraversalStateBuilder.hpp"
#include "Krystal.Booey/Graphics/PathTraversalState.hpp"

namespace krys::boo::svg
{
  SVGPathTraversalStateBuilder::SVGPathTraversalStateBuilder(PathTraversalState &state,
                                                             float desiredLength) noexcept
      : _traversalState(state)
  {
    _traversalState.SetDesiredLength(desiredLength);
  }

  void SVGPathTraversalStateBuilder::MoveTo(const FloatPoint &targetPoint, bool, PathCoordinateMode) noexcept
  {
    (void)_traversalState.ProcessPathElement(PathElement::Type::MoveToPoint, std::span {&targetPoint, 1});
  }

  void SVGPathTraversalStateBuilder::LineTo(const FloatPoint &targetPoint, PathCoordinateMode) noexcept
  {
    (void)_traversalState.ProcessPathElement(PathElement::Type::AddLineToPoint, std::span {&targetPoint, 1});
  }

  void SVGPathTraversalStateBuilder::CurveToCubic(const FloatPoint &point1, const FloatPoint &point2,
                                                  const FloatPoint &targetPoint, PathCoordinateMode) noexcept
  {
    std::array points {point1, point2, targetPoint};

    (void)_traversalState.ProcessPathElement(PathElement::Type::AddCurveToPoint,
                                             std::span<const FloatPoint> {points});
  }

  void SVGPathTraversalStateBuilder::ClosePath() noexcept
  {
    (void)_traversalState.ProcessPathElement(PathElement::Type::CloseSubpath, {});
  }

  bool SVGPathTraversalStateBuilder::ContinueConsuming() noexcept
  {
    return !_traversalState.Success();
  }

  float SVGPathTraversalStateBuilder::TotalLength() const noexcept
  {
    return _traversalState.TotalLength();
  }

  FloatPoint SVGPathTraversalStateBuilder::CurrentPoint() const noexcept
  {
    return _traversalState.Current();
  }
}
