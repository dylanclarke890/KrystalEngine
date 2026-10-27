#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGPath.hpp"
#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGSubpathData.hpp"
#include "Krystal.Booey/SVG/SVGPathElement.hpp"

namespace krys::boo
{
  LegacyRenderSVGPath::LegacyRenderSVGPath(svg::SVGGraphicsElement &element, RenderStyle &&style) noexcept
      : LegacyRenderSVGShape(Type::LegacySVGPath, element, krys::move(style))
  {
    krys_debug_assert(IsLegacyRenderSVGPath());
  }

  LegacyRenderSVGPath::~LegacyRenderSVGPath() noexcept = default;

  void LegacyRenderSVGPath::UpdateShapeFromElement() noexcept
  {
    ClearPath();
    _shapeType = ShapeType::Empty;
    _fillBoundingBox = EnsurePath().BoundingRect();
    _strokeBoundingBox = null;
    _approximateStrokeBoundingBox = null;
    // FIXME: Find the marker positions (processMarkerPositions), with markers (B11f).
    UpdateZeroLengthSubpaths();

    krys_debug_assert(HasPath());
    if (GetPath().IsEmpty())
    {
      return;
    }
    if (GetPath().DefinitelySingleLine())
    {
      _shapeType = ShapeType::Line;
    }
    else
    {
      _shapeType = ShapeType::Path;
    }

    // FIXME: Compute the stroke bounding box eagerly when there are markers, with markers (B11f).
  }

  FloatRect LegacyRenderSVGPath::AdjustStrokeBoundingBoxForMarkersAndZeroLengthLinecaps(
    RepaintRectCalculation, FloatRect strokeBoundingBox) const noexcept
  {
    float strokeWidth = StrokeWidth();

    // FIXME: Unite the markers' rect (markerRect), with markers (B11f).

    if (Style().HasStroke())
    {
      // FIXME: zero-length subpaths do not respect vector-effect = non-scaling-stroke.
      for (const auto &zeroLengthLinecapLocation : _zeroLengthLinecapLocations)
      {
        auto subpathRect = ZeroLengthSubpathRect(zeroLengthLinecapLocation, strokeWidth);
        if (!subpathRect.IsNaN())
        {
          strokeBoundingBox.Unite(subpathRect);
        }
      }
    }

    return strokeBoundingBox;
  }

  namespace
  {
    void UseStrokeStyleToFill(GraphicsContext &context) noexcept
    {
      if (auto *gradient = context.StrokeGradient())
      {
        context.SetFillGradient(ShareRef(*gradient), context.StrokeGradientSpaceTransform());
      }
      else if (auto *pattern = context.StrokePattern())
      {
        context.SetFillPattern(ShareRef(*pattern));
      }
      else
      {
        context.SetFillColor(context.StrokeColor());
      }
    }
  }

  void LegacyRenderSVGPath::StrokeShape(GraphicsContext &context) const noexcept
  {
    if (!Style().HasStroke() || !Style().GetStrokeWidth().IsPossiblyPositive())
    {
      return;
    }

    // This happens only if the layout was never been called for this element.
    if (!HasPath())
    {
      return;
    }

    LegacyRenderSVGShape::StrokeShape(context);
    StrokeZeroLengthSubpaths(context);
  }

  bool LegacyRenderSVGPath::ShapeDependentStrokeContains(const FloatPoint &point,
                                                         PointCoordinateSpace pointCoordinateSpace) noexcept
  {
    if (LegacyRenderSVGShape::ShapeDependentStrokeContains(point, pointCoordinateSpace))
    {
      return true;
    }

    for (const auto &location : _zeroLengthLinecapLocations)
    {
      krys_debug_assert(Style().HasStroke());
      float strokeWidth = StrokeWidth();
      if (Style().CapStyle() == LineCap::Square)
      {
        if (ZeroLengthSubpathRect(location, strokeWidth).Contains(point))
        {
          return true;
        }
      }
      else
      {
        krys_debug_assert(Style().CapStyle() == LineCap::Round);
        FloatPoint radiusVector(point.X() - location.X(), point.Y() - location.Y());
        if (radiusVector.LengthSquared() < strokeWidth * strokeWidth * .25f)
        {
          return true;
        }
      }
    }
    return false;
  }

  bool LegacyRenderSVGPath::ShouldStrokeZeroLengthSubpath() const noexcept
  {
    // Spec(11.4): Any zero length subpath shall not be stroked if the "stroke-linecap" property has a value
    // of butt but shall be stroked if the "stroke-linecap" property has a value of round or square
    return Style().HasStroke() && Style().CapStyle() != LineCap::Butt;
  }

  Path *LegacyRenderSVGPath::ZeroLengthLinecapPath(const FloatPoint &linecapPosition) const noexcept
  {
    static Path tempPath;

    tempPath.Clear();
    if (Style().CapStyle() == LineCap::Square)
    {
      tempPath.AddRect(ZeroLengthSubpathRect(linecapPosition, StrokeWidth()));
    }
    else
    {
      tempPath.AddEllipseInRect(ZeroLengthSubpathRect(linecapPosition, StrokeWidth()));
    }

    return &tempPath;
  }

  FloatRect LegacyRenderSVGPath::ZeroLengthSubpathRect(const FloatPoint &linecapPosition,
                                                       float strokeWidth) const noexcept
  {
    return FloatRect(linecapPosition.X() - strokeWidth / 2, linecapPosition.Y() - strokeWidth / 2,
                     strokeWidth, strokeWidth);
  }

  void LegacyRenderSVGPath::UpdateZeroLengthSubpaths() noexcept
  {
    _zeroLengthLinecapLocations.clear();

    if (!StrokeWidth() || !ShouldStrokeZeroLengthSubpath())
    {
      return;
    }

    SVGSubpathData subpathData(_zeroLengthLinecapLocations);
    GetPath().ApplyElements([&subpathData](const PathElement &pathElement)
                            { SVGSubpathData::UpdateFromPathElement(subpathData, pathElement); });
    subpathData.PathIsDone();
  }

  void LegacyRenderSVGPath::StrokeZeroLengthSubpaths(GraphicsContext &context) const noexcept
  {
    if (_zeroLengthLinecapLocations.empty())
    {
      return;
    }

    AffineTransform nonScalingTransform;
    if (HasNonScalingStroke())
    {
      nonScalingTransform = NonScalingStrokeTransform();
    }

    GraphicsContextStateSaver stateSaver(context, true);
    UseStrokeStyleToFill(context);
    for (const auto &location : _zeroLengthLinecapLocations)
    {
      auto *usePath = ZeroLengthLinecapPath(location);
      if (HasNonScalingStroke())
      {
        usePath = NonScalingStrokePath(usePath, nonScalingTransform);
      }
      context.FillPath(*usePath);
    }
  }

  void LegacyRenderSVGPath::DrawMarkers(PaintInfo &) noexcept
  {
    // FIXME: Draw the start, middle and end markers at their positions, with markers (B11f).
  }

  bool LegacyRenderSVGPath::IsRenderingDisabled() const noexcept
  {
    // For a polygon, polyline or path, rendering is disabled if there is no path data.
    // No path data is possible in the case of a missing or empty 'd' or 'points' attribute.
    return !HasPath() || GetPath().IsEmpty();
  }

  void LegacyRenderSVGPath::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    if (auto *pathElement = DynamicDowncast<svg::SVGPathElement>(GraphicsElement()))
    {
      if (!oldStyle || Style().D() != oldStyle->D())
      {
        pathElement->PathDidChange();
      }
    }

    LegacyRenderSVGShape::StyleDidChange(diff, oldStyle);
  }
}
