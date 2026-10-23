#include "Krystal.Booey/Rendering/OutlinePainter.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/FloatPointGraph.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/LayoutRoundedRect.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorInlineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/Rendering/BorderPainter.hpp"
#include "Krystal.Booey/Rendering/BorderShape.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
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

    // `outline-style: auto` is drawn in the outline colour, as on platforms other than Cocoa, GTK and WPE
    // (usePlatformFocusRingColorForOutlineStyleAuto is false), and shrink-wrapped around rounded boxes, as on
    // Cocoa, GTK and WPE (useShrinkWrappedFocusRingForOutlineStyleAuto is true).
    void DrawFocusRing(GraphicsContext &context, const Path &path, const RenderStyle &style,
                       const Color &color) noexcept
    {
      context.DrawFocusRing(path, style::Evaluate<float>(style.OutlineWidth(), style::ZoomNeeded {}), color);
    }

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
    auto &theme = RenderTheme::Singleton();
    auto hasThemedFocusRing = theme.SupportsFocusRing(renderer, styleToUse);

    // Only paint the focus ring by hand if the theme isn't able to draw it.
    if (styleToUse.GetOutlineStyle() == OutlineStyle::Auto && !hasThemedFocusRing)
    {
      LayoutRect paintRectToUse {paintRect};
      if (auto *box = DynamicDowncast<RenderBox>(renderer))
      {
        paintRectToUse = theme.AdjustedPaintRect(*box, paintRectToUse);
      }
      auto focusRingRects =
        CollectFocusRingRects(renderer, paintRectToUse.Location(), _paintInfo.PaintContainer);

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

  void OutlinePainter::PaintOutline(const RenderInline &renderer,
                                    const LayoutPoint &paintOffset) const noexcept
  {
    krys_debug_assert(renderer.HasOutline());

    const auto &styleToUse = renderer.Style();

    if (styleToUse.GetOutlineStyle() == OutlineStyle::Auto)
    {
      auto focusRingRects = CollectFocusRingRects(renderer, paintOffset, _paintInfo.PaintContainer);

      PaintFocusRing(renderer, focusRingRects);
      return;
    }

    // FIXME: Link annotations for PDFs (addPDFURLAnnotationForLink) come with printing.

    if (_paintInfo.Context().PaintingDisabled() || !styleToUse.HasOutline())
    {
      return;
    }

    if (!renderer.ContainingBlock())
    {
      krys_debug_assert(false);
      return;
    }

    auto isHorizontalWritingMode = renderer.IsHorizontalWritingMode();
    const auto &containingBlock = *renderer.ContainingBlock();
    auto isFlipped = containingBlock.GetWritingMode().IsBlockFlipped();
    List<LayoutRect> rects;
    for (auto box = inline_iterator::LineLeftmostInlineBoxFor(renderer); box;
         box.TraverseInlineBoxLineRightward())
    {
      auto lineBox = box->GetLineBox();
      auto logicalTop = std::max(lineBox->ContentLogicalTop(), box->LogicalTop());
      auto logicalBottom = std::min(lineBox->ContentLogicalBottom(), box->LogicalBottom());
      auto enclosingVisualRect = FloatRect {box->LogicalLeftIgnoringInlineDirection(), logicalTop,
                                            box->LogicalWidth(), logicalBottom - logicalTop};

      if (!isHorizontalWritingMode)
      {
        enclosingVisualRect = enclosingVisualRect.TransposedRect();
      }

      if (isFlipped)
      {
        containingBlock.FlipForWritingMode(enclosingVisualRect);
      }

      rects.push_back(LayoutRect {enclosingVisualRect});
    }
    PaintOutlineWithLineRects(renderer, paintOffset, rects);
  }

  void OutlinePainter::PaintOutlineWithLineRects(const RenderInline &renderer, const LayoutPoint &paintOffset,
                                                 const List<LayoutRect> &lineRects) const noexcept
  {
    if (lineRects.size() == 1)
    {
      auto adjustedPaintRect = lineRects[0];
      adjustedPaintRect.MoveBy(paintOffset);
      PaintOutline(static_cast<const RenderElement &>(renderer), adjustedPaintRect);
      return;
    }

    const auto &styleToUse = renderer.Style();

    auto outlineOffset = style::Evaluate<float>(styleToUse.OutlineOffset(), style::ZoomNeeded {});
    auto outlineWidth = style::Evaluate<float>(styleToUse.OutlineWidth(), style::ZoomNeeded {});

    auto deviceScaleFactor = DeviceScaleFactor(renderer);

    List<FloatRect> pixelSnappedRects;
    for (size_t index = 0; index < lineRects.size(); ++index)
    {
      auto rect = lineRects[index];

      rect.MoveBy(paintOffset);
      rect.Inflate(LayoutUnit(outlineOffset + outlineWidth / 2));
      pixelSnappedRects.push_back(SnapRectToDevicePixels(rect, deviceScaleFactor));
    }
    auto path = PathWithShrinkWrappedRects(pixelSnappedRects, styleToUse.BorderRadii(), outlineOffset,
                                           styleToUse.GetWritingMode(), deviceScaleFactor);
    if (path.IsEmpty())
    {
      // Disjoint line spanning inline boxes.
      for (auto rect : lineRects)
      {
        rect.MoveBy(paintOffset);
        PaintOutline(static_cast<const RenderElement &>(renderer), rect);
      }
      return;
    }

    auto &graphicsContext = _paintInfo.Context();
    auto outlineColor = styleToUse.VisitedDependentColorWithColorFilter(css::PropertyId::OutlineColor);
    auto useTransparencyLayer = !outlineColor.IsOpaque();
    if (useTransparencyLayer)
    {
      graphicsContext.BeginTransparencyLayer(outlineColor.AlphaAsFloat());
      outlineColor = outlineColor.OpaqueColor();
    }

    graphicsContext.SetStrokeColor(outlineColor);
    graphicsContext.SetStrokeThickness(outlineWidth);
    graphicsContext.SetStrokeStyle(StrokeStyle::SolidStroke);
    graphicsContext.StrokePath(path);

    if (useTransparencyLayer)
    {
      graphicsContext.EndTransparencyLayer();
    }
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
    if (style.HasBorderRadius())
    {
      auto path = PathWithShrinkWrappedRects(pixelSnappedFocusRingRects, style.BorderRadii(), outlineOffset,
                                             style.GetWritingMode(), deviceScaleFactor);
      if (path.IsEmpty())
      {
        for (auto rect : pixelSnappedFocusRingRects)
        {
          path.AddRect(rect);
        }
      }
      DrawFocusRing(_paintInfo.Context(), path, style, focusRingColor);
    }
    else
    {
      DrawFocusRing(_paintInfo.Context(), pixelSnappedFocusRingRects, style, focusRingColor);
    }
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
    if (const auto *renderInline = DynamicDowncast<RenderInline>(renderer))
    {
      CollectFocusRingRectsForInline(*renderInline, rects, additionalOffset, paintContainer);
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

  void OutlinePainter::CollectFocusRingRectsForInline(const RenderInline &renderer, List<LayoutRect> &rects,
                                                      const LayoutPoint &additionalOffset,
                                                      const RenderLayerModelObject *paintContainer) noexcept
  {
    renderer.CollectLineBoxRects(rects, additionalOffset);

    for (const auto &child : ChildrenOfType<RenderBoxModelObject>(renderer))
    {
      if (child.IsRenderListMarker())
      {
        continue;
      }
      FloatPoint pos(additionalOffset);
      // FIXME: This doesn't work correctly with transforms.
      if (child.HasLayer())
      {
        pos = child.LocalToContainerPoint(FloatPoint(), paintContainer);
      }
      else if (const auto *box = DynamicDowncast<RenderBox>(child))
      {
        pos.Move(box->LocationOffset());
      }
      CollectFocusRingRects(child, rects, FlooredIntPoint(pos), paintContainer);
    }

    // There are no continuations: blocks in inlines are laid out in the inline's lines.
  }

  void OutlinePainter::CollectFocusRingRectsForInlineChildren(
    const RenderBlockFlow &renderer, List<LayoutRect> &rects, const LayoutPoint &additionalOffset,
    const RenderLayerModelObject *paintContainer) noexcept
  {
    krys_debug_assert(renderer.ChildrenInline());

    bool hasBlockContent = false;

    for (auto box = inline_iterator::FirstRootInlineBoxFor(renderer); box;
         box.TraverseInlineBoxLineRightward())
    {
      auto lineBox = box->GetLineBox();
      if (lineBox->HasBlockContent())
      {
        hasBlockContent = true;
        continue;
      }
      // FIXME: This is mixing physical and logical coordinates.
      auto unflippedVisualRect = box->VisualRectIgnoringBlockDirection();
      auto top = std::max(lineBox->ContentLogicalTop(), unflippedVisualRect.Y());
      auto bottom = std::min(lineBox->ContentLogicalBottom(), unflippedVisualRect.MaxY());
      auto rect = LayoutRect {LayoutUnit {additionalOffset.X() + unflippedVisualRect.X()},
                              LayoutUnit {additionalOffset.Y() + top},
                              LayoutUnit {unflippedVisualRect.Width()}, LayoutUnit {bottom - top}};
      AppendIfNotEmpty(rects, krys::move(rect));
    }

    if (hasBlockContent)
    {
      for (auto line = inline_iterator::FirstLineBoxFor(renderer); line; line.TraverseNext())
      {
        auto blockLevelBox = line->BlockLevelBox();
        if (!blockLevelBox)
        {
          continue;
        }
        const auto &renderBox = Downcast<RenderBox>(blockLevelBox->Renderer());
        CollectFocusRingRectsForChildBox(renderBox, rects, additionalOffset, paintContainer);
      }
    }
  }

  bool OutlinePainter::CollectFocusRingRectsForBlock(const RenderBlock &renderer, List<LayoutRect> &rects,
                                                     const LayoutPoint &additionalOffset,
                                                     const RenderLayerModelObject *paintContainer) noexcept
  {
    if (renderer.IsRenderTextControl())
    {
      return false;
    }

    // For blocks inside inlines, we include margins so that we run right up to the inline boxes
    // above and below us (thus getting merged with them to form a single irregular shape).
    // FIXME: Blocks in inline continuations (inlineContinuation, with their collapsed margins and the
    // continuation's own rects), once there are continuations.
    if (renderer.Width() && renderer.Height())
    {
      rects.push_back(LayoutRect(additionalOffset, renderer.Size()));
    }

    if (!renderer.HasNonVisibleOverflow() && !renderer.HasControlClip())
    {
      if (renderer.ChildrenInline() && Is<RenderBlockFlow>(renderer))
      {
        CollectFocusRingRectsForInlineChildren(Downcast<RenderBlockFlow>(renderer), rects, additionalOffset,
                                               paintContainer);
      }

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

  namespace
  {
    KRYS_NODISCARD std::pair<FloatPoint, FloatPoint>
      StartAndEndPointsForCorner(const FloatPointGraph::Edge &fromEdge, const FloatPointGraph::Edge &toEdge,
                                 const FloatSize &radius) noexcept
    {
      FloatSize fromEdgeVector = *fromEdge.second - *fromEdge.first;
      FloatSize toEdgeVector = *toEdge.second - *toEdge.first;

      FloatPoint fromEdgeNorm = ToFloatPoint(fromEdgeVector);
      fromEdgeNorm.Normalise();
      FloatSize fromOffset = FloatSize(radius.Width() * fromEdgeNorm.X(), radius.Height() * fromEdgeNorm.Y());
      FloatPoint startPoint = *fromEdge.second - fromOffset;

      FloatPoint toEdgeNorm = ToFloatPoint(toEdgeVector);
      toEdgeNorm.Normalise();
      FloatSize toOffset = FloatSize(radius.Width() * toEdgeNorm.X(), radius.Height() * toEdgeNorm.Y());
      FloatPoint endPoint = *toEdge.first + toOffset;
      return std::make_pair(startPoint, endPoint);
    }

    enum class CornerType : uint8
    {
      TopLeft,
      TopRight,
      BottomRight,
      BottomLeft,
      Other
    };

    KRYS_NODISCARD CornerType GetCornerType(const FloatPointGraph::Edge &fromEdge,
                                            const FloatPointGraph::Edge &toEdge) noexcept
    {
      auto fromEdgeVector = *fromEdge.second - *fromEdge.first;
      auto toEdgeVector = *toEdge.second - *toEdge.first;

      if (fromEdgeVector.Height() < 0 && toEdgeVector.Width() > 0)
      {
        return CornerType::TopLeft;
      }
      if (fromEdgeVector.Width() > 0 && toEdgeVector.Height() > 0)
      {
        return CornerType::TopRight;
      }
      if (fromEdgeVector.Height() > 0 && toEdgeVector.Width() < 0)
      {
        return CornerType::BottomRight;
      }
      if (fromEdgeVector.Width() < 0 && toEdgeVector.Height() < 0)
      {
        return CornerType::BottomLeft;
      }
      return CornerType::Other;
    }

    KRYS_NODISCARD CornerType CornerTypeForMultiline(const FloatPointGraph::Edge &fromEdge,
                                                     const FloatPointGraph::Edge &toEdge,
                                                     const List<FloatPoint> &corners) noexcept
    {
      auto corner = GetCornerType(fromEdge, toEdge);
      if (corner == CornerType::TopLeft && corners.at(0) == *fromEdge.second)
      {
        return corner;
      }
      if (corner == CornerType::TopRight && corners.at(1) == *fromEdge.second)
      {
        return corner;
      }
      if (corner == CornerType::BottomRight && corners.at(2) == *fromEdge.second)
      {
        return corner;
      }
      if (corner == CornerType::BottomLeft && corners.at(3) == *fromEdge.second)
      {
        return corner;
      }
      return CornerType::Other;
    }

    KRYS_NODISCARD std::pair<FloatPoint, FloatPoint>
      ControlPointsForBezierCurve(CornerType cornerType, const FloatPointGraph::Edge &fromEdge,
                                  const FloatPointGraph::Edge &toEdge, const FloatSize &radius) noexcept
    {
      FloatPoint cp1;
      FloatPoint cp2;
      switch (cornerType)
      {
        case CornerType::TopLeft:
        {
          cp1 = FloatPoint(fromEdge.second->X(),
                           fromEdge.second->Y() + radius.Height() * Path::CircleControlPoint());
          cp2 =
            FloatPoint(toEdge.first->X() + radius.Width() * Path::CircleControlPoint(), toEdge.first->Y());
          break;
        }
        case CornerType::TopRight:
        {
          cp1 = FloatPoint(fromEdge.second->X() - radius.Width() * Path::CircleControlPoint(),
                           fromEdge.second->Y());
          cp2 =
            FloatPoint(toEdge.first->X(), toEdge.first->Y() + radius.Height() * Path::CircleControlPoint());
          break;
        }
        case CornerType::BottomRight:
        {
          cp1 = FloatPoint(fromEdge.second->X(),
                           fromEdge.second->Y() - radius.Height() * Path::CircleControlPoint());
          cp2 =
            FloatPoint(toEdge.first->X() - radius.Width() * Path::CircleControlPoint(), toEdge.first->Y());
          break;
        }
        case CornerType::BottomLeft:
        {
          cp1 = FloatPoint(fromEdge.second->X() + radius.Width() * Path::CircleControlPoint(),
                           fromEdge.second->Y());
          cp2 =
            FloatPoint(toEdge.first->X(), toEdge.first->Y() - radius.Height() * Path::CircleControlPoint());
          break;
        }
        case CornerType::Other:
        {
          krys_debug_assert(false);
          break;
        }
      }
      return std::make_pair(cp1, cp2);
    }

    KRYS_NODISCARD FloatRoundedRect::Radii
      AdjustedRadiiForHuggingCurve(const FloatRoundedRect::Radii &inputRadii, float outlineOffset) noexcept
    {
      // This adjusts the radius so that it follows the border curve even when offset is present.
      auto adjustedRadius = [outlineOffset](const FloatSize &radius)
      {
        FloatSize expandSize;
        if (radius.Width() > outlineOffset)
        {
          expandSize.SetWidth(std::min(outlineOffset, radius.Width() - outlineOffset));
        }
        if (radius.Height() > outlineOffset)
        {
          expandSize.SetHeight(std::min(outlineOffset, radius.Height() - outlineOffset));
        }
        FloatSize adjustedRadius = radius;
        adjustedRadius.Expand(expandSize.Width(), expandSize.Height());
        // Do not go to negative radius.
        return adjustedRadius.ExpandedTo(FloatSize(0, 0));
      };

      return {
        adjustedRadius(inputRadii.TopLeft()),
        adjustedRadius(inputRadii.TopRight()),
        adjustedRadius(inputRadii.BottomLeft()),
        adjustedRadius(inputRadii.BottomRight()),
      };
    }

    KRYS_NODISCARD Maybe<FloatRect> RectFromPolygon(const FloatPointGraph::Polygon &poly) noexcept
    {
      if (poly.size() != 4)
      {
        return Maybe<FloatRect>();
      }

      Maybe<FloatPoint> topLeft;
      Maybe<FloatPoint> bottomRight;
      for (size_t i = 0; i < poly.size(); ++i)
      {
        const auto &toEdge = poly[i];
        const auto &fromEdge = (i > 0) ? poly[i - 1] : poly[poly.size() - 1];
        auto corner = GetCornerType(fromEdge, toEdge);
        if (corner == CornerType::TopLeft)
        {
          krys_debug_assert(!topLeft);
          topLeft = *fromEdge.second;
        }
        else if (corner == CornerType::BottomRight)
        {
          krys_debug_assert(!bottomRight);
          bottomRight = *fromEdge.second;
        }
      }
      if (!topLeft || !bottomRight)
      {
        return Maybe<FloatRect>();
      }
      return FloatRect(topLeft.value(), bottomRight.value());
    }
  }

  Path OutlinePainter::PathWithShrinkWrappedRects(const List<FloatRect> &rects,
                                                  const style::BorderRadius &radii, float outlineOffset,
                                                  WritingMode writingMode, float deviceScaleFactor) noexcept
  {
    auto roundedRect = [&radii, outlineOffset, deviceScaleFactor](const FloatRect &rect)
    {
      auto adjustedRadii = AdjustedRadiiForHuggingCurve(
        style::Evaluate<FloatRoundedRect::Radii>(radii, rect.Size(), style::ZoomNeeded {}), outlineOffset);
      adjustedRadii.Scale(CalcBorderRadiiConstraintScaleFor(rect, adjustedRadii));

      LayoutRoundedRect layoutRoundedRect(
        LayoutRect(rect), LayoutRoundedRect::Radii(
                            LayoutSize(adjustedRadii.TopLeft()), LayoutSize(adjustedRadii.TopRight()),
                            LayoutSize(adjustedRadii.BottomLeft()), LayoutSize(adjustedRadii.BottomRight())));
      Path path;
      path.AddRoundedRect(layoutRoundedRect.PixelSnappedRoundedRectForPainting(deviceScaleFactor));
      return path;
    };

    if (rects.size() == 1)
    {
      return roundedRect(rects.at(0));
    }

    auto [graph, polys] = FloatPointGraph::PolygonsForRect(rects);

    // Fall back to corner painting with no radius for empty and disjoint rectangles.
    if (!polys.size() || polys.size() > 1)
    {
      return Path();
    }
    const auto &poly = polys.at(0);
    // Fast path when poly has one rect only.
    Maybe<FloatRect> rect = RectFromPolygon(poly);
    if (rect)
    {
      return roundedRect(rect.value());
    }

    Path path;
    // Multiline outline needs to match multiline border painting. Only first and last lines are getting
    // rounded borders.
    auto isLeftToRight = writingMode.IsBidiLTR();
    auto firstLineRect = isLeftToRight ? rects.at(0) : rects.at(rects.size() - 1);
    auto lastLineRect = isLeftToRight ? rects.at(rects.size() - 1) : rects.at(0);
    // Adjust radius so that it matches the box border.
    auto firstLineRadii =
      style::Evaluate<FloatRoundedRect::Radii>(radii, firstLineRect.Size(), style::ZoomNeeded {});
    auto lastLineRadii =
      style::Evaluate<FloatRoundedRect::Radii>(radii, lastLineRect.Size(), style::ZoomNeeded {});
    firstLineRadii.Scale(CalcBorderRadiiConstraintScaleFor(firstLineRect, firstLineRadii));
    lastLineRadii.Scale(CalcBorderRadiiConstraintScaleFor(lastLineRect, lastLineRadii));

    // physical topLeft/topRight/bottomRight/bottomLeft
    auto isHorizontal = writingMode.IsHorizontal();
    auto corners = List<FloatPoint> {
      firstLineRect.MinXMinYCorner(),
      isHorizontal ? lastLineRect.MaxXMinYCorner() : firstLineRect.MaxXMinYCorner(),
      lastLineRect.MaxXMaxYCorner(),
      isHorizontal ? firstLineRect.MinXMaxYCorner() : lastLineRect.MinXMaxYCorner(),
    };

    for (size_t i = 0; i < poly.size(); ++i)
    {
      auto moveOrAddLineTo = [i, &path](const FloatPoint &startPoint)
      {
        if (!i)
        {
          path.MoveTo(startPoint);
        }
        else
        {
          path.AddLineTo(startPoint);
        }
      };
      const auto &toEdge = poly[i];
      const auto &fromEdge = (i > 0) ? poly[i - 1] : poly[poly.size() - 1];
      FloatSize radius;
      auto corner = CornerTypeForMultiline(fromEdge, toEdge, corners);
      switch (corner)
      {
        case CornerType::TopLeft:     radius = firstLineRadii.TopLeft(); break;
        case CornerType::TopRight:    radius = lastLineRadii.TopRight(); break;
        case CornerType::BottomRight: radius = lastLineRadii.BottomRight(); break;
        case CornerType::BottomLeft:  radius = firstLineRadii.BottomLeft(); break;
        case CornerType::Other:
          // Do not apply border radius on corners that normal border painting skips. (multiline content)
          moveOrAddLineTo(*fromEdge.second);
          continue;
      }
      auto [startPoint, endPoint] = StartAndEndPointsForCorner(fromEdge, toEdge, radius);
      moveOrAddLineTo(startPoint);

      auto [cp1, cp2] = ControlPointsForBezierCurve(corner, fromEdge, toEdge, radius);
      path.AddBezierCurveTo(cp1, cp2, endPoint);
    }
    path.CloseSubpath();
    return path;
  }
}
