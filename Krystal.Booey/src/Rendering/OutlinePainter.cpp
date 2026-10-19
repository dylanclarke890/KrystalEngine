#include "Krystal.Booey/Rendering/OutlinePainter.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Rendering/BorderPainter.hpp"
#include "Krystal.Booey/Rendering/BorderShape.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/Style/OutlineValue.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD float DeviceScaleFactor(const RenderElement &renderer) noexcept
    {
      return renderer.GetDocument().DeviceScaleFactor();
    }

    // `outline-style: auto` is drawn in the outline colour around the rects, as on platforms other than
    // Cocoa, GTK and WPE (usePlatformFocusRingColorForOutlineStyleAuto and
    // useShrinkWrappedFocusRingForOutlineStyleAuto are false).
    void DrawFocusRing(GraphicsContext &context, const List<FloatRect> &rects, const RenderStyle &style,
                       const Color &color) noexcept
    {
      context.DrawFocusRing(rects, style::Evaluate<float>(style.OutlineOffset(), style::ZoomNeeded {}),
                            style::Evaluate<float>(style.OutlineWidth(), style::ZoomNeeded {}), color);
    }

    void AppendIfNotEmpty(List<LayoutRect> &rects, LayoutRect &&rect) noexcept
    {
      if (rect.IsEmpty())
      {
        return;
      }
      rects.push_back(rect);
    }
  }

  OutlinePainter::OutlinePainter(const PaintInfo &paintInfo) noexcept : _paintInfo(paintInfo)
  {
  }

  void OutlinePainter::PaintOutline(const RenderElement &renderer, const LayoutRect &paintRect) const noexcept
  {
    krys_debug_assert(renderer.HasOutline());

    const auto &styleToUse = renderer.Style();
    // FIXME: Themed focus rings (supportsFocusRing, adjustedPaintRect), once there's a theme; the focus ring
    // is always painted by hand.

    // Only paint the focus ring by hand if the theme isn't able to draw it.
    if (styleToUse.GetOutlineStyle() == OutlineStyle::Auto)
    {
      auto focusRingRects = CollectFocusRingRects(renderer, paintRect.Location(), _paintInfo.PaintContainer);

      PaintFocusRing(renderer, focusRingRects);
      return;
    }

    // FIXME: Link annotations for PDFs (addPDFURLAnnotationForLink) come with printing.

    auto borderStyle = ToBorderStyle(styleToUse.GetOutlineStyle());
    if (!borderStyle || *borderStyle == BorderStyle::None)
    {
      return;
    }

    auto outlineWidth = style::Evaluate<LayoutUnit>(styleToUse.OutlineWidth(), style::ZoomNeeded {});
    auto outlineOffset = style::Evaluate<LayoutUnit>(styleToUse.OutlineOffset(), style::ZoomNeeded {});

    auto outerRect = paintRect;
    outerRect.Inflate(outlineOffset + outlineWidth);
    // FIXME: This prevents outlines from painting inside the object http://webkit.org/b/12042.
    if (outerRect.IsEmpty())
    {
      return;
    }

    auto hasBorderRadius = styleToUse.HasBorderRadius();
    auto closedEdges = RectEdges<bool> {true};

    auto outlineEdgeWidths = RectEdges<LayoutUnit> {outlineWidth};
    auto outlineShape =
      BorderShape::ShapeForOutsetRect(styleToUse, paintRect, outerRect, outlineEdgeWidths, closedEdges);

    auto bleedAvoidance = BleedAvoidance::ShrinkBackground;
    auto appliedClipAlready = false;
    auto edges = GetBorderEdgesForOutline(styleToUse, *borderStyle, DeviceScaleFactor(renderer));
    auto haveAllSolidEdges = BorderPainter::DecorationHasAllSolidEdges(edges);

    BorderPainter {renderer, _paintInfo}.PaintSides(
      outlineShape, {
                      hasBorderRadius ? Maybe<BorderData::RadiiType>(styleToUse.BorderRadii()) : std::nullopt,
                      edges,
                      haveAllSolidEdges,
                      outlineShape.OuterShapeIsRectangular(),
                      outlineShape.InnerShapeIsRectangular(),
                      bleedAvoidance,
                      closedEdges,
                      appliedClipAlready,
                    });
  }

  void OutlinePainter::PaintFocusRing(const RenderElement &renderer,
                                      const List<LayoutRect> &focusRingRects) const noexcept
  {
    const auto &style = renderer.Style();

    krys_debug_assert(style.GetOutlineStyle() == OutlineStyle::Auto);

    auto deviceScaleFactor = DeviceScaleFactor(renderer);
    auto outlineOffset = style::Evaluate<float>(style.OutlineOffset(), style::ZoomNeeded {});

    List<FloatRect> pixelSnappedFocusRingRects;
    for (auto rect : focusRingRects)
    {
      rect.Inflate(LayoutUnit(outlineOffset));
      pixelSnappedFocusRingRects.push_back(SnapRectToDevicePixels(rect, deviceScaleFactor));
    }
    auto focusRingColor = style.VisitedDependentColorWithColorFilter(css::PropertyId::OutlineColor);
    DrawFocusRing(_paintInfo.Context(), pixelSnappedFocusRingRects, style, focusRingColor);
  }

  List<LayoutRect>
    OutlinePainter::CollectFocusRingRects(const RenderElement &renderer, const LayoutPoint &additionalOffset,
                                          const RenderLayerModelObject *paintContainer) noexcept
  {
    List<LayoutRect> rects;
    CollectFocusRingRects(renderer, rects, additionalOffset, paintContainer);
    return rects;
  }

  void OutlinePainter::CollectFocusRingRects(const RenderElement &renderer, List<LayoutRect> &rects,
                                             const LayoutPoint &additionalOffset,
                                             const RenderLayerModelObject *paintContainer) noexcept
  {
    // FIXME: SVG renderers' focus ring rects (addFocusRingRects), once there's SVG painting.
    if (Is<RenderInline>(renderer))
    {
      // FIXME: Inlines' focus ring rects (collectFocusRingRectsForInline, their line boxes) come with inline
      // painting.
      return;
    }
    // FIXME: List boxes' focus ring rects (collectFocusRingRectsForListBox), once there are list boxes.
    if (const auto *block = DynamicDowncast<RenderBlock>(renderer))
    {
      if (CollectFocusRingRectsForBlock(*block, rects, additionalOffset, paintContainer))
      {
        return;
      }
    }
    if (const auto *box = DynamicDowncast<RenderBox>(renderer))
    {
      AppendIfNotEmpty(rects, {additionalOffset, box->Size()});
    }
  }

  bool OutlinePainter::CollectFocusRingRectsForBlock(const RenderBlock &renderer, List<LayoutRect> &rects,
                                                     const LayoutPoint &additionalOffset,
                                                     const RenderLayerModelObject *paintContainer) noexcept
  {
    // FIXME: Text controls (isRenderTextControl), once there are text controls.

    // For blocks inside inlines, we include margins so that we run right up to the inline boxes
    // above and below us (thus getting merged with them to form a single irregular shape).
    // FIXME: Blocks in inline continuations (inlineContinuation, with their collapsed margins and the
    // continuation's own rects), once there are continuations.
    if (renderer.Width() && renderer.Height())
    {
      rects.push_back(LayoutRect(additionalOffset, renderer.Size()));
    }

    // FIXME: Control clips (hasControlClip), once there are form controls.
    if (!renderer.HasNonVisibleOverflow())
    {
      // FIXME: Inline children (collectFocusRingRectsForInlineChildren, the line boxes) come with inline
      // painting.

      for (const auto &box : ChildrenOfType<RenderBox>(renderer))
      {
        CollectFocusRingRectsForChildBox(box, rects, additionalOffset, paintContainer);
      }
    }

    return true;
  }

  void OutlinePainter::CollectFocusRingRectsForChildBox(const RenderBox &box, List<LayoutRect> &rects,
                                                        const LayoutPoint &additionalOffset,
                                                        const RenderLayerModelObject *paintContainer) noexcept
  {
    if (box.IsRenderListMarker() || box.IsOutOfFlowPositioned())
    {
      return;
    }

    FloatPoint pos;
    // FIXME: This doesn't work correctly with transforms.
    if (box.Layer())
    {
      pos = box.LocalToContainerPoint(FloatPoint(), paintContainer);
    }
    else
    {
      pos =
        FloatPoint((additionalOffset.X() + box.X()).ToFloat(), (additionalOffset.Y() + box.Y()).ToFloat());
    }
    CollectFocusRingRects(box, rects, FlooredLayoutPoint(pos), paintContainer);
  }
}
