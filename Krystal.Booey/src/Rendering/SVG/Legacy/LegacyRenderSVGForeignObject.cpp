#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGForeignObject.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/HitTestLocation.hpp"
#include "Krystal.Booey/Rendering/LayoutRepainter.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderingContext.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Booey/SVG/SVGForeignObjectElement.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"

namespace krys::boo
{
  LegacyRenderSVGForeignObject::LegacyRenderSVGForeignObject(svg::SVGForeignObjectElement &element,
                                                             RenderStyle &&style) noexcept
      : RenderSVGBlock(Type::LegacySVGForeignObject, element, krys::move(style))
  {
    krys_debug_assert(IsLegacyRenderSVGForeignObject());
  }

  LegacyRenderSVGForeignObject::~LegacyRenderSVGForeignObject() noexcept = default;

  svg::SVGForeignObjectElement &LegacyRenderSVGForeignObject::ForeignObjectElement() const noexcept
  {
    return Downcast<svg::SVGForeignObjectElement>(RenderSVGBlock::GraphicsElement());
  }

  void LegacyRenderSVGForeignObject::Paint(PaintInfo &paintInfo, const LayoutPoint &) noexcept
  {
    if (paintInfo.Context().PaintingDisabled())
    {
      return;
    }

    if (paintInfo.Phase != PaintPhase::Foreground && paintInfo.Phase != PaintPhase::Selection)
    {
      return;
    }

    PaintInfo childPaintInfo(paintInfo);
    GraphicsContextStateSaver stateSaver(childPaintInfo.Context());
    childPaintInfo.ApplyTransform(LocalTransform());
    // NOTE: painted from the block's location, where the repaint rect (which opacity, blending and isolation
    // clip to) starts; the block, which paints itself at its location, is painted offset back by it.
    childPaintInfo.ApplyTransform(AffineTransform::MakeTranslation(FloatSize(LocationOffset())));

    if (SVGRenderSupport::IsOverflowHidden(*this))
    {
      childPaintInfo.Context().Clip(FloatRect(FloatPoint(), _viewport.Size()));
    }

    SVGRenderingContext renderingContext;
    if (paintInfo.Phase == PaintPhase::Foreground)
    {
      renderingContext.PrepareToRenderSVGContent(*this, childPaintInfo);
      if (!renderingContext.IsRenderingPrepared())
      {
        return;
      }
    }

    LayoutPoint childPoint(-Location().X(), -Location().Y());
    if (paintInfo.Phase == PaintPhase::Selection)
    {
      RenderBlock::Paint(childPaintInfo, childPoint);
      return;
    }

    // Paint all phases of FO elements atomically, as though the FO element established its
    // own stacking context.
    childPaintInfo.Phase = PaintPhase::BlockBackground;
    RenderBlock::Paint(childPaintInfo, childPoint);
    childPaintInfo.Phase = PaintPhase::ChildBlockBackgrounds;
    RenderBlock::Paint(childPaintInfo, childPoint);
    childPaintInfo.Phase = PaintPhase::Float;
    RenderBlock::Paint(childPaintInfo, childPoint);
    childPaintInfo.Phase = PaintPhase::Foreground;
    RenderBlock::Paint(childPaintInfo, childPoint);
    childPaintInfo.Phase = PaintPhase::Outline;
    RenderBlock::Paint(childPaintInfo, childPoint);
  }

  const AffineTransform &LegacyRenderSVGForeignObject::LocalToParentTransform() const noexcept
  {
    _localToParentTransform = LocalTransform();
    _localToParentTransform.Translate(_viewport.Location());
    return _localToParentTransform;
  }

  void LegacyRenderSVGForeignObject::UpdateLogicalWidth() noexcept
  {
    auto logicalWidth = Style().GetWritingMode().IsHorizontal() ? _viewport.Width() : _viewport.Height();
    SetLogicalWidth(LayoutUnit {logicalWidth * Style().UsedZoom()});
  }

  auto LegacyRenderSVGForeignObject::ComputeLogicalHeight(LayoutUnit, LayoutUnit logicalTop) const noexcept
    -> LogicalExtentComputedValues
  {
    auto logicalHeight = Style().GetWritingMode().IsHorizontal() ? _viewport.Height() : _viewport.Width();
    return {LayoutUnit {logicalHeight * Style().UsedZoom()}, logicalTop, ComputedMarginValues()};
  }

  void LegacyRenderSVGForeignObject::Layout() noexcept
  {
    krys_debug_assert(NeedsLayout());
    // LegacyRenderSVGRoot disables paint offset cache for the SVG rendering tree.
    krys_debug_assert(!View().FrameView().LayoutContext().IsPaintOffsetCacheEnabled());

    LayoutRepainter repainter(*this, SVGRenderSupport::CheckForSVGRepaintDuringLayout(*this));

    bool updateCachedBoundariesInParents = false;
    if (_needsTransformUpdate)
    {
      _localTransform = ForeignObjectElement().AnimatedLocalTransform();
      _needsTransformUpdate = false;
      updateCachedBoundariesInParents = true;
    }

    FloatRect oldViewport = _viewport;

    // Cache viewport boundaries
    auto &foreignObjectElement = ForeignObjectElement();
    svg::SVGLengthContext lengthContext(&foreignObjectElement);
    FloatPoint viewportLocation(foreignObjectElement.X().Value(lengthContext),
                                foreignObjectElement.Y().Value(lengthContext));
    _viewport = FloatRect(viewportLocation, FloatSize(foreignObjectElement.Width().Value(lengthContext),
                                                      foreignObjectElement.Height().Value(lengthContext)));
    if (!updateCachedBoundariesInParents)
    {
      updateCachedBoundariesInParents = oldViewport != _viewport;
    }

    // Set box origin to the foreignObject x/y translation, so positioned objects in XHTML content get correct
    // positions. A regular RenderBoxModelObject would pull this information from RenderStyle - in SVG those
    // properties are ignored for non <svg> elements, so we mimic what happens when specifying them through
    // CSS.
    SetLocation(LayoutPoint(viewportLocation));

    RenderBlock::Layout();
    krys_debug_assert(!NeedsLayout());

    // If our bounds changed, notify the parents.
    if (updateCachedBoundariesInParents)
    {
      if (auto *parent = Parent())
      {
        parent->InvalidateCachedBoundaries();
      }
    }

    // Invalidate all resources of this client if our layout changed.
    SVGResourcesCache::ClientLayoutChanged(*this);

    repainter.RepaintAfterLayout();
  }

  bool LegacyRenderSVGForeignObject::NodeAtFloatPoint(const HitTestRequest &request, HitTestResult &result,
                                                      const FloatPoint &pointInParent,
                                                      HitTestAction hitTestAction) noexcept
  {
    // Embedded content is drawn in the foreground phase.
    if (hitTestAction != HitTestForeground)
    {
      return false;
    }

    FloatPoint localPoint = LocalTransform().Inverse().value_or(AffineTransform {}).MapPoint(pointInParent);

    // Early exit if local point is not contained in clipped viewport area
    if (SVGRenderSupport::IsOverflowHidden(*this) && !_viewport.Contains(localPoint))
    {
      return false;
    }

    // FOs establish a stacking context, so we need to hit-test all layers.
    HitTestLocation hitTestLocation(FlooredLayoutPoint(localPoint));
    return RenderBlock::NodeAtPoint(request, result, hitTestLocation, LayoutPoint(), HitTestForeground)
           || RenderBlock::NodeAtPoint(request, result, hitTestLocation, LayoutPoint(), HitTestFloat)
           || RenderBlock::NodeAtPoint(request, result, hitTestLocation, LayoutPoint(),
                                       HitTestChildBlockBackgrounds);
  }

  LayoutSize LegacyRenderSVGForeignObject::OffsetFromContainer(const RenderElement &container,
                                                               const LayoutPoint &, bool *) const noexcept
  {
    krys_debug_assert(&container == Container());
    krys_debug_assert(!IsInFlowPositioned());
    krys_debug_assert(!IsAbsolutelyPositioned());
    krys_debug_assert(!IsInline());
    (void)container;
    return LocationOffset();
  }
}
