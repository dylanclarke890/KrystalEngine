#include "Krystal.Booey/Rendering/RenderHTMLCanvas.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/HTML/HTMLCanvasElement.hpp"
#include "Krystal.Booey/Rendering/ImageQualityController.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderLayerBacking.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  RenderHTMLCanvas::RenderHTMLCanvas(html::HTMLCanvasElement &element, RenderStyle &&style) noexcept
      : RenderReplaced(Type::HTMLCanvas, element, krys::move(style), LayoutSize(element.Size()))
  {
    krys_debug_assert(IsRenderHTMLCanvas());
  }

  RenderHTMLCanvas::~RenderHTMLCanvas() noexcept = default;

  html::HTMLCanvasElement &RenderHTMLCanvas::CanvasElement() const noexcept
  {
    return Downcast<html::HTMLCanvasElement>(NodeForNonAnonymous());
  }

  bool RenderHTMLCanvas::RequiresLayer() const noexcept
  {
    if (RenderReplaced::RequiresLayer())
    {
      return true;
    }

    return GetCanvasCompositingStrategy(*this) != CanvasCompositingStrategy::CanvasPaintedToEnclosingLayer;
  }

  void RenderHTMLCanvas::PaintReplaced(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    GraphicsContext &context = paintInfo.Context();

    LayoutRect contentBoxRect = ContentBoxRect();

    if (context.DetectingContentfulPaint())
    {
      if (!context.ContentfulPaintDetected() && CanvasElement().RenderingContext())
      {
        context.SetContentfulPaintDetected();
      }
      return;
    }

    contentBoxRect.MoveBy(paintOffset);
    LayoutRect replacedContentRect = ReplacedContentRect();
    replacedContentRect.MoveBy(paintOffset);

    // Not allowed to overflow the content box.
    bool clip = !contentBoxRect.Contains(replacedContentRect);
    GraphicsContextStateSaver stateSaver(context, clip);
    if (clip)
    {
      context.Clip(FloatRect(SnappedIntRect(contentBoxRect)));
    }

    // FIXME: Recording repainted objects for paint milestones (addRelevantRepaintedObject), once there are
    // paint milestones.

    InterpolationQualityMaintainer interpolationMaintainer(
      context, ImageQualityController::InterpolationQualityFromStyle(Style()));

    CanvasElement().SetIsSnapshotting(paintInfo.PaintBehavior.Contains(PaintBehavior::Snapshotting));
    CanvasElement().Paint(context, replacedContentRect);
    CanvasElement().SetIsSnapshotting(false);
  }

  void RenderHTMLCanvas::CanvasSizeChanged() noexcept
  {
    IntSize canvasSize = CanvasElement().Size();
    LayoutSize zoomedSize(canvasSize.Width() * Style().UsedZoom(), canvasSize.Height() * Style().UsedZoom());

    if (zoomedSize == IntrinsicSize())
    {
      return;
    }

    SetIntrinsicSize(zoomedSize);

    if (!Parent())
    {
      return;
    }

    (void)SetNeedsLayoutIfNeededAfterIntrinsicSizeChange();
  }
}
