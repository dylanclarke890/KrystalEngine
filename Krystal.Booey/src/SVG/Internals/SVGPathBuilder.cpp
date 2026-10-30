#include "Krystal.Booey/SVG/Internals/SVGPathBuilder.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"

namespace krys::boo::svg
{
  SVGPathBuilder::SVGPathBuilder(Path &path) noexcept : _path(path)
  {
  }

  void SVGPathBuilder::MoveTo(const FloatPoint &targetPoint, bool closed, PathCoordinateMode mode) noexcept
  {
    _current = mode == PathCoordinateMode::AbsoluteCoordinates ? targetPoint : _current + targetPoint;
    if (closed && !_path.IsEmpty())
    {
      _path.CloseSubpath();
    }
    _path.MoveTo(_current);
  }

  void SVGPathBuilder::LineTo(const FloatPoint &targetPoint, PathCoordinateMode mode) noexcept
  {
    _current = mode == PathCoordinateMode::AbsoluteCoordinates ? targetPoint : _current + targetPoint;
    _path.AddLineTo(_current);
  }

  void SVGPathBuilder::CurveToCubic(const FloatPoint &point1, const FloatPoint &point2,
                                    const FloatPoint &targetPoint, PathCoordinateMode mode) noexcept
  {
    if (mode == PathCoordinateMode::RelativeCoordinates)
    {
      _path.AddBezierCurveTo(_current + point1, _current + point2, _current + targetPoint);
      _current += targetPoint;
    }
    else
    {
      _current = targetPoint;
      _path.AddBezierCurveTo(point1, point2, _current);
    }
  }

  void SVGPathBuilder::ClosePath() noexcept
  {
    _path.CloseSubpath();
  }
}
