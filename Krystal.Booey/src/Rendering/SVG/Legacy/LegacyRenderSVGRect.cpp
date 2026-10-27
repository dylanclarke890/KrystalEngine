#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Booey/SVG/SVGRectElement.hpp"

namespace krys::boo
{
  LegacyRenderSVGRect::LegacyRenderSVGRect(svg::SVGRectElement &element, RenderStyle &&style) noexcept
      : LegacyRenderSVGShape(Type::LegacySVGRect, element, krys::move(style))
  {
  }

  LegacyRenderSVGRect::~LegacyRenderSVGRect() noexcept = default;

  svg::SVGRectElement &LegacyRenderSVGRect::RectElement() const noexcept
  {
    return Downcast<svg::SVGRectElement>(LegacyRenderSVGShape::GraphicsElement());
  }

  void LegacyRenderSVGRect::UpdateShapeFromElement() noexcept
  {
    // Before creating a new object we need to clear the cached bounding box
    // to avoid using garbage.
    ClearPath();
    _shapeType = ShapeType::Empty;
    _fillBoundingBox = FloatRect();
    _strokeBoundingBox = null;
    _approximateStrokeBoundingBox = null;

    svg::SVGLengthContext lengthContext(&RectElement());
    const auto &style = Style();
    auto usedZoom = style.UsedZoomForLength();
    FloatSize boundingBoxSize(
      lengthContext.ValueForLength(style.Width(), usedZoom, svg::SVGLengthMode::Width),
      lengthContext.ValueForLength(style.Height(), usedZoom, svg::SVGLengthMode::Height));

    // Spec: "A negative value is illegal. A value of zero disables rendering of the element."
    if (boundingBoxSize.IsEmpty())
    {
      return;
    }

    if (lengthContext.ValueForLength(style.Rx(), style::ZoomNeeded {}, svg::SVGLengthMode::Width) > 0
        || lengthContext.ValueForLength(style.Ry(), style::ZoomNeeded {}, svg::SVGLengthMode::Height) > 0)
    {
      _shapeType = ShapeType::RoundedRectangle;
    }
    else
    {
      _shapeType = ShapeType::Rectangle;
    }

    if (_shapeType != ShapeType::Rectangle || HasNonScalingStroke())
    {
      // Fallback to path-based approach.
      _fillBoundingBox = EnsurePath().BoundingRect();
      return;
    }

    _fillBoundingBox = FloatRect(
      FloatPoint(lengthContext.ValueForLength(style.X(), style::ZoomNeeded {}, svg::SVGLengthMode::Width),
                 lengthContext.ValueForLength(style.Y(), style::ZoomNeeded {}, svg::SVGLengthMode::Height)),
      boundingBoxSize);

    auto strokeBoundingBox = _fillBoundingBox;
    if (style.HasStroke())
    {
      strokeBoundingBox.Inflate(StrokeWidth() / 2);
    }

    _strokeBoundingBox = strokeBoundingBox;
  }

  void LegacyRenderSVGRect::FillShape(GraphicsContext &context) const noexcept
  {
    if (HasPath())
    {
      LegacyRenderSVGShape::FillShape(context);
      return;
    }

    context.FillRect(_fillBoundingBox, FillRequiresClip() ? GraphicsContext::RequiresClipToRect::Yes
                                                          : GraphicsContext::RequiresClipToRect::No);
  }

  void LegacyRenderSVGRect::StrokeShape(GraphicsContext &context) const noexcept
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

    context.StrokeRect(_fillBoundingBox, StrokeWidth());
  }

  bool LegacyRenderSVGRect::CanUseStrokeHitTestFastPath() const noexcept
  {
    // Non-scaling-stroke needs special handling.
    if (HasNonScalingStroke())
    {
      return false;
    }

    // We can compute intersections with simple, continuous strokes on
    // regular rectangles without using a Path.
    return _shapeType == ShapeType::Rectangle && DefinitelyHasSimpleStroke();
  }

  // Returns true if the stroke is continuous and definitely uses miter joins.
  bool LegacyRenderSVGRect::DefinitelyHasSimpleStroke() const noexcept
  {
    // The four angles of a rect are 90 degrees. Using the formula at:
    // http://www.w3.org/TR/SVG/painting.html#StrokeMiterlimitProperty
    // when the join style of the rect is "miter", the ratio of the miterLength
    // to the stroke-width is found to be
    // miterLength / stroke-width = 1 / sin(45 degrees)
    //                            = 1 / (1 / sqrt(2))
    //                            = sqrt(2)
    //                            = 1.414213562373095...
    // When sqrt(2) exceeds the miterlimit, then the join style switches to
    // "bevel". When the miterlimit is greater than or equal to sqrt(2) then
    // the join style remains "miter".
    //
    // An approximation of sqrt(2) is used here because at certain precise
    // miterlimits, the join style used might not be correct (e.g. a miterlimit
    // of 1.4142135 should result in bevel joins, but may be drawn using miter
    // joins).
    return Style().StrokeDashArray().IsNone() && Style().JoinStyle() == LineJoin::Miter
           && Style().StrokeMiterLimit().Value.Value >= 1.5;
  }

  bool LegacyRenderSVGRect::ShapeDependentStrokeContains(const FloatPoint &point,
                                                         PointCoordinateSpace pointCoordinateSpace) noexcept
  {
    if (!CanUseStrokeHitTestFastPath())
    {
      (void)EnsurePath();
      return LegacyRenderSVGShape::ShapeDependentStrokeContains(point, pointCoordinateSpace);
    }

    auto halfStrokeWidth = StrokeWidth() / 2;
    auto halfWidth = _fillBoundingBox.Width() / 2;
    auto halfHeight = _fillBoundingBox.Height() / 2;

    auto fillBoundingBoxCenter =
      FloatPoint(_fillBoundingBox.X() + halfWidth, _fillBoundingBox.Y() + halfHeight);
    auto absDeltaX = std::abs(point.X() - fillBoundingBoxCenter.X());
    auto absDeltaY = std::abs(point.Y() - fillBoundingBoxCenter.Y());

    if (!(absDeltaX <= halfWidth + halfStrokeWidth && absDeltaY <= halfHeight + halfStrokeWidth))
    {
      return false;
    }

    return (halfWidth - halfStrokeWidth <= absDeltaX) || (halfHeight - halfStrokeWidth <= absDeltaY);
  }

  bool LegacyRenderSVGRect::ShapeDependentFillContains(const FloatPoint &point,
                                                       const WindRule fillRule) const noexcept
  {
    if (_shapeType == ShapeType::Empty)
    {
      return false;
    }
    if (_shapeType != ShapeType::Rectangle)
    {
      return LegacyRenderSVGShape::ShapeDependentFillContains(point, fillRule);
    }
    return _fillBoundingBox.Contains(point.X(), point.Y());
  }

  bool LegacyRenderSVGRect::IsRenderingDisabled() const noexcept
  {
    // A width or height of zero disables rendering for the element, and results in an empty bounding box.
    return _fillBoundingBox.IsEmpty();
  }
}
