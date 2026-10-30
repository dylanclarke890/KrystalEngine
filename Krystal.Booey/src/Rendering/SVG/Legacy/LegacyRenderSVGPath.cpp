#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGPath.hpp"
#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceMarker.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResources.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
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
    ProcessMarkerPositions();
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

    // FIXME: This should not exist. However, currently SVG is relying on ordering of calculation of SVG2
    // strokeBoundingBox via layout() function for recursive SVGs (markers are pointing to each other
    // recursively). If we move to SVG2 computation, we no longer need this since SVG2 strokeBoundingBox does
    // not include markers rect (so we do not need to have this in LSBE). Right now, this exists only for
    // RepaintRectCalculation::Accurate, and it should be removed once
    // 1. We fix checkInsertion / checkEnclosure implementations. Currently they are not aligned to what the
    // spec requires.
    // 2. We move our RenderTreeAsText to avoid dumping Accurate repaint rect. We should dump
    // strokeBoundingBox or actual repaintBoundingBox instead. We fall back to path-based eager
    // strokeBoundingBox only when there are markers and it is not SVG2. There are several cases we use
    // approximate repaintBoundingBox. But only LegacyRenderSVGPath can reference to the other approximate
    // repaintBoundingBox via markers. The other resources including maskers, clippers etc. are already
    // computing bounding rect via Accurate eagerly. So they do not matter.
    // https://bugs.webkit.org/show_bug.cgi?id=263348
    if (!_markerPositions.empty())
    {
      (void)StrokeBoundingBox();
    }
  }

  FloatRect LegacyRenderSVGPath::AdjustStrokeBoundingBoxForMarkersAndZeroLengthLinecaps(
    RepaintRectCalculation repaintRectCalculation, FloatRect strokeBoundingBox) const noexcept
  {
    float strokeWidth = StrokeWidth();

    if (!_markerPositions.empty())
    {
      auto markerRect = MarkerRect(repaintRectCalculation, strokeWidth);
      if (!markerRect.IsNaN())
      {
        strokeBoundingBox.Unite(markerRect);
      }
    }

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

  namespace
  {
    KRYS_NODISCARD LegacyRenderSVGResourceMarker *
      MarkerForType(SVGMarkerType type, LegacyRenderSVGResourceMarker *markerStart,
                    LegacyRenderSVGResourceMarker *markerMid,
                    LegacyRenderSVGResourceMarker *markerEnd) noexcept
    {
      switch (type)
      {
        case StartMarker:
        {
          return markerStart;
        }
        case MidMarker:
        {
          return markerMid;
        }
        case EndMarker:
        {
          return markerEnd;
        }
      }

      krys_assert_msg(false, "Not reached");
      return nullptr;
    }
  }

  bool LegacyRenderSVGPath::ShouldGenerateMarkerPositions() const noexcept
  {
    if (!Style().HasMarkers())
    {
      return false;
    }

    if (!GraphicsElement().SupportsMarkers())
    {
      return false;
    }

    auto *resources = SVGResourcesCache::CachedResourcesForRenderer(*this);
    if (!resources)
    {
      return false;
    }

    return resources->MarkerStart() || resources->MarkerMid() || resources->MarkerEnd();
  }

  void LegacyRenderSVGPath::DrawMarkers(PaintInfo &paintInfo) noexcept
  {
    if (_markerPositions.empty())
    {
      return;
    }

    auto *resources = SVGResourcesCache::CachedResourcesForRenderer(*this);
    if (!resources)
    {
      return;
    }

    auto *markerStart = resources->MarkerStart();
    auto *markerMid = resources->MarkerMid();
    auto *markerEnd = resources->MarkerEnd();
    if (!markerStart && !markerMid && !markerEnd)
    {
      return;
    }

    float strokeWidth = StrokeWidth();
    for (const auto &position : _markerPositions)
    {
      if (auto *marker = MarkerForType(position.Type, markerStart, markerMid, markerEnd))
      {
        marker->Draw(paintInfo, marker->MarkerTransformation(position.Origin, position.Angle, strokeWidth));
      }
    }
  }

  FloatRect LegacyRenderSVGPath::MarkerRect(RepaintRectCalculation repaintRectCalculation,
                                            float strokeWidth) const noexcept
  {
    krys_debug_assert(!_markerPositions.empty());

    auto *resources = SVGResourcesCache::CachedResourcesForRenderer(*this);
    krys_debug_assert(resources);

    auto *markerStart = resources->MarkerStart();
    auto *markerMid = resources->MarkerMid();
    auto *markerEnd = resources->MarkerEnd();
    krys_debug_assert(markerStart || markerMid || markerEnd);

    FloatRect boundaries;
    for (const auto &position : _markerPositions)
    {
      if (auto *marker = MarkerForType(position.Type, markerStart, markerMid, markerEnd))
      {
        boundaries.Unite(marker->MarkerBoundaries(
          repaintRectCalculation,
          marker->MarkerTransformation(position.Origin, position.Angle, strokeWidth)));
      }
    }
    return boundaries;
  }

  void LegacyRenderSVGPath::ProcessMarkerPositions() noexcept
  {
    _markerPositions.clear();

    if (!ShouldGenerateMarkerPositions())
    {
      return;
    }

    krys_debug_assert(HasPath());

    SVGMarkerData markerData(_markerPositions,
                             SVGResourcesCache::CachedResourcesForRenderer(*this)->MarkerReverseStart());
    GetPath().ApplyElements([&markerData](const PathElement &pathElement)
                            { SVGMarkerData::UpdateFromPathElement(markerData, pathElement); });
    markerData.PathIsDone();
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
