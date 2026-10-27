#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGShape.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Rendering/HitTestRequest.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/LayoutRepainter.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/PointerEventsHitRules.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResource.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceSolidColor.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGPathData.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderingContext.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGVisitedRendererTracking.hpp"
#include "Krystal.Booey/SVG/SVGGraphicsElement.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"

namespace krys::boo
{
  LegacyRenderSVGShape::LegacyRenderSVGShape(Type type, svg::SVGGraphicsElement &element,
                                             RenderStyle &&style) noexcept
      : LegacyRenderSVGModelObject(type, element, krys::move(style),
                                   {SVGModelObjectFlag::IsShape, SVGModelObjectFlag::UsesBoundaryCaching})
  {
  }

  LegacyRenderSVGShape::~LegacyRenderSVGShape() noexcept = default;

  svg::SVGGraphicsElement &LegacyRenderSVGShape::GraphicsElement() const noexcept
  {
    return Downcast<svg::SVGGraphicsElement>(LegacyRenderSVGModelObject::GetElement());
  }

  bool LegacyRenderSVGShape::HasNonScalingStroke() const noexcept
  {
    return Style().GetVectorEffect() == VectorEffect::NonScalingStroke;
  }

  bool LegacyRenderSVGShape::IsEmpty() const noexcept
  {
    // This function should never be called before assigning a new Path to m_path.
    // But this bug can happen if this renderer was created and its layout was not
    // done before painting. Assert this did not happen but do not crash.
    krys_debug_assert(HasPath());
    return !HasPath() || GetPath().IsEmpty();
  }

  void LegacyRenderSVGShape::FillShape(GraphicsContext &context) const noexcept
  {
    context.FillPath(GetPath());
  }

  void LegacyRenderSVGShape::StrokeShape(GraphicsContext &context) const noexcept
  {
    krys_debug_assert(_path);
    Path *usePath = _path.get();

    if (HasNonScalingStroke())
    {
      usePath = NonScalingStrokePath(usePath, NonScalingStrokeTransform());
    }

    context.StrokePath(*usePath);
  }

  bool LegacyRenderSVGShape::ShapeDependentStrokeContains(const FloatPoint &point,
                                                          PointCoordinateSpace pointCoordinateSpace) noexcept
  {
    krys_debug_assert(_path);

    if (HasNonScalingStroke() && pointCoordinateSpace != LocalCoordinateSpace)
    {
      AffineTransform nonScalingTransform = NonScalingStrokeTransform();
      Path *usePath = NonScalingStrokePath(_path.get(), nonScalingTransform);
      return usePath->StrokeContains(
        nonScalingTransform.MapPoint(point), [this](GraphicsContext &context)
        { SVGRenderSupport::ApplyStrokeStyleToContext(context, Style(), *this); });
    }

    return _path->StrokeContains(point, [this](GraphicsContext &context)
                                 { SVGRenderSupport::ApplyStrokeStyleToContext(context, Style(), *this); });
  }

  bool LegacyRenderSVGShape::ShapeDependentFillContains(const FloatPoint &point,
                                                        const WindRule fillRule) const noexcept
  {
    return const_cast<LegacyRenderSVGShape &>(*this).EnsurePath().Contains(point, fillRule);
  }

  bool LegacyRenderSVGShape::FillContains(const FloatPoint &point, bool requiresFill,
                                          const WindRule fillRule) noexcept
  {
    if (_fillBoundingBox.IsEmpty() || !_fillBoundingBox.Contains(point))
    {
      return false;
    }

    Color fallbackColor;
    if (requiresFill && !LegacyRenderSVGResource::FillPaintingResource(*this, Style(), fallbackColor))
    {
      return false;
    }

    return ShapeDependentFillContains(point, fillRule);
  }

  bool LegacyRenderSVGShape::StrokeContains(const FloatPoint &point, bool requiresStroke) noexcept
  {
    // "A zero value causes no stroke to be painted."
    if (!StrokeWidth())
    {
      return false;
    }

    auto approximateStrokeBoundingBox = ApproximateStrokeBoundingBox();
    if (approximateStrokeBoundingBox.IsEmpty() || !approximateStrokeBoundingBox.Contains(point))
    {
      return false;
    }

    Color fallbackColor;
    if (requiresStroke && !LegacyRenderSVGResource::StrokePaintingResource(*this, Style(), fallbackColor))
    {
      return false;
    }

    return ShapeDependentStrokeContains(point);
  }

  void LegacyRenderSVGShape::Layout() noexcept
  {
    auto checkForRepaintOverride = !SelfNeedsLayout()
                                     ? LayoutRepainter::CheckForRepaint::No
                                     : SVGRenderSupport::CheckForSVGRepaintDuringLayout(*this);
    LayoutRepainter repainter(*this, checkForRepaintOverride, {}, RepaintOutlineBounds::No);

    bool updateCachedBoundariesInParents = false;

    if (_needsShapeUpdate || _needsBoundariesUpdate)
    {
      UpdateShapeFromElement();
      _needsShapeUpdate = false;
      UpdateRepaintBoundingBox();
      _needsBoundariesUpdate = false;
      updateCachedBoundariesInParents = true;
    }

    if (_needsTransformUpdate)
    {
      _localTransform = GraphicsElement().AnimatedLocalTransform();
      _needsTransformUpdate = false;
      updateCachedBoundariesInParents = true;
    }

    // Invalidate all resources of this client if our layout changed.
    // FIXME: SVGResourcesCache::clientLayoutChanged, with resources (B11e).

    // If our bounds changed, notify the parents.
    if (updateCachedBoundariesInParents)
    {
      if (auto *parent = Parent())
      {
        parent->InvalidateCachedBoundaries();
      }
    }

    repainter.RepaintAfterLayout();
    ClearNeedsLayout();
  }

  Path *LegacyRenderSVGShape::NonScalingStrokePath(const Path *path,
                                                   const AffineTransform &strokeTransform) const noexcept
  {
    static Path tempPath;

    tempPath = *path;
    tempPath.Transform(strokeTransform);

    return &tempPath;
  }

  bool LegacyRenderSVGShape::SetupNonScalingStrokeContext(AffineTransform &strokeTransform,
                                                          GraphicsContextStateSaver &stateSaver) noexcept
  {
    Maybe<AffineTransform> inverse = strokeTransform.Inverse();
    if (!inverse)
    {
      return false;
    }

    stateSaver.Save();
    stateSaver.Context()->ConcatCTM(inverse.value());
    return true;
  }

  AffineTransform LegacyRenderSVGShape::NonScalingStrokeTransform() const noexcept
  {
    return GraphicsElement().GetScreenCTM(svg::SVGLocatable::DisallowStyleUpdate);
  }

  void LegacyRenderSVGShape::FillShape(const RenderStyle &style, GraphicsContext &originalContext) noexcept
  {
    GraphicsContext *context = &originalContext;
    Color fallbackColor;
    if (auto *fillPaintingResource =
          LegacyRenderSVGResource::FillPaintingResource(*this, style, fallbackColor))
    {
      if (ResourceWasApplied(
            fillPaintingResource->ApplyResource(*this, style, context, RenderSVGResourceMode::ApplyToFill)))
      {
        fillPaintingResource->PostApplyResource(*this, context, RenderSVGResourceMode::ApplyToFill, nullptr,
                                                this);
      }
      else if (fallbackColor.IsValid())
      {
        auto *fallbackResource = LegacyRenderSVGResource::SharedSolidPaintingResource();
        fallbackResource->SetColor(fallbackColor);
        if (ResourceWasApplied(
              fallbackResource->ApplyResource(*this, style, context, RenderSVGResourceMode::ApplyToFill)))
        {
          fallbackResource->PostApplyResource(*this, context, RenderSVGResourceMode::ApplyToFill, nullptr,
                                              this);
        }
      }
    }
  }

  void LegacyRenderSVGShape::StrokeShapeInternal(const RenderStyle &style,
                                                 GraphicsContext &originalContext) noexcept
  {
    GraphicsContext *context = &originalContext;
    Color fallbackColor;
    if (auto *strokePaintingResource =
          LegacyRenderSVGResource::StrokePaintingResource(*this, style, fallbackColor))
    {
      if (ResourceWasApplied(strokePaintingResource->ApplyResource(*this, style, context,
                                                                   RenderSVGResourceMode::ApplyToStroke)))
      {
        strokePaintingResource->PostApplyResource(*this, context, RenderSVGResourceMode::ApplyToStroke,
                                                  nullptr, this);
      }
      else if (fallbackColor.IsValid())
      {
        auto *fallbackResource = LegacyRenderSVGResource::SharedSolidPaintingResource();
        fallbackResource->SetColor(fallbackColor);
        if (ResourceWasApplied(
              fallbackResource->ApplyResource(*this, style, context, RenderSVGResourceMode::ApplyToStroke)))
        {
          fallbackResource->PostApplyResource(*this, context, RenderSVGResourceMode::ApplyToStroke, nullptr,
                                              this);
        }
      }
    }
  }

  void LegacyRenderSVGShape::StrokeShape(const RenderStyle &style, GraphicsContext &context) noexcept
  {
    if (!style.HasStroke() || !style.GetStrokeWidth().IsPossiblyPositive())
    {
      return;
    }

    GraphicsContextStateSaver stateSaver(context, false);
    if (HasNonScalingStroke())
    {
      AffineTransform nonScalingTransform = NonScalingStrokeTransform();
      if (!SetupNonScalingStrokeContext(nonScalingTransform, stateSaver))
      {
        return;
      }
    }
    StrokeShapeInternal(style, context);
  }

  void LegacyRenderSVGShape::FillStrokeMarkers(PaintInfo &childPaintInfo) noexcept
  {
    for (auto type : Style().PaintOrder())
    {
      switch (type)
      {
        case style::PaintType::Fill:    FillShape(Style(), childPaintInfo.Context()); break;
        case style::PaintType::Stroke:  StrokeShape(Style(), childPaintInfo.Context()); break;
        case style::PaintType::Markers: DrawMarkers(childPaintInfo); break;
      }
    }
  }

  void LegacyRenderSVGShape::Paint(PaintInfo &paintInfo, const LayoutPoint &) noexcept
  {
    if (Style().UsedVisibility() == Visibility::Hidden || IsEmpty())
    {
      return;
    }

    if (paintInfo.Phase == PaintPhase::EventRegion)
    {
      // FIXME: Unite the fill bounding box into the event region context, once there's one.
      return;
    }

    if (paintInfo.Context().PaintingDisabled() || paintInfo.Phase != PaintPhase::Foreground)
    {
      return;
    }

    FloatRect boundingBox = RepaintRectInLocalCoordinates();
    if (!SVGRenderSupport::PaintInfoIntersectsRepaintRect(boundingBox, _localTransform, paintInfo))
    {
      return;
    }

    PaintInfo childPaintInfo(paintInfo);
    GraphicsContextStateSaver stateSaver(childPaintInfo.Context());
    childPaintInfo.ApplyTransform(_localTransform);

    if (childPaintInfo.Phase == PaintPhase::Foreground)
    {
      SVGRenderingContext renderingContext(*this, childPaintInfo);

      if (renderingContext.IsRenderingPrepared())
      {
        if (Style().GetShapeRendering() == ShapeRendering::CrispEdges)
        {
          childPaintInfo.Context().SetShouldAntialias(false);
        }

        _fillRequiresClip = !renderingContext.PathClippingIsEntirelyWithinRendererContents();
        FillStrokeMarkers(childPaintInfo);
        _fillRequiresClip = true;
      }
    }

    if (style::Evaluate<float>(Style().OutlineWidth(), style::ZoomNeeded {}))
    {
      PaintOutline(childPaintInfo, LayoutRect(IntRect(boundingBox)));
    }
  }

  // This method is called from inside paintOutline() since we call paintOutline()
  // while transformed to our coord system, return local coords
  void LegacyRenderSVGShape::AddFocusRingRects(List<LayoutRect> &rects, const LayoutPoint &,
                                               const RenderLayerModelObject *) const noexcept
  {
    LayoutRect rect = LayoutRect(RepaintRectInLocalCoordinates());
    if (!rect.IsEmpty())
    {
      rects.push_back(rect);
    }
  }

  bool LegacyRenderSVGShape::IsPointInFill(const FloatPoint &point) noexcept
  {
    return ShapeDependentFillContains(point, Style().GetFillRule());
  }

  bool LegacyRenderSVGShape::IsPointInStroke(const FloatPoint &point) noexcept
  {
    if (!Style().HasStroke())
    {
      return false;
    }

    return ShapeDependentStrokeContains(point, LocalCoordinateSpace);
  }

  float LegacyRenderSVGShape::GetTotalLength() const noexcept
  {
    return HasPath() ? GetPath().Length() : CreatePath()->Length();
  }

  FloatPoint LegacyRenderSVGShape::GetPointAtLength(float distance) const noexcept
  {
    return HasPath() ? GetPath().PointAtLength(distance) : CreatePath()->PointAtLength(distance);
  }

  bool LegacyRenderSVGShape::NodeAtFloatPoint(const HitTestRequest &request, HitTestResult &result,
                                              const FloatPoint &pointInParent,
                                              HitTestAction hitTestAction) noexcept
  {
    // We only draw in the forground phase, so we only hit-test then.
    if (hitTestAction != HitTestForeground)
    {
      return false;
    }

    static SVGVisitedRendererTracking::VisitedSet visitedSet;

    SVGVisitedRendererTracking recursionTracking(visitedSet);
    if (recursionTracking.IsVisiting(*this))
    {
      return false;
    }

    FloatPoint localPoint = _localTransform.Inverse().value_or(AffineTransform {}).MapPoint(pointInParent);

    if (!SVGRenderSupport::PointInClippingArea(*this, localPoint))
    {
      return false;
    }

    SVGVisitedRendererTracking::Scope recursionScope(recursionTracking, *this);

    PointerEventsHitRules hitRules(PointerEventsHitRules::HitTestingTargetType::SVGPath, request,
                                   Style().UsedPointerEvents());
    if (VisibleToHitTesting(&request) || !hitRules.RequireVisible)
    {
      WindRule fillRule = Style().GetFillRule();
      if (request.SVGClipContent())
      {
        fillRule = Style().ClipRule();
      }
      if ((hitRules.CanHitStroke && (Style().HasStroke() || !hitRules.RequireStroke)
           && StrokeContains(localPoint, hitRules.RequireStroke))
          || (hitRules.CanHitFill && (Style().HasFill() || !hitRules.RequireFill)
              && FillContains(localPoint, hitRules.RequireFill, fillRule))
          || (hitRules.CanHitBoundingBox && ObjectBoundingBox().Contains(localPoint)))
      {
        UpdateHitTestResult(result, LayoutPoint(localPoint));
        if (result.AddNodeToListBasedTestResult(NodeForHitTest(), request,
                                                HitTestLocation(FlooredLayoutPoint(localPoint)))
            == HitTestProgress::Stop)
        {
          return true;
        }
      }
    }
    return false;
  }

  FloatRect LegacyRenderSVGShape::StrokeBoundingBox() const noexcept
  {
    if (_shapeType == ShapeType::Empty)
    {
      return {};
    }
    if (!_strokeBoundingBox)
    {
      // Initialize m_strokeBoundingBox before calling calculateStrokeBoundingBox, since recursively
      // referenced markers can cause us to re-enter here.
      _strokeBoundingBox = FloatRect {};
      _strokeBoundingBox = CalculateStrokeBoundingBox();
    }
    return *_strokeBoundingBox;
  }

  FloatRect LegacyRenderSVGShape::CalculateStrokeBoundingBox() const noexcept
  {
    krys_debug_assert(_path);
    FloatRect strokeBoundingBox = _fillBoundingBox;

    if (Style().HasStroke())
    {
      auto strokeStyleApplier = [this](GraphicsContext &context)
      {
        SVGRenderSupport::ApplyStrokeStyleToContext(context, Style(), *this);
      };

      if (HasNonScalingStroke())
      {
        AffineTransform nonScalingTransform = NonScalingStrokeTransform();
        if (Maybe<AffineTransform> inverse = nonScalingTransform.Inverse())
        {
          Path *usePath = NonScalingStrokePath(_path.get(), nonScalingTransform);
          FloatRect strokeBoundingRect = usePath->StrokeBoundingRect(strokeStyleApplier);
          strokeBoundingRect = inverse.value().MapRect(strokeBoundingRect);
          if (!strokeBoundingRect.IsNaN())
          {
            strokeBoundingBox.Unite(strokeBoundingRect);
          }
        }
      }
      else
      {
        auto strokeBoundingRect = GetPath().StrokeBoundingRect(strokeStyleApplier);
        if (!strokeBoundingRect.IsNaN())
        {
          strokeBoundingBox.Unite(strokeBoundingRect);
        }
      }
    }

    return AdjustStrokeBoundingBoxForMarkersAndZeroLengthLinecaps(RepaintRectCalculation::Accurate,
                                                                  strokeBoundingBox);
  }

  FloatRect LegacyRenderSVGShape::ApproximateStrokeBoundingBox() const noexcept
  {
    if (_shapeType == ShapeType::Empty)
    {
      return {};
    }
    if (!_approximateStrokeBoundingBox)
    {
      // Initialize m_approximateStrokeBoundingBox before calling calculateApproximateStrokeBoundingBox, since
      // recursively referenced markers can cause us to re-enter here.
      _approximateStrokeBoundingBox = FloatRect {};
      _approximateStrokeBoundingBox = CalculateApproximateStrokeBoundingBox();
    }
    return *_approximateStrokeBoundingBox;
  }

  FloatRect LegacyRenderSVGShape::CalculateApproximateStrokeBoundingBox() const noexcept
  {
    if (_shapeType == ShapeType::Empty)
    {
      return {};
    }

    if (_strokeBoundingBox)
    {
      return *_strokeBoundingBox;
    }

    return SVGRenderSupport::CalculateApproximateStrokeBoundingBox(*this);
  }

  void LegacyRenderSVGShape::UpdateRepaintBoundingBox() noexcept
  {
    _repaintBoundingBox = ApproximateStrokeBoundingBox();
    SVGRenderSupport::IntersectRepaintRectWithResources(*this, _repaintBoundingBox);
  }

  FloatRect LegacyRenderSVGShape::RepaintRectInLocalCoordinates(
    RepaintRectCalculation repaintRectCalculation) const noexcept
  {
    if (repaintRectCalculation == RepaintRectCalculation::Fast)
    {
      return _repaintBoundingBox;
    }

    FloatRect strokeBoundingBox = StrokeBoundingBox();
    SVGRenderSupport::IntersectRepaintRectWithResources(*this, strokeBoundingBox,
                                                        RepaintRectCalculation::Accurate);
    return strokeBoundingBox;
  }

  FloatRect LegacyRenderSVGShape::DecoratedBoundingBox() const noexcept
  {
    // FIXME: Compute decorated bounding box directly instead of delegating
    // to accurate repaint rect.
    return RepaintRectInLocalCoordinates(RepaintRectCalculation::Accurate);
  }

  float LegacyRenderSVGShape::StrokeWidth() const noexcept
  {
    svg::SVGLengthContext lengthContext(&GraphicsElement());
    auto strokeWidth = lengthContext.ValueForLength(Style().GetStrokeWidth(), style::ZoomNeeded {});
    return std::isnan(strokeWidth) ? 0 : strokeWidth;
  }

  Path &LegacyRenderSVGShape::EnsurePath() noexcept
  {
    if (!HasPath())
    {
      _path = CreatePath();
    }
    return GetPath();
  }

  UniquePtr<Path> LegacyRenderSVGShape::CreatePath() const noexcept
  {
    return CreateUnique<Path>(PathFromGraphicsElement(GraphicsElement()));
  }
}
