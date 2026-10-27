#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGContainer.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/HitTestRequest.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/LayoutRepainter.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderingContext.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGVisitedRendererTracking.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD bool ShouldSuspendRepaintForChildren(const LegacyRenderSVGContainer &container) noexcept
    {
      // Issuing repaint requires absolute rect which means ancestor tree walk.
      // In cases when a container has many direct children, the overhead of resolving each individual repaint
      // rects is so large that issuing repaint for the container itself ends up being cheaper.
      static constexpr size_t maximumRequiredChildren = 200;
      static constexpr size_t minimumRequiredChildren = 50;

      size_t numberOfChildren = 0;
      for (auto &child : ChildrenOfType<RenderElement>(container))
      {
        ++numberOfChildren;
        if (!child.NeedsLayout() || child.FirstChild())
        {
          return false;
        }
        if (numberOfChildren >= maximumRequiredChildren)
        {
          return true;
        }
      }
      return numberOfChildren >= minimumRequiredChildren;
    }
  }

  LegacyRenderSVGContainer::LegacyRenderSVGContainer(Type type, svg::SVGElement &element, RenderStyle &&style,
                                                     EnumSet<SVGModelObjectFlag> svgFlags) noexcept
      : LegacyRenderSVGModelObject(type, element, krys::move(style),
                                   svgFlags | SVGModelObjectFlag::IsContainer
                                     | SVGModelObjectFlag::UsesBoundaryCaching)
  {
  }

  LegacyRenderSVGContainer::~LegacyRenderSVGContainer() noexcept = default;

  void LegacyRenderSVGContainer::Layout() noexcept
  {
    krys_debug_assert(NeedsLayout());

    // LegacyRenderSVGRoot disables paint offset cache for the SVG rendering tree.
    krys_debug_assert(!View().FrameView().LayoutContext().IsPaintOffsetCacheEnabled());
    SetForScope repaintIsSuspendedForChildrenDuringLayoutScope {_repaintIsSuspendedForChildrenDuringLayout,
                                                                ShouldSuspendRepaintForChildren(*this)};

    auto checkForRepaintOverride = _repaintIsSuspendedForChildrenDuringLayout || SelfWillPaint()
                                     ? LayoutRepainter::CheckForRepaint::Yes
                                     : SVGRenderSupport::CheckForSVGRepaintDuringLayout(*this);
    auto shouldIssueFullRepaint = _repaintIsSuspendedForChildrenDuringLayout
                                    ? LayoutRepainter::ShouldAlwaysIssueFullRepaint::Yes
                                    : LayoutRepainter::ShouldAlwaysIssueFullRepaint::No;
    LayoutRepainter repainter(*this, checkForRepaintOverride, shouldIssueFullRepaint,
                              RepaintOutlineBounds::No);

    // Allow LegacyRenderSVGViewportContainer to update its viewport.
    CalcViewport();

    // Allow LegacyRenderSVGTransformableContainer to update its transform.
    bool updatedTransform = CalculateLocalTransform();

    // LegacyRenderSVGViewportContainer needs to set the 'layout size changed' flag.
    DetermineIfLayoutSizeChanged();

    SVGRenderSupport::LayoutChildren(*this, SelfNeedsLayout()
                                              || SVGRenderSupport::FiltersForceContainerLayout(*this));

    // Invalidate all resources of this client if our layout changed.
    // FIXME: SVGResourcesCache::clientLayoutChanged, with resources (B11e).

    // At this point LayoutRepainter already grabbed the old bounds,
    // recalculate them now so repaintAfterLayout() uses the new bounds.
    if (_needsBoundariesUpdate || updatedTransform)
    {
      UpdateCachedBoundaries();
      _needsBoundariesUpdate = false;

      // New bounds can affect transforms, so recompute them here if needed.
      CalculateLocalTransform();

      if (auto *parent = Parent())
      {
        parent->InvalidateCachedBoundaries();
      }
    }

    repainter.RepaintAfterLayout();
    ClearNeedsLayout();
  }

  bool LegacyRenderSVGContainer::SelfWillPaint() noexcept
  {
    // FIXME: Whether the container has a filter resource (SVGResourcesCache), with filters (B11h).
    return false;
  }

  void LegacyRenderSVGContainer::Paint(PaintInfo &paintInfo, const LayoutPoint &) noexcept
  {
    if (paintInfo.Phase != PaintPhase::EventRegion && paintInfo.Context().PaintingDisabled())
    {
      return;
    }

    // Spec: groups w/o children still may render filter content.
    if (!FirstChild() && !SelfWillPaint())
    {
      return;
    }

    FloatRect repaintRect = RepaintRectInLocalCoordinates();
    if (!SVGRenderSupport::PaintInfoIntersectsRepaintRect(repaintRect, LocalToParentTransform(), paintInfo))
    {
      return;
    }

    PaintInfo childPaintInfo(paintInfo);
    {
      GraphicsContextStateSaver stateSaver(childPaintInfo.Context());

      // Let the LegacyRenderSVGViewportContainer subclass clip if necessary
      ApplyViewportClip(childPaintInfo);

      auto transform = LocalToParentTransform();
      childPaintInfo.ApplyTransform(transform);
      // FIXME: Push the transform onto the event region context, once there's one.

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
          child.Paint(childPaintInfo, IntPoint());
        }
      }
    }

    // FIXME: This really should be drawn from local coordinates, but currently we hack it
    // to avoid our clip killing our outline rect. Thus we translate our
    // outline rect into parent coords before drawing.
    // FIXME: This means our focus ring won't share our rotation like it should.
    // We should instead disable our clip during PaintPhase::Outline
    if (paintInfo.Phase == PaintPhase::SelfOutline
        && style::Evaluate<float>(Style().OutlineWidth(), style::ZoomNeeded {})
        && Style().UsedVisibility() == Visibility::Visible)
    {
      IntRect paintRectInParent = EnclosingIntRect(LocalToParentTransform().MapRect(repaintRect));
      PaintOutline(paintInfo, paintRectInParent);
    }
  }

  // addFocusRingRects is called from paintOutline and needs to be in the same coordinates as the paintOuline
  // call
  void LegacyRenderSVGContainer::AddFocusRingRects(List<LayoutRect> &rects, const LayoutPoint &,
                                                   const RenderLayerModelObject *) const noexcept
  {
    LayoutRect paintRectInParent =
      LayoutRect(LocalToParentTransform().MapRect(RepaintRectInLocalCoordinates()));
    if (!paintRectInParent.IsEmpty())
    {
      rects.push_back(paintRectInParent);
    }
  }

  void LegacyRenderSVGContainer::UpdateCachedBoundaries() noexcept
  {
    _strokeBoundingBox = null;
    _repaintBoundingBox = {};
    _accurateRepaintBoundingBox = null;

    auto boundingBoxes = SVGRenderSupport::ComputeContainerBoundingBoxes(*this);
    _objectBoundingBox = boundingBoxes.ObjectBoundingBox;

    SVGRenderSupport::IntersectRepaintRectWithResources(*this, boundingBoxes.RepaintBoundingBox);
    _repaintBoundingBox = boundingBoxes.RepaintBoundingBox;
  }

  FloatRect LegacyRenderSVGContainer::StrokeBoundingBox() const noexcept
  {
    if (!_strokeBoundingBox)
    {
      // Initialize m_strokeBoundingBox before calling computeContainerStrokeBoundingBox, since recursively
      // referenced markers can cause us to re-enter here.
      _strokeBoundingBox = FloatRect {};
      _strokeBoundingBox = SVGRenderSupport::ComputeContainerStrokeBoundingBox(*this);
    }
    return *_strokeBoundingBox;
  }

  FloatRect LegacyRenderSVGContainer::RepaintRectInLocalCoordinates(
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
      _accurateRepaintBoundingBox = boundingBoxes.RepaintBoundingBox;
    }
    return *_accurateRepaintBoundingBox;
  }

  FloatRect LegacyRenderSVGContainer::DecoratedBoundingBox() const noexcept
  {
    // FIXME: Compute decorated bounding box directly instead of delegating
    // to accurate repaint rect.
    return RepaintRectInLocalCoordinates(RepaintRectCalculation::Accurate);
  }

  bool LegacyRenderSVGContainer::NodeAtFloatPoint(const HitTestRequest &request, HitTestResult &result,
                                                  const FloatPoint &pointInParent,
                                                  HitTestAction hitTestAction) noexcept
  {
    // Give LegacyRenderSVGViewportContainer a chance to apply its viewport clip
    if (!PointIsInsideViewportClip(pointInParent))
    {
      return false;
    }

    static SVGVisitedRendererTracking::VisitedSet visitedSet;

    SVGVisitedRendererTracking recursionTracking(visitedSet);
    if (recursionTracking.IsVisiting(*this))
    {
      return false;
    }

    SVGVisitedRendererTracking::Scope recursionScope(recursionTracking, *this);

    FloatPoint localPoint =
      LocalToParentTransform().Inverse().value_or(AffineTransform {}).MapPoint(pointInParent);
    if (!SVGRenderSupport::PointInClippingArea(*this, localPoint))
    {
      return false;
    }

    for (RenderObject *child = LastChild(); child; child = child->PreviousSibling())
    {
      if (child->NodeAtFloatPoint(request, result, localPoint, hitTestAction))
      {
        UpdateHitTestResult(result, LayoutPoint(localPoint));
        if (result.AddNodeToListBasedTestResult(child->GetNode(), request,
                                                HitTestLocation(FlooredLayoutPoint(localPoint)))
            == HitTestProgress::Stop)
        {
          return true;
        }
      }
    }

    // Accessibility wants to return SVG containers, if appropriate.
    if (request.GetType().Contains(HitTestRequest::Type::AccessibilityHitTest)
        && ObjectBoundingBox().Contains(localPoint))
    {
      UpdateHitTestResult(result, LayoutPoint(localPoint));
      if (result.AddNodeToListBasedTestResult(NodeForHitTest(), request,
                                              HitTestLocation(FlooredLayoutPoint(localPoint)))
          == HitTestProgress::Stop)
      {
        return true;
      }
    }

    // pointer-events=bounding-box makes it possible for containers to be direct targets.
    if (Style().GetPointerEvents() == PointerEvents::BoundingBox)
    {
      if (!IsObjectBoundingBoxValid())
      {
        return false;
      }
      if (ObjectBoundingBox().Contains(localPoint))
      {
        UpdateHitTestResult(result, LayoutPoint(localPoint));
        return true;
      }
    }
    // 16.4: "If there are no graphics elements whose relevant graphics content is under the pointer (i.e.,
    // there is no target element), the event is not dispatched."
    return false;
  }
}
