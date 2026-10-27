#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGRoot.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/HitTestLocation.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/LayoutRepainter.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderLayoutState.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderingContext.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Booey/SVG/Graphics/SVGImage.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo
{
  namespace
  {
    constexpr int DefaultWidth = 300;
    constexpr int DefaultHeight = 150;
  }

  LegacyRenderSVGRoot::LegacyRenderSVGRoot(svg::SVGSvgElement &element, RenderStyle &&style) noexcept
      : RenderReplaced(Type::LegacySVGRoot, element, krys::move(style), ReplacedFlag::UsesBoundaryCaching)
  {
    krys_debug_assert(IsLegacyRenderSVGRoot());
    LayoutSize intrinsicSize(ComputeIntrinsicSize());
    if (!intrinsicSize.Width())
    {
      intrinsicSize.SetWidth(DefaultWidth);
    }
    if (!intrinsicSize.Height())
    {
      intrinsicSize.SetHeight(DefaultHeight);
    }
    SetIntrinsicSize(intrinsicSize);
  }

  LegacyRenderSVGRoot::~LegacyRenderSVGRoot() noexcept = default;

  svg::SVGSvgElement &LegacyRenderSVGRoot::SvgSvgElement() const noexcept
  {
    return Downcast<svg::SVGSvgElement>(Downcast<svg::SVGElement>(NodeForNonAnonymous()));
  }

  bool LegacyRenderSVGRoot::HasIntrinsicAspectRatio() const noexcept
  {
    return ComputeIntrinsicAspectRatio();
  }

  FloatSize LegacyRenderSVGRoot::ComputeIntrinsicSize() const noexcept
  {
    FloatSize intrinsicSize = {SvgSvgElement().IntrinsicWidth(), SvgSvgElement().IntrinsicHeight()};
    // Transpose for vertical writing mode
    if (!IsHorizontalWritingMode())
    {
      return intrinsicSize.TransposedSize();
    }
    return intrinsicSize;
  }

  FloatSize LegacyRenderSVGRoot::PreferredAspectRatio() const noexcept
  {
    krys_debug_assert(!ShouldApplySizeContainment());

    if (Style().GetAspectRatio().IsRatio())
    {
      return FloatSize::NarrowPrecision(Style().AspectRatioLogicalWidth().Value,
                                        Style().AspectRatioLogicalHeight().Value);
    }

    auto intrinsicSize = ComputeIntrinsicSize();
    Maybe<FloatSize> intrinsicRatioValue;
    FloatSize preferredAspectRatio;
    if (!intrinsicSize.IsEmpty())
    {
      preferredAspectRatio = {intrinsicSize.Width(), intrinsicSize.Height()};
    }
    else
    {
      FloatSize viewBoxSize = SvgSvgElement().CurrentViewBoxRect().Size();
      if (!viewBoxSize.IsEmpty())
      {
        // The viewBox can only yield an intrinsic ratio, not an intrinsic size.
        if (IsHorizontalWritingMode())
        {
          intrinsicRatioValue = FloatSize {viewBoxSize.Width(), viewBoxSize.Height()};
        }
        else
        {
          intrinsicRatioValue = FloatSize {viewBoxSize.Height(), viewBoxSize.Width()};
        }
      }
    }

    if (intrinsicRatioValue)
    {
      return *intrinsicRatioValue;
    }
    if (Style().GetAspectRatio().IsAutoAndRatio())
    {
      return FloatSize::NarrowPrecision(Style().AspectRatioLogicalWidth().Value,
                                        Style().AspectRatioLogicalHeight().Value);
    }

    return preferredAspectRatio;
  }

  bool LegacyRenderSVGRoot::IsEmbeddedThroughSVGImage() const noexcept
  {
    return svg::IsInSVGImage(&SvgSvgElement());
  }

  LayoutUnit LegacyRenderSVGRoot::ComputeReplacedLogicalWidth(
    ShouldComputePreferred shouldComputePreferred) const noexcept
  {
    // When we're embedded through SVGImage (border-image/background-image/<html:img>/...) we're forced to
    // resize to a specific size.
    if (!_containerSize.IsEmpty())
    {
      return LayoutUnit(_containerSize.Width());
    }

    // SVG embedded via SVGImage (background-image/border-image/etc) / Inline SVG.
    return RenderReplaced::ComputeReplacedLogicalWidth(shouldComputePreferred);
  }

  LayoutUnit
    LegacyRenderSVGRoot::ComputeReplacedLogicalHeight(Maybe<LayoutUnit> estimatedUsedWidth) const noexcept
  {
    // When we're embedded through SVGImage (border-image/background-image/<html:img>/...) we're forced to
    // resize to a specific size.
    if (!_containerSize.IsEmpty())
    {
      return LayoutUnit(_containerSize.Height());
    }

    // SVG embedded via SVGImage (background-image/border-image/etc) / Inline SVG.
    return RenderReplaced::ComputeReplacedLogicalHeight(estimatedUsedWidth);
  }

  void LegacyRenderSVGRoot::Layout() noexcept
  {
    SetForScope change(_inLayout, true);
    krys_debug_assert(NeedsLayout());

    _resourcesNeedingToInvalidateClients.clear();

    // Arbitrary affine transforms are incompatible with RenderLayoutState.
    LayoutStateDisabler layoutStateDisabler(View().FrameView().LayoutContext());

    bool needsLayout = SelfNeedsLayout();
    auto checkForRepaintOverride =
      !needsLayout ? Maybe<LayoutRepainter::CheckForRepaint>(LayoutRepainter::CheckForRepaint::No) : null;
    LayoutRepainter repainter(*this, checkForRepaintOverride);

    LayoutSize oldSize = Size();
    UpdateLogicalWidth();
    UpdateLogicalHeight();
    BuildLocalToBorderBoxTransform();

    _isLayoutSizeChanged = needsLayout || (SvgSvgElement().HasRelativeLengths() && oldSize != Size());
    SVGRenderSupport::LayoutChildren(*this,
                                     needsLayout || SVGRenderSupport::FiltersForceContainerLayout(*this));

    std::erase_if(_resourcesNeedingToInvalidateClients,
                  [](const WeakPtr<LegacyRenderSVGResourceContainer> &resource) { return !resource; });
    if (!_resourcesNeedingToInvalidateClients.empty())
    {
      // Invalidate resource clients, which may mark some nodes for layout.
      auto resources = _resourcesNeedingToInvalidateClients;
      for (auto &resource : resources)
      {
        if (!resource)
        {
          continue;
        }
        resource->RemoveAllClientsFromCacheAndMarkForInvalidation();
        SVGResourcesCache::ClientStyleChanged(*resource, style::DifferenceResult::Layout, nullptr,
                                              resource->Style());
      }

      _isLayoutSizeChanged = false;
      SVGRenderSupport::LayoutChildren(*this, false);
    }

    // At this point LayoutRepainter already grabbed the old bounds,
    // recalculate them now so repaintAfterLayout() uses the new bounds.
    if (_needsBoundariesOrTransformUpdate)
    {
      UpdateCachedBoundaries();
      _needsBoundariesOrTransformUpdate = false;
    }

    ClearOverflow();
    if (!ShouldApplyViewportClip())
    {
      AddVisualOverflow(ComputeContentsInkOverflow());
    }

    UpdateLayerTransform();
    _hasBoxDecorations =
      IsDocumentElementRenderer() ? HasVisibleBoxDecorationStyle() : HasVisibleBoxDecorations();
    InvalidateBackgroundObscurationStatus();

    repainter.RepaintAfterLayout();

    ClearNeedsLayout();
  }

  LayoutRect LegacyRenderSVGRoot::ComputeContentsInkOverflow() const noexcept
  {
    FloatRect contentRepaintRect = RepaintRectInLocalCoordinates();
    contentRepaintRect = _localToBorderBoxTransform.MapRect(contentRepaintRect);
    // Condition the visual overflow rect to avoid being clipped/culled
    // out if it is huge. This may sacrifice overflow, but usually only
    // overflow that would never be seen anyway.
    // To condition, we intersect with something that we oftentimes
    // consider to be "infinity".
    return Intersection(EnclosingLayoutRect(contentRepaintRect), LayoutRect::InfiniteRect());
  }

  bool LegacyRenderSVGRoot::ShouldApplyViewportClip() const noexcept
  {
    // the outermost svg is clipped if auto, and svg document roots are always clipped
    // When the svg is stand-alone (isDocumentElement() == true) the viewport clipping should always
    // be applied, noting that the window scrollbars should be hidden if overflow=hidden.
    return IsNonVisibleOverflow(EffectiveOverflowX()) || Style().OverflowX() == Overflow::Auto
           || this->IsDocumentElementRenderer();
  }

  void LegacyRenderSVGRoot::PaintReplaced(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    krys_debug_assert(!IsSkippedContentRoot(*this));

    // An empty viewport disables rendering.
    bool clipViewport = ShouldApplyViewportClip();
    if (clipViewport && ContentBoxSize().IsEmpty())
    {
      return;
    }

    // Don't paint, if the context explicitly disabled it.
    if (paintInfo.Phase != PaintPhase::EventRegion && paintInfo.Context().PaintingDisabled()
        && !paintInfo.Context().DetectingContentfulPaint())
    {
      return;
    }

    // SVG outlines are painted during PaintPhase::Foreground.
    if (paintInfo.Phase == PaintPhase::Outline || paintInfo.Phase == PaintPhase::SelfOutline)
    {
      return;
    }

    // An empty viewBox also disables rendering.
    // (http://www.w3.org/TR/SVG/coords.html#ViewBoxAttribute)
    if (SvgSvgElement().HasEmptyViewBox())
    {
      return;
    }

    GraphicsContext &context = paintInfo.Context();
    if (context.DetectingContentfulPaint())
    {
      for (auto &current : ChildrenOfType<RenderObject>(*this))
      {
        if (!current.IsLegacyRenderSVGHiddenContainer())
        {
          context.SetContentfulPaintDetected();
          return;
        }
      }
      return;
    }

    // Don't paint if we don't have kids, except if we have filters we should paint those.
    // FIXME: Paint a filter without children (SVGResourcesCache), with filters (B11h).
    // FIXME: The page's relevant repainted and unpainted objects come with paint milestones.
    if (!FirstChild())
    {
      return;
    }

    // Make a copy of the PaintInfo because applyTransform will modify the damage rect.
    PaintInfo childPaintInfo(paintInfo);
    childPaintInfo.Context().Save();

    // Apply initial viewport clip
    if (clipViewport)
    {
      auto clipRect = SnappedIntRect(OverflowClipRect(paintOffset));
      childPaintInfo.Context().Clip(clipRect);
      // FIXME: Push the clip onto the event region context, once there's one.
    }

    // Convert from container offsets (html renderers) to a relative transform (svg renderers).
    // Transform from our paint container's coordinate system to our local coords.
    IntPoint adjustedPaintOffset = RoundedIntPoint(paintOffset);
    auto transform =
      AffineTransform::MakeTranslation(ToFloatSize(adjustedPaintOffset)) * LocalToBorderBoxTransform();
    childPaintInfo.ApplyTransform(transform);

    // SVGRenderingContext must be destroyed before we restore the childPaintInfo.context(), because a filter
    // may have changed the context and it is only reverted when the SVGRenderingContext destructor finishes
    // applying the filter.
    {
      SVGRenderingContext renderingContext;
      bool continueRendering = true;
      if (childPaintInfo.Phase == PaintPhase::Foreground)
      {
        renderingContext.PrepareToRenderSVGContent(*this, childPaintInfo);
        continueRendering = renderingContext.IsRenderingPrepared();
      }

      if (continueRendering)
      {
        childPaintInfo.UpdateSubtreePaintRootForChildren(this);
        for (auto &child : ChildrenOfType<RenderElement>(*this))
        {
          child.Paint(childPaintInfo, Location());
        }
      }
    }

    childPaintInfo.Context().Restore();
  }

  void LegacyRenderSVGRoot::WillBeDestroyed() noexcept
  {
    RenderBlock::RemovePercentHeightDescendant(const_cast<LegacyRenderSVGRoot &>(*this));

    SVGResourcesCache::ClientDestroyed(*this);
    RenderReplaced::WillBeDestroyed();
  }

  void LegacyRenderSVGRoot::InsertedIntoTree() noexcept
  {
    RenderReplaced::InsertedIntoTree();
    SVGResourcesCache::ClientWasAddedToTree(*this);
  }

  void LegacyRenderSVGRoot::WillBeRemovedFromTree() noexcept
  {
    SVGResourcesCache::ClientWillBeRemovedFromTree(*this);
    RenderReplaced::WillBeRemovedFromTree();
  }

  void LegacyRenderSVGRoot::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    if (diff == style::DifferenceResult::Layout)
    {
      InvalidateCachedBoundaries();
    }

    // Box decorations may have appeared/disappeared - recompute status.
    if (diff == style::DifferenceResult::Repaint)
    {
      _hasBoxDecorations = HasVisibleBoxDecorationStyle();
    }

    RenderReplaced::StyleDidChange(diff, oldStyle);
    SVGResourcesCache::ClientStyleChanged(*this, diff, oldStyle, Style());
  }

  // RenderBox methods will expect coordinates w/o any transforms in coordinates
  // relative to our borderBox origin. This method gives us exactly that.
  void LegacyRenderSVGRoot::BuildLocalToBorderBoxTransform() noexcept
  {
    float scale = Style().UsedZoom();
    FloatPoint translate = SvgSvgElement().CurrentTranslateValue();
    LayoutSize borderAndPadding(BorderLeft() + PaddingLeft(), BorderTop() + PaddingTop());
    _localToBorderBoxTransform =
      SvgSvgElement().ViewBoxToViewTransform(ContentBoxWidth() / scale, ContentBoxHeight() / scale);
    if (borderAndPadding.IsZero() && scale == 1 && translate == FloatPoint::Zero())
    {
      return;
    }
    _localToBorderBoxTransform = AffineTransform(scale, 0, 0, scale, borderAndPadding.Width() + translate.X(),
                                                 borderAndPadding.Height() + translate.Y())
                                 * _localToBorderBoxTransform;
  }

  const AffineTransform &LegacyRenderSVGRoot::LocalToParentTransform() const noexcept
  {
    // Slightly optimized version of m_localToParentTransform = AffineTransform::makeTranslation(x(), y()) *
    // m_localToBorderBoxTransform;
    _localToParentTransform = _localToBorderBoxTransform;
    if (X())
    {
      _localToParentTransform.SetE(_localToParentTransform.E() + RoundToInt(X()));
    }
    if (Y())
    {
      _localToParentTransform.SetF(_localToParentTransform.F() + RoundToInt(Y()));
    }
    return _localToParentTransform;
  }

  LayoutRect LegacyRenderSVGRoot::LocalClippedOverflowRect(
    RepaintRectCalculation repaintRectCalculation) const noexcept
  {
    auto contentRepaintRect =
      _localToBorderBoxTransform.MapRect(RepaintRectInLocalCoordinates(repaintRectCalculation));
    contentRepaintRect.Intersect(SnappedIntRect(BorderBoxRect()));

    LayoutRect repaintRect = EnclosingLayoutRect(contentRepaintRect);
    if (_hasBoxDecorations || HasRenderOverflow())
    {
      repaintRect.Unite(UnionRect(LocalSelectionRect(false), VisualOverflowRect()));
    }

    return EnclosingIntRect(repaintRect);
  }

  LayoutRect LegacyRenderSVGRoot::ClippedOverflowRect(const RenderLayerModelObject *repaintContainer,
                                                      VisibleRectContext context) const noexcept
  {
    if (IsInsideEntirelyHiddenLayer())
    {
      return {};
    }

    auto rects = RepaintRects {LocalClippedOverflowRect(context.GetRepaintRectCalculation())};
    return RenderReplaced::ComputeRects(rects, repaintContainer, context).ClippedOverflowRect;
  }

  auto LegacyRenderSVGRoot::RectsForRepaintingAfterLayout(
    const RenderLayerModelObject *repaintContainer, RepaintOutlineBounds repaintOutlineBounds) const noexcept
    -> RepaintRects
  {
    if (IsInsideEntirelyHiddenLayer())
    {
      return {};
    }

    auto rects = RepaintRects {LocalClippedOverflowRect(RepaintRectCalculation::Fast)};
    if (repaintOutlineBounds == RepaintOutlineBounds::Yes)
    {
      rects.OutlineBoundsRect = LocalOutlineBoundsRepaintRect();
    }

    return RenderReplaced::ComputeRects(rects, repaintContainer, VisibleRectContextForRepaint());
  }

  Maybe<FloatRect> LegacyRenderSVGRoot::ComputeFloatVisibleRectInContainer(
    const FloatRect &rect, const RenderLayerModelObject *container, VisibleRectContext context) const noexcept
  {
    // Apply our local transforms (except for x/y translation) and then call
    // RenderBox's method to handle all the normal CSS Box model bits
    FloatRect adjustedRect = _localToBorderBoxTransform.MapRect(rect);

    // Apply initial viewport clip
    if (ShouldApplyViewportClip())
    {
      if (context.Options.Contains(VisibleRectContext::Option::UseEdgeInclusiveIntersection))
      {
        if (!adjustedRect.EdgeInclusiveIntersect(FloatRect(SnappedIntRect(BorderBoxRect()))))
        {
          return null;
        }
      }
      else
      {
        adjustedRect.Intersect(FloatRect(SnappedIntRect(BorderBoxRect())));
      }
    }

    if (_hasBoxDecorations || HasRenderOverflow())
    {
      // The selectionRect can project outside of the overflowRect, so take their union
      // for repainting to avoid selection painting glitches.
      LayoutRect decoratedRepaintRect = UnionRect(LocalSelectionRect(false), VisualOverflowRect());
      adjustedRect.Unite(FloatRect(decoratedRepaintRect));
    }

    auto rects = RepaintRects {LayoutRect(EnclosingIntRect(adjustedRect))};
    auto rectsInContainer = RenderReplaced::ComputeVisibleRectsInContainer(rects, container, context);
    if (!rectsInContainer)
    {
      return null;
    }

    return FloatRect(rectsInContainer->ClippedOverflowRect);
  }

  // This method expects local CSS box coordinates.
  // Callers with local SVG viewport coordinates should first apply the localToBorderBoxTransform
  // to convert from SVG viewport coordinates to local CSS box coordinates.
  void LegacyRenderSVGRoot::MapLocalToContainer(const RenderLayerModelObject *ancestorContainer,
                                                TransformState &transformState,
                                                EnumSet<MapCoordinatesMode> mode,
                                                bool *wasFixed) const noexcept
  {
    RenderReplaced::MapLocalToContainer(ancestorContainer, transformState,
                                        mode | MapCoordinatesMode::ApplyContainerFlip, wasFixed);
  }

  const RenderElement *
    LegacyRenderSVGRoot::PushMappingToContainer(const RenderLayerModelObject *ancestorToStopAt,
                                                RenderGeometryMap &geometryMap) const noexcept
  {
    return RenderReplaced::PushMappingToContainer(ancestorToStopAt, geometryMap);
  }

  void LegacyRenderSVGRoot::UpdateCachedBoundaries() noexcept
  {
    _strokeBoundingBox = null;
    _repaintBoundingBox = {};
    _accurateRepaintBoundingBox = null;

    auto boundingBoxes = SVGRenderSupport::ComputeContainerBoundingBoxes(*this);
    _objectBoundingBox = boundingBoxes.ObjectBoundingBox;

    SVGRenderSupport::IntersectRepaintRectWithResources(*this, boundingBoxes.RepaintBoundingBox);
    boundingBoxes.RepaintBoundingBox.Inflate(HorizontalBorderAndPaddingExtent());

    _repaintBoundingBox = boundingBoxes.RepaintBoundingBox;
  }

  FloatRect LegacyRenderSVGRoot::StrokeBoundingBox() const noexcept
  {
    // FIXME: Once we enable approximate repainting bounding box computation, m_strokeBoundingBox becomes
    // std::nullopt in updateCachedBoundaries and gets lazily computed.
    // https://bugs.webkit.org/show_bug.cgi?id=262409
    if (!_strokeBoundingBox)
    {
      // Initialize m_strokeBoundingBox before calling computeContainerStrokeBoundingBox, since recursively
      // referenced markers can cause us to re-enter here.
      _strokeBoundingBox = FloatRect {};
      _strokeBoundingBox = SVGRenderSupport::ComputeContainerStrokeBoundingBox(*this);
    }
    return *_strokeBoundingBox;
  }

  FloatRect LegacyRenderSVGRoot::RepaintRectInLocalCoordinates(
    RepaintRectCalculation repaintRectCalculation) const noexcept
  {
    if (repaintRectCalculation == RepaintRectCalculation::Fast)
    {
      return _repaintBoundingBox;
    }

    if (!_accurateRepaintBoundingBox)
    {
      // Initialize m_accurateRepaintBoundingBox before calling computeContainerBoundingBoxes, since
      // recursively referenced markers can cause us to re-enter here.
      _accurateRepaintBoundingBox = FloatRect {};

      auto boundingBoxes =
        SVGRenderSupport::ComputeContainerBoundingBoxes(*this, RepaintRectCalculation::Accurate);
      SVGRenderSupport::IntersectRepaintRectWithResources(*this, boundingBoxes.RepaintBoundingBox,
                                                          RepaintRectCalculation::Accurate);
      boundingBoxes.RepaintBoundingBox.Inflate(HorizontalBorderAndPaddingExtent());

      _accurateRepaintBoundingBox = boundingBoxes.RepaintBoundingBox;
    }
    return *_accurateRepaintBoundingBox;
  }

  FloatRect LegacyRenderSVGRoot::DecoratedBoundingBox() const noexcept
  {
    // FIXME: Compute decorated bounding box directly instead of delegating
    // to accurate repaint rect.
    return RepaintRectInLocalCoordinates(RepaintRectCalculation::Accurate);
  }

  bool LegacyRenderSVGRoot::NodeAtPoint(const HitTestRequest &request, HitTestResult &result,
                                        const HitTestLocation &locationInContainer,
                                        const LayoutPoint &accumulatedOffset,
                                        HitTestAction hitTestAction) noexcept
  {
    LayoutPoint pointInParent = locationInContainer.Point() - ToLayoutSize(accumulatedOffset);
    LayoutPoint pointInBorderBox = pointInParent - ToLayoutSize(Location());

    // Test SVG content if the point is in our content box or it is inside the visualOverflowRect and the
    // overflow is visible.
    // FIXME: This should be an intersection when rect-based hit tests are supported by nodeAtFloatPoint.
    if (ContentBoxRect().Contains(pointInBorderBox)
        || (!ShouldApplyViewportClip() && VisualOverflowRect().Contains(pointInParent)))
    {
      FloatPoint localPoint =
        LocalToParentTransform().Inverse().value_or(AffineTransform {}).MapPoint(FloatPoint(pointInParent));

      for (RenderObject *child = LastChild(); child; child = child->PreviousSibling())
      {
        // FIXME: nodeAtFloatPoint() doesn't handle rect-based hit tests yet.
        if (child->NodeAtFloatPoint(request, result, localPoint, hitTestAction))
        {
          UpdateHitTestResult(result, pointInBorderBox);
          if (result.AddNodeToListBasedTestResult(child->GetNode(), request, locationInContainer)
              == HitTestProgress::Stop)
          {
            return true;
          }
        }
      }
    }

    // If we didn't early exit above, we've just hit the container <svg> element. Unlike SVG 1.1, 2nd Edition
    // allows container elements to be hit.
    if ((hitTestAction == HitTestBlockBackground || hitTestAction == HitTestChildBlockBackground)
        && VisibleToHitTesting(&request))
    {
      // Only return true here, if the last hit testing phase 'BlockBackground' is executed. If we'd return
      // true in the 'Foreground' phase, hit testing would stop immediately. For SVG only trees this doesn't
      // matter. Though when we have a <foreignObject> subtree we need to be able to detect hits on the
      // background of a <div> element. If we'd return true here in the 'Foreground' phase, we are not able to
      // detect these hits anymore.
      LayoutRect boundsRect(accumulatedOffset + Location(), Size());
      if (locationInContainer.Intersects(boundsRect))
      {
        UpdateHitTestResult(result, pointInBorderBox);
        if (result.AddNodeToListBasedTestResult(NodeForHitTest(), request, locationInContainer, boundsRect)
            == HitTestProgress::Stop)
        {
          return true;
        }
      }
    }

    return false;
  }

  bool LegacyRenderSVGRoot::HasRelativeDimensions() const noexcept
  {
    return false;
  }

  void
    LegacyRenderSVGRoot::AddResourceForClientInvalidation(LegacyRenderSVGResourceContainer *resource) noexcept
  {
    auto *svgRoot = SVGRenderSupport::FindTreeRootObject(*resource);
    if (!svgRoot)
    {
      return;
    }
    auto &resources = svgRoot->_resourcesNeedingToInvalidateClients;
    if (std::ranges::none_of(resources, [&](const WeakPtr<LegacyRenderSVGResourceContainer> &weakResource)
                             { return weakResource.get() == resource; }))
    {
      resources.push_back(CreateWeakPtr(resource));
    }
  }
}
