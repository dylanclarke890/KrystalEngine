#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGEllipse.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/SVG/SVGCircleElement.hpp"
#include "Krystal.Booey/SVG/SVGEllipseElement.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"

namespace krys::boo
{
  LegacyRenderSVGEllipse::LegacyRenderSVGEllipse(svg::SVGGraphicsElement &element,
                                                 RenderStyle &&style) noexcept
      : LegacyRenderSVGShape(Type::LegacySVGEllipse, element, krys::move(style))
  {
  }

  LegacyRenderSVGEllipse::~LegacyRenderSVGEllipse() noexcept = default;

  void LegacyRenderSVGEllipse::UpdateShapeFromElement() noexcept
  {
    // Before creating a new object we need to clear the cached bounding box
    // to avoid using garbage.
    ClearPath();
    _shapeType = ShapeType::Empty;
    _fillBoundingBox = FloatRect();
    _strokeBoundingBox = null;
    _approximateStrokeBoundingBox = null;
    _center = FloatPoint();
    _radii = FloatSize();

    CalculateRadiiAndCenter();

    // Spec: "A negative value is illegal. A value of zero disables rendering of the element."
    if (_radii.IsEmpty())
    {
      return;
    }

    if (_radii.Width() == _radii.Height())
    {
      _shapeType = ShapeType::Circle;
    }
    else
    {
      _shapeType = ShapeType::Ellipse;
    }

    if (HasNonScalingStroke())
    {
      // Fallback to path-based approach if shape has a non-scaling stroke.
      _fillBoundingBox = EnsurePath().BoundingRect();
      return;
    }

    _fillBoundingBox = FloatRect(_center.X() - _radii.Width(), _center.Y() - _radii.Height(),
                                 2 * _radii.Width(), 2 * _radii.Height());
    auto strokeBoundingBox = _fillBoundingBox;
    if (Style().HasStroke())
    {
      strokeBoundingBox.Inflate(StrokeWidth() / 2);
    }
    _strokeBoundingBox = strokeBoundingBox;
  }

  void LegacyRenderSVGEllipse::CalculateRadiiAndCenter() noexcept
  {
    auto &graphicsElement = GraphicsElement();
    svg::SVGLengthContext lengthContext(&graphicsElement);
    _center = FloatPoint(
      lengthContext.ValueForLength(Style().Cx(), style::ZoomNeeded {}, svg::SVGLengthMode::Width),
      lengthContext.ValueForLength(Style().Cy(), style::ZoomNeeded {}, svg::SVGLengthMode::Height));
    if (Is<svg::SVGCircleElement>(graphicsElement))
    {
      float radius = lengthContext.ValueForLength(Style().R(), style::ZoomNeeded {});
      _radii = FloatSize(radius, radius);
      return;
    }

    krys_debug_assert(Is<svg::SVGEllipseElement>(graphicsElement));

    const auto &rx = Style().Rx();
    const auto &ry = Style().Ry();
    _radii = FloatSize(
      lengthContext.ValueForLength(rx.IsAuto() ? ry : rx, style::ZoomNeeded {}, svg::SVGLengthMode::Width),
      lengthContext.ValueForLength(ry.IsAuto() ? rx : ry, style::ZoomNeeded {}, svg::SVGLengthMode::Height));
    if (rx.IsAuto())
    {
      _radii.SetWidth(_radii.Height());
    }
    else if (ry.IsAuto())
    {
      _radii.SetHeight(_radii.Width());
    }
  }

  void LegacyRenderSVGEllipse::FillShape(GraphicsContext &context) const noexcept
  {
    if (HasPath())
    {
      LegacyRenderSVGShape::FillShape(context);
      return;
    }
    context.FillEllipse(_fillBoundingBox);
  }

  void LegacyRenderSVGEllipse::StrokeShape(GraphicsContext &context) const noexcept
  {
    if (!Style().HasStroke() || !Style().GetStrokeWidth().IsPossiblyPositive())
    {
      return;
    }
    if (HasPath())
    {
      LegacyRenderSVGShape::StrokeShape(context);
      return;
    }
    context.StrokeEllipse(_fillBoundingBox);
  }

  bool LegacyRenderSVGEllipse::CanUseStrokeHitTestFastPath() const noexcept
  {
    // Non-scaling-stroke needs special handling.
    if (HasNonScalingStroke())
    {
      return false;
    }

    // We can compute intersections with continuous strokes on circles
    // without using a Path.
    return _shapeType == ShapeType::Circle && Style().StrokeDashArray().IsNone();
  }

  bool
    LegacyRenderSVGEllipse::ShapeDependentStrokeContains(const FloatPoint &point,
                                                         PointCoordinateSpace pointCoordinateSpace) noexcept
  {
    if (_radii.IsEmpty())
    {
      return false;
    }

    // The optimized code below does not support dash strokes and non-circle shape.
    // Thus we fallback to path-based approach in that case.
    if (!CanUseStrokeHitTestFastPath())
    {
      (void)EnsurePath();
      return LegacyRenderSVGShape::ShapeDependentStrokeContains(point, pointCoordinateSpace);
    }

    float halfStrokeWidth = StrokeWidth() / 2;
    FloatPoint centerOffset = FloatPoint(_center.X() - point.X(), _center.Y() - point.Y());
    return std::abs(centerOffset.Length() - _radii.Width()) <= halfStrokeWidth;
  }

  bool LegacyRenderSVGEllipse::ShapeDependentFillContains(const FloatPoint &point,
                                                          const WindRule) const noexcept
  {
    if (_radii.IsEmpty())
    {
      return false;
    }

    FloatPoint center = FloatPoint(_center.X() - point.X(), _center.Y() - point.Y());

    // This works by checking if the point satisfies the ellipse equation.
    // (x/rX)^2 + (y/rY)^2 <= 1
    float xrX = center.X() / _radii.Width();
    float yrY = center.Y() / _radii.Height();
    return xrX * xrX + yrY * yrY <= 1.0;
  }

  bool LegacyRenderSVGEllipse::IsRenderingDisabled() const noexcept
  {
    // A radius of zero disables rendering of the element, and results in an empty bounding box.
    return _fillBoundingBox.IsEmpty();
  }
}
