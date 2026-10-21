#include "Krystal.Booey/Rendering/BorderPainter.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Rendering/BorderShape.hpp"
#include "Krystal.Booey/Rendering/NinePieceImagePainter.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/BorderImage.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include <cmath>
#include <numeric>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD bool BorderStyleFillsBorderArea(BorderStyle style) noexcept
    {
      switch (style)
      {
        case BorderStyle::None:
        case BorderStyle::Hidden:
        case BorderStyle::Inset:
        case BorderStyle::Groove:
        case BorderStyle::Outset:
        case BorderStyle::Ridge:
        case BorderStyle::Solid:  return true;
        case BorderStyle::Dotted:
        case BorderStyle::Dashed:
        case BorderStyle::Double: return false;
      }
      return true;
    }

    KRYS_NODISCARD bool StyleRequiresClipPolygon(BorderStyle style) noexcept
    {
      switch (style)
      {
        case BorderStyle::None:
        case BorderStyle::Hidden:
        case BorderStyle::Inset:
        case BorderStyle::Groove:
        case BorderStyle::Outset:
        case BorderStyle::Ridge:
        case BorderStyle::Solid:
        case BorderStyle::Double: return false;
        case BorderStyle::Dotted:
        case BorderStyle::Dashed:
          // These are drawn with a stroke, so we have to clip to get corner miters.
          return true;
      }

      return false;
    }

    KRYS_NODISCARD bool BorderStyleHasInnerDetail(BorderStyle style) noexcept
    {
      switch (style)
      {
        case BorderStyle::None:
        case BorderStyle::Hidden:
        case BorderStyle::Inset:
        case BorderStyle::Outset:
        case BorderStyle::Solid:
        case BorderStyle::Dotted:
        case BorderStyle::Dashed: return false;

        case BorderStyle::Groove:
        case BorderStyle::Ridge:
        case BorderStyle::Double: return true;
      }

      return false;
    }

    KRYS_NODISCARD bool BorderStyleIsDottedOrDashed(BorderStyle style) noexcept
    {
      return style == BorderStyle::Dotted || style == BorderStyle::Dashed;
    }

    KRYS_NODISCARD bool DecorationHasAllSimpleEdges(const RectEdges<BorderEdge> &edges) noexcept
    {
      for (auto side : AllBoxSides)
      {
        const auto &currEdge = edges.At(side);

        if (!currEdge.WidthForPainting())
        {
          continue;
        }

        if (!BorderStyleFillsBorderArea(currEdge.Style()))
        {
          return false;
        }
      }
      return true;
    }

    KRYS_NODISCARD LayoutRect CalculateSideRect(const LayoutRect &outerBorderRect, const BorderEdges &edges,
                                                BoxSide side) noexcept
    {
      auto sideRect = outerBorderRect;
      float width = edges.At(side).WidthForPainting();

      switch (side)
      {
        case BoxSide::Top:    sideRect.SetHeight(LayoutUnit(width)); break;
        case BoxSide::Right:  sideRect.ShiftXEdgeTo(sideRect.MaxX() - LayoutUnit(width)); break;
        case BoxSide::Bottom: sideRect.ShiftYEdgeTo(sideRect.MaxY() - LayoutUnit(width)); break;
        case BoxSide::Left:   sideRect.SetWidth(LayoutUnit(width)); break;
      }

      return sideRect;
    }

    KRYS_NODISCARD LayoutSize SizeForDevicePixel(const GraphicsContext &context,
                                                 float devicePixelRatio) noexcept
    {
      auto transform = context.GetCTM();
      return {LayoutUnit(CeilToDevicePixel(LayoutUnit(static_cast<float>(1.0 / transform.XScale())),
                                           devicePixelRatio)),
              LayoutUnit(CeilToDevicePixel(LayoutUnit(static_cast<float>(1.0 / transform.YScale())),
                                           devicePixelRatio))};
    }
  }

  BorderPainter::BorderPainter(const RenderElement &renderer, const PaintInfo &paintInfo) noexcept
      : _renderer(renderer), _paintInfo(paintInfo)
  {
  }

  Maybe<Path> BorderPainter::PathForBorderArea(const LayoutRect &rect, const RenderStyle &style,
                                               float deviceScaleFactor, RectEdges<bool> closedEdges) noexcept
  {
    auto edges = GetBorderEdges(style, deviceScaleFactor, closedEdges);
    if (!DecorationHasAllSimpleEdges(edges))
    {
      return std::nullopt;
    }

    auto borderShape = BorderShape::ShapeForBorderRect(style, rect, closedEdges);
    return borderShape.PathForBorderArea(deviceScaleFactor);
  }

  LayoutRect ShrinkRectByOneDevicePixel(const GraphicsContext &context, const LayoutRect &rect,
                                        float devicePixelRatio) noexcept
  {
    auto shrunkRect = rect;
    auto devicePixelUnits = SizeForDevicePixel(context, devicePixelRatio);
    shrunkRect.Inflate(-devicePixelUnits);
    return shrunkRect;
  }

  bool BorderPainter::DecorationHasAllSolidEdges(const RectEdges<BorderEdge> &edges) noexcept
  {
    for (auto side : AllBoxSides)
    {
      const auto &currEdge = edges.At(side);

      if (currEdge.PresentButInvisible() || !currEdge.WidthForPainting())
      {
        continue;
      }

      if (currEdge.Style() != BorderStyle::Solid)
      {
        return false;
      }
    }
    return true;
  }

  void BorderPainter::PaintBorder(const LayoutRect &rect, const RenderStyle &style,
                                  BleedAvoidance bleedAvoidance, RectEdges<bool> closedEdges) const noexcept
  {
    auto &graphicsContext = _paintInfo.Context();

    if (graphicsContext.PaintingDisabled())
    {
      return;
    }

    auto paintsBorderImage = [&](LayoutRect rect, const style::BorderImage &borderImage)
    {
      auto image = borderImage.Source.TryStyleImage();
      if (!image)
      {
        return false;
      }

      if (!image->IsLoaded(&_renderer))
      {
        return false;
      }

      if (!image->CanRender(&_renderer, style.UsedZoom()))
      {
        return false;
      }

      auto rectWithOutsets = rect;
      rectWithOutsets.Expand(style.ImageOutsets(borderImage));
      return !rectWithOutsets.IsEmpty();
    };

    if (rect.IsEmpty() && !paintsBorderImage(rect, style.GetBorderImage()))
    {
      return;
    }

    auto rectToClipOut = const_cast<RenderElement &>(_renderer).PaintRectToClipOutFromBorder(rect);
    bool appliedClipAlready = !rectToClipOut.IsEmpty();
    GraphicsContextStateSaver stateSave(graphicsContext, appliedClipAlready);
    if (!rectToClipOut.IsEmpty())
    {
      graphicsContext.ClipOut(SnapRectToDevicePixels(rectToClipOut, Document().DeviceScaleFactor()));
    }

    // border-image is not affected by border-radius.
    if (PaintNinePieceImage(rect, style, style.GetBorderImage()))
    {
      return;
    }

    bool forceBlackBorder = _paintInfo.PaintBehavior.Contains(PaintBehavior::ForceBlackBorder);
    auto [shape, edges] = [&]() -> Pair<BorderShape, BorderEdges>
    {
      switch (bleedAvoidance)
      {
        case BleedAvoidance::None:
        case BleedAvoidance::ShrinkBackground:
        case BleedAvoidance::UseTransparencyLayer:
          return {BorderShape::ShapeForBorderRect(style, rect, closedEdges),
                  GetBorderEdges(style, Document().DeviceScaleFactor(), closedEdges, {}, forceBlackBorder)};

        case BleedAvoidance::BackgroundOverBorder:
        {
          // Shrink the inner edge so there's no gap between the border and the background, which will be
          // painted atop.
          auto shrinkAmount = SizeForDevicePixel(_paintInfo.Context(), Document().DeviceScaleFactor());
          auto edges = GetBorderEdges(style, Document().DeviceScaleFactor(), closedEdges, shrinkAmount,
                                      forceBlackBorder);
          auto borderWidths = RectEdges<LayoutUnit> {edges.Top().Width(), edges.Right().Width(),
                                                     edges.Bottom().Width(), edges.Left().Width()};

          return {BorderShape::ShapeForBorderRect(style, rect, borderWidths, closedEdges), krys::move(edges)};
        }
      }

      return {BorderShape({}, {0_lu}), {}};
    }();

    bool outerEdgeIsRectangular = !shape.IsRounded() || shape.OuterShapeContains(_paintInfo.Rect);
    bool innerEdgeIsRectangular = shape.InnerShapeIsRectangular();

    PaintSides(shape,
               {
                 style.HasBorderRadius() ? Maybe<BorderData::RadiiType>(style.BorderRadii()) : std::nullopt,
                 edges,
                 DecorationHasAllSolidEdges(edges),
                 outerEdgeIsRectangular,
                 innerEdgeIsRectangular,
                 bleedAvoidance,
                 closedEdges,
                 appliedClipAlready,
               });
  }

  void BorderPainter::PaintSides(const BorderShape &borderShape, const Sides &sides) const noexcept
  {
    GraphicsContext &graphicsContext = _paintInfo.Context();

    krys_debug_assert(!graphicsContext.PaintingDisabled());

    // If no borders intersects with the dirty area, we can skip the painting.
    if (borderShape.InnerShapeContains(_paintInfo.Rect))
    {
      return;
    }

    auto deviceScaleFactor = Document().DeviceScaleFactor();
    bool haveAlphaColor = false;
    bool haveAllDoubleEdges = true;
    int32 numEdgesVisible = 4;
    bool allEdgesShareColor = true;
    Maybe<BoxSide> firstVisibleSide;
    EnumSet<BoxSide> edgesToDraw;

    for (auto boxSide : AllBoxSides)
    {
      const auto &currEdge = sides.Edges.At(boxSide);

      if (currEdge.ShouldRender())
      {
        edgesToDraw.Add(boxSide);
      }

      if (currEdge.PresentButInvisible())
      {
        --numEdgesVisible;
        allEdgesShareColor = false;
        continue;
      }

      if (!currEdge.WidthForPainting())
      {
        --numEdgesVisible;
        continue;
      }

      if (!firstVisibleSide)
      {
        firstVisibleSide = boxSide;
      }
      else if (!EqualIgnoringSemanticColor(currEdge.GetColor(), sides.Edges.At(*firstVisibleSide).GetColor()))
      {
        allEdgesShareColor = false;
      }

      if (!currEdge.GetColor().IsOpaque())
      {
        haveAlphaColor = true;
      }

      if (currEdge.Style() != BorderStyle::Double)
      {
        haveAllDoubleEdges = false;
      }
    }

    auto drawUniformRoundedOrAlphaBorders = [&]()
    {
      krys_debug_assert(numEdgesVisible == 4);
      krys_debug_assert(allEdgesShareColor);
      krys_debug_assert(sides.HaveAllSolidEdges);
      krys_debug_assert(!sides.OuterEdgeIsRectangular || haveAlphaColor);

      auto path = borderShape.PathForOuterShape(deviceScaleFactor);
      borderShape.AddInnerShapeToPath(path, deviceScaleFactor);

      graphicsContext.SetFillRule(WindRule::EvenOdd);
      graphicsContext.SetFillColor(sides.Edges.At(*firstVisibleSide).GetColor());
      graphicsContext.FillPath(path);
    };

    auto drawUniformDoubleBorders = [&]()
    {
      krys_debug_assert(numEdgesVisible == 4);
      krys_debug_assert(allEdgesShareColor);
      krys_debug_assert(haveAllDoubleEdges);
      krys_debug_assert(!sides.OuterEdgeIsRectangular || haveAlphaColor);

      auto path = borderShape.PathForOuterShape(deviceScaleFactor);

      RectEdges<LayoutUnit> outerThirdInsets;
      RectEdges<LayoutUnit> innerThirdInsets;

      sides.Edges.At(BoxSide::Top)
        .GetDoubleBorderStripeWidths(outerThirdInsets.Top(), innerThirdInsets.Top());
      sides.Edges.At(BoxSide::Right)
        .GetDoubleBorderStripeWidths(outerThirdInsets.Right(), innerThirdInsets.Right());
      sides.Edges.At(BoxSide::Bottom)
        .GetDoubleBorderStripeWidths(outerThirdInsets.Bottom(), innerThirdInsets.Bottom());
      sides.Edges.At(BoxSide::Left)
        .GetDoubleBorderStripeWidths(outerThirdInsets.Left(), innerThirdInsets.Left());

      auto outerThirdShape = borderShape.ShapeWithBorderWidths(outerThirdInsets);
      outerThirdShape.AddInnerShapeToPath(path, deviceScaleFactor);

      auto innerThirdShape = borderShape.ShapeWithBorderWidths(innerThirdInsets);
      innerThirdShape.AddInnerShapeToPath(path, deviceScaleFactor);

      borderShape.AddInnerShapeToPath(path, deviceScaleFactor);

      graphicsContext.SetFillRule(WindRule::EvenOdd);
      graphicsContext.SetFillColor(sides.Edges.At(*firstVisibleSide).GetColor());
      graphicsContext.FillPath(path);
    };

    if ((sides.HaveAllSolidEdges || haveAllDoubleEdges) && allEdgesShareColor)
    {
      // Fast path for drawing all solid edges and all unrounded double edges which need path-based rendering
      // because of rounding or alpha colors.
      if (numEdgesVisible == 4 && (!sides.OuterEdgeIsRectangular || haveAlphaColor))
      {
        if (sides.HaveAllSolidEdges)
        {
          drawUniformRoundedOrAlphaBorders();
          return;
        }

        if (haveAllDoubleEdges)
        {
          drawUniformDoubleBorders();
          return;
        }
      }

      // Avoid creating transparent layers
      if (sides.HaveAllSolidEdges && numEdgesVisible != 4 && sides.OuterEdgeIsRectangular && haveAlphaColor)
      {
        auto outerBorderRect = borderShape.BorderRect();
        Path path;
        for (auto side : AllBoxSides)
        {
          if (sides.Edges.At(side).ShouldRender())
          {
            auto sideRect = CalculateSideRect(outerBorderRect, sides.Edges, side);
            path.AddRect(FloatRect(sideRect)); // FIXME: Need pixel snapping here.
          }
        }

        graphicsContext.SetFillRule(WindRule::NonZero);
        graphicsContext.SetFillColor(sides.Edges.At(*firstVisibleSide).GetColor());
        graphicsContext.FillPath(path);
        return;
      }
    }

    bool clipToOuterBorder = !sides.OuterEdgeIsRectangular;
    GraphicsContextStateSaver stateSaver(graphicsContext, clipToOuterBorder && !sides.AppliedClipAlready);
    if (clipToOuterBorder)
    {
      // Clip to the inner and outer radii rects.
      if (sides.BleedAvoidance != BleedAvoidance::UseTransparencyLayer)
      {
        borderShape.ClipToOuterShape(graphicsContext, deviceScaleFactor);
      }
      borderShape.ClipOutInnerShape(graphicsContext, deviceScaleFactor);
    }

    // If only one edge visible antialiasing doesn't create seams
    bool antialias = ShouldAntialiasLines(graphicsContext) || numEdgesVisible == 1;
    if (haveAlphaColor)
    {
      PaintTranslucentBorderSides(borderShape, sides, edgesToDraw, antialias);
    }
    else
    {
      PaintBorderSides(borderShape, sides, edgesToDraw, antialias);
    }
  }

  template <typename T>
  bool BorderPainter::PaintNinePieceImageImpl(const LayoutRect &rect, const RenderStyle &style,
                                              const T &ninePieceImage, CompositeOperator op) const noexcept
  {
    auto image = ninePieceImage.Source.TryStyleImage();
    if (!image)
    {
      return false;
    }

    if (!image->IsLoaded(&_renderer))
    {
      return true; // Never paint a nine-piece image incrementally, but don't paint the fallback borders
                   // either.
    }

    if (!image->CanRender(&_renderer, style.UsedZoom()))
    {
      return false;
    }

    const auto *modelObject = DynamicDowncast<RenderBoxModelObject>(_renderer);
    if (!modelObject)
    {
      return false;
    }

    // FIXME: HDR content and the dynamic range limit come with bitmap images.
    ImagePaintingOptions options = {op, ImageOrientation::Orientation::FromImage};

    // FIXME: border-image is broken with full page zooming when tiling has to happen, since the tiling
    // function doesn't have any understanding of the zoom that is in effect on the tile.
    float deviceScaleFactor = Document().DeviceScaleFactor();

    LayoutRect rectWithOutsets = rect;
    rectWithOutsets.Expand(style.ImageOutsets(ninePieceImage));
    LayoutRect destination = LayoutRect(SnapRectToDevicePixels(rectWithOutsets, deviceScaleFactor));

    auto source = modelObject->CalculateImageIntrinsicDimensions(image.get(), destination.Size(),
                                                                 RenderBoxModelObject::ScaleByUsedZoom::No);

    // If both values are ‘auto’ then the intrinsic width and/or height of the image should be used, if any.
    image->SetContainerContextForRenderer(_renderer, FloatSize(source), style.UsedZoom());

    NinePieceImagePainter::Paint(ninePieceImage, _paintInfo.Context(), &_renderer, style, destination, source,
                                 deviceScaleFactor, options);
    return true;
  }

  bool BorderPainter::PaintNinePieceImage(const LayoutRect &rect, const RenderStyle &style,
                                          const style::BorderImage &borderImage,
                                          CompositeOperator op) const noexcept
  {
    return PaintNinePieceImageImpl(rect, style, borderImage, op);
  }

  bool BorderPainter::PaintNinePieceImage(const LayoutRect &rect, const RenderStyle &style,
                                          const style::MaskBorder &maskBorder,
                                          CompositeOperator op) const noexcept
  {
    return PaintNinePieceImageImpl(rect, style, maskBorder, op);
  }

  void BorderPainter::PaintTranslucentBorderSides(const BorderShape &borderShape, const Sides &sides,
                                                  EnumSet<BoxSide> edgesToDraw, bool antialias) const noexcept
  {
    // willBeOverdrawn assumes that we draw in order: top, bottom, left, right.
    // This is different from BoxSide enum order.
    static constexpr Array<BoxSide, 4> paintOrderSides = {BoxSide::Top, BoxSide::Bottom, BoxSide::Left,
                                                          BoxSide::Right};

    while (!edgesToDraw.IsEmpty())
    {
      // Find undrawn edges sharing a color.
      Color commonColor;

      EnumSet<BoxSide> commonColorEdgeSet;
      for (auto side : paintOrderSides)
      {
        if (!edgesToDraw.Contains(side))
        {
          continue;
        }

        const auto &edge = sides.Edges.At(side);
        bool includeEdge;
        if (commonColorEdgeSet.IsEmpty())
        {
          commonColor = edge.GetColor();
          includeEdge = true;
        }
        else
        {
          includeEdge = EqualIgnoringSemanticColor(edge.GetColor(), commonColor);
        }

        if (includeEdge)
        {
          commonColorEdgeSet.Add(side);
        }
      }

      bool useTransparencyLayer = IncludesAdjacentEdges(commonColorEdgeSet) && !commonColor.IsOpaque();
      if (useTransparencyLayer)
      {
        _paintInfo.Context().BeginTransparencyLayer(commonColor.AlphaAsFloat());
        commonColor = commonColor.OpaqueColor();
      }

      PaintBorderSides(borderShape, sides, commonColorEdgeSet, antialias, &commonColor);

      if (useTransparencyLayer)
      {
        _paintInfo.Context().EndTransparencyLayer();
      }

      edgesToDraw.Remove(commonColorEdgeSet);
    }
  }

  namespace
  {
    // BorderStyle::Outset darkens the bottom and right (and maybe lightens the top and left)
    // BorderStyle::Inset darkens the top and left (and maybe lightens the bottom and right)
    KRYS_NODISCARD bool BorderStyleHasUnmatchedColorsAtCorner(BorderStyle style, BoxSide side,
                                                              BoxSide adjacentSide) noexcept
    {
      // These styles match at the top/left and bottom/right.
      if (style == BorderStyle::Inset || style == BorderStyle::Groove || style == BorderStyle::Ridge
          || style == BorderStyle::Outset)
      {
        EnumSet<BoxSide> topRightSides = {BoxSide::Top, BoxSide::Right};
        EnumSet<BoxSide> bottomLeftSides = {BoxSide::Bottom, BoxSide::Left};

        EnumSet<BoxSide> usedSides {side, adjacentSide};
        return usedSides == topRightSides || usedSides == bottomLeftSides;
      }
      return false;
    }

    KRYS_NODISCARD bool ColorsMatchAtCorner(BoxSide side, BoxSide adjacentSide,
                                            const BorderEdges &edges) noexcept
    {
      const auto &edge = edges.At(side);
      const auto &adjacentEdge = edges.At(adjacentSide);

      if (edge.ShouldRender() != adjacentEdge.ShouldRender())
      {
        return false;
      }

      if (!EdgesShareColor(edge, adjacentEdge))
      {
        return false;
      }

      return !BorderStyleHasUnmatchedColorsAtCorner(edge.Style(), side, adjacentSide);
    }

    KRYS_NODISCARD bool ColorNeedsAntiAliasAtCorner(BoxSide side, BoxSide adjacentSide,
                                                    const BorderEdges &edges) noexcept
    {
      const auto &edge = edges.At(side);
      const auto &adjacentEdge = edges.At(adjacentSide);

      if (edge.GetColor().IsOpaque())
      {
        return false;
      }

      if (edge.ShouldRender() != adjacentEdge.ShouldRender())
      {
        return false;
      }

      if (!EdgesShareColor(edge, adjacentEdge))
      {
        return true;
      }

      return BorderStyleHasUnmatchedColorsAtCorner(edge.Style(), side, adjacentSide);
    }

    // This assumes that we draw in order: top, bottom, left, right.
    KRYS_NODISCARD bool WillBeOverdrawn(BoxSide side, BoxSide adjacentSide, const BorderEdges &edges) noexcept
    {
      switch (side)
      {
        case BoxSide::Top:
        case BoxSide::Bottom:
        {
          const auto &edge = edges.At(side);
          const auto &adjacentEdge = edges.At(adjacentSide);

          if (adjacentEdge.PresentButInvisible())
          {
            return false;
          }

          if (!EdgesShareColor(edge, adjacentEdge) && !adjacentEdge.GetColor().IsOpaque())
          {
            return false;
          }

          if (!BorderStyleFillsBorderArea(adjacentEdge.Style()))
          {
            return false;
          }

          return true;
        }
        case BoxSide::Left:
        case BoxSide::Right:
          // These draw last, so are never overdrawn.
          return false;
      }
      return false;
    }

    KRYS_NODISCARD bool BorderStylesRequireMitre(BoxSide side, BoxSide adjacentSide, BorderStyle style,
                                                 BorderStyle adjacentStyle) noexcept
    {
      if (style == BorderStyle::Double || adjacentStyle == BorderStyle::Double
          || adjacentStyle == BorderStyle::Groove || adjacentStyle == BorderStyle::Ridge)
      {
        return true;
      }

      if (BorderStyleIsDottedOrDashed(style) != BorderStyleIsDottedOrDashed(adjacentStyle))
      {
        return true;
      }

      if (style != adjacentStyle)
      {
        return true;
      }

      return BorderStyleHasUnmatchedColorsAtCorner(style, side, adjacentSide);
    }

    KRYS_NODISCARD bool JoinRequiresMitre(BoxSide side, BoxSide adjacentSide, const BorderEdges &edges,
                                          bool allowOverdraw) noexcept
    {
      const auto &edge = edges.At(side);
      const auto &adjacentEdge = edges.At(adjacentSide);

      if ((edge.IsTransparent() && adjacentEdge.IsTransparent()) || !adjacentEdge.IsPresent())
      {
        return false;
      }

      if (allowOverdraw && WillBeOverdrawn(side, adjacentSide, edges))
      {
        return false;
      }

      if (!EdgesShareColor(edge, adjacentEdge))
      {
        return true;
      }

      if (BorderStylesRequireMitre(side, adjacentSide, edge.Style(), adjacentEdge.Style()))
      {
        return true;
      }

      return false;
    }
  }

  void BorderPainter::PaintBorderSides(const BorderShape &borderShape, const Sides &sides,
                                       EnumSet<BoxSide> edgeSet, bool antialias,
                                       const Color *overrideColor) const noexcept
  {
    Path roundedPath;
    if (!sides.OuterEdgeIsRectangular)
    {
      float deviceScaleFactor = Document().DeviceScaleFactor();
      roundedPath = borderShape.PathForOuterShape(deviceScaleFactor);
    }

    auto innerEdgeRadii = borderShape.InnerEdgeRadii();

    // The inner border adjustment for bleed avoidance mode BleedAvoidance::BackgroundOverBorder
    // is only applied to sideRect, which is okay since BleedAvoidance::BackgroundOverBorder
    // is only to be used for solid borders and the shape of the border painted by drawBoxSideFromPath
    // only depends on sideRect when painting solid borders.

    auto paintOneSide = [&](BoxSide side, BoxSide adjacentSide1, BoxSide adjacentSide2)
    {
      const auto &edge = sides.Edges.At(side);
      if (!edge.ShouldRender() || !edgeSet.Contains(side))
      {
        return;
      }

      LayoutRect sideRect = borderShape.BorderRect();
      LayoutSize firstRadius;
      LayoutSize secondRadius;

      switch (side)
      {
        case BoxSide::Top:
          sideRect.SetHeight(borderShape.BorderWidths().Top());
          firstRadius = innerEdgeRadii.TopLeft();
          secondRadius = innerEdgeRadii.TopRight();
          break;
        case BoxSide::Right:
          sideRect.ShiftXEdgeTo(sideRect.MaxX() - borderShape.BorderWidths().Right());
          firstRadius = innerEdgeRadii.BottomRight();
          secondRadius = innerEdgeRadii.TopRight();
          break;
        case BoxSide::Bottom:
          sideRect.ShiftYEdgeTo(sideRect.MaxY() - borderShape.BorderWidths().Bottom());
          firstRadius = innerEdgeRadii.BottomLeft();
          secondRadius = innerEdgeRadii.BottomRight();
          break;
        case BoxSide::Left:
          sideRect.SetWidth(borderShape.BorderWidths().Left());
          firstRadius = innerEdgeRadii.BottomLeft();
          secondRadius = innerEdgeRadii.TopLeft();
          break;
      }

      auto borderWillArcInnerEdge = [](LayoutSize firstRadius, LayoutSize secondRadius)
      {
        return !firstRadius.IsEmpty() || !secondRadius.IsEmpty();
      };

      bool usePath =
        !sides.OuterEdgeIsRectangular
        && (BorderStyleHasInnerDetail(edge.Style()) || borderWillArcInnerEdge(firstRadius, secondRadius));
      PaintOneBorderSide(borderShape, sides, sideRect, side, adjacentSide1, adjacentSide2,
                         usePath ? &roundedPath : nullptr, antialias, overrideColor);
    };

    paintOneSide(BoxSide::Top, BoxSide::Left, BoxSide::Right);
    paintOneSide(BoxSide::Bottom, BoxSide::Left, BoxSide::Right);
    paintOneSide(BoxSide::Left, BoxSide::Top, BoxSide::Bottom);
    paintOneSide(BoxSide::Right, BoxSide::Top, BoxSide::Bottom);
  }

  void BorderPainter::PaintOneBorderSide(const BorderShape &borderShape, const Sides &sides,
                                         const LayoutRect &sideRect, BoxSide side, BoxSide adjacentSide1,
                                         BoxSide adjacentSide2, const Path *path, bool antialias,
                                         const Color *overrideColor) const noexcept
  {
    const auto &edgeToRender = sides.Edges.At(side);
    krys_debug_assert(edgeToRender.WidthForPainting());
    const auto &adjacentEdge1 = sides.Edges.At(adjacentSide1);
    const auto &adjacentEdge2 = sides.Edges.At(adjacentSide2);

    bool mitreAdjacentSide1 = JoinRequiresMitre(side, adjacentSide1, sides.Edges, !antialias);
    bool mitreAdjacentSide2 = JoinRequiresMitre(side, adjacentSide2, sides.Edges, !antialias);

    bool adjacentSide1StylesMatch = ColorsMatchAtCorner(side, adjacentSide1, sides.Edges);
    bool adjacentSide2StylesMatch = ColorsMatchAtCorner(side, adjacentSide2, sides.Edges);

    const Color &colorToPaint = overrideColor ? *overrideColor : edgeToRender.GetColor();

    auto &graphicsContext = _paintInfo.Context();

    if (path)
    {
      GraphicsContextStateSaver stateSaver(graphicsContext);

      ClipBorderSidePolygon(borderShape, side, adjacentSide1StylesMatch, adjacentSide2StylesMatch);

      float thickness = std::max(std::max(edgeToRender.WidthForPainting(), adjacentEdge1.WidthForPainting()),
                                 adjacentEdge2.WidthForPainting());
      DrawBoxSideFromPath(borderShape, *path, sides.Edges, edgeToRender.WidthForPainting(), thickness, side,
                          colorToPaint, edgeToRender.Style(), sides.BleedAvoidance);
    }
    else
    {
      bool clipForStyle =
        StyleRequiresClipPolygon(edgeToRender.Style()) && (mitreAdjacentSide1 || mitreAdjacentSide2);
      bool clipAdjacentSide1 =
        ColorNeedsAntiAliasAtCorner(side, adjacentSide1, sides.Edges) && mitreAdjacentSide1;
      bool clipAdjacentSide2 =
        ColorNeedsAntiAliasAtCorner(side, adjacentSide2, sides.Edges) && mitreAdjacentSide2;
      bool shouldClip = clipForStyle || clipAdjacentSide1 || clipAdjacentSide2;

      GraphicsContextStateSaver clipStateSaver(graphicsContext, shouldClip);
      if (shouldClip)
      {
        bool aliasAdjacentSide1 = clipAdjacentSide1 || (clipForStyle && mitreAdjacentSide1);
        bool aliasAdjacentSide2 = clipAdjacentSide2 || (clipForStyle && mitreAdjacentSide2);
        ClipBorderSidePolygon(borderShape, side, !aliasAdjacentSide1, !aliasAdjacentSide2);
        // Since we clipped, no need to draw with a mitre.
        mitreAdjacentSide1 = false;
        mitreAdjacentSide2 = false;
      }
      DrawLineForBoxSide(graphicsContext, Document(), FloatRect(sideRect), side, colorToPaint,
                         edgeToRender.Style(), mitreAdjacentSide1 ? adjacentEdge1.WidthForPainting() : 0,
                         mitreAdjacentSide2 ? adjacentEdge2.WidthForPainting() : 0, antialias);
    }
  }

  void BorderPainter::DrawBoxSideFromPath(const BorderShape &borderShape, const Path &borderPath,
                                          const BorderEdges &edges, float thickness, float drawThickness,
                                          BoxSide side, Color color, BorderStyle borderStyle,
                                          BleedAvoidance bleedAvoidance) const noexcept
  {
    if (thickness <= 0)
    {
      return;
    }

    auto &graphicsContext = _paintInfo.Context();

    if (borderStyle == BorderStyle::Double && thickness < 3)
    {
      borderStyle = BorderStyle::Solid;
    }

    switch (borderStyle)
    {
      case BorderStyle::None:
      case BorderStyle::Hidden: return;
      case BorderStyle::Dotted:
      case BorderStyle::Dashed:
      {
        graphicsContext.SetStrokeColor(color);

        // The stroke is doubled here because the provided path is the
        // outside edge of the border so half the stroke is clipped off.
        // The extra multiplier is so that the clipping mask can antialias
        // the edges to prevent jaggies.
        graphicsContext.SetStrokeThickness(drawThickness * 2 * 1.1f);
        graphicsContext.SetStrokeStyle(borderStyle == BorderStyle::Dashed ? StrokeStyle::DashedStroke
                                                                          : StrokeStyle::DottedStroke);

        // If the number of dashes that fit in the path is odd and non-integral then we
        // will have an awkwardly-sized dash at the end of the path. To try to avoid that
        // here, we simply make the whitespace dashes ever so slightly bigger.
        // FIXME: This could be even better if we tried to manipulate the dash offset
        // and possibly the gapLength to get the corners dash-symmetrical.
        float dashLength = thickness * ((borderStyle == BorderStyle::Dashed) ? 3.0f : 1.0f);
        float gapLength = dashLength;
        float numberOfDashes = borderPath.Length() / dashLength;
        // Don't try to show dashes if we have less than 2 dashes + 2 gaps.
        // FIXME: should do this test per side.
        if (numberOfDashes >= 4)
        {
          bool evenNumberOfFullDashes = !(static_cast<int32>(numberOfDashes) % 2);
          bool integralNumberOfDashes =
            !(numberOfDashes - static_cast<float>(static_cast<int32>(numberOfDashes)));
          if (!evenNumberOfFullDashes && !integralNumberOfDashes)
          {
            float numberOfGaps = numberOfDashes / 2;
            gapLength += (dashLength / numberOfGaps);
          }

          graphicsContext.SetLineDash(DashArray {dashLength, gapLength}, dashLength);
        }

        // FIXME: stroking the border path causes issues with tight corners:
        // https://bugs.webkit.org/show_bug.cgi?id=58711
        // Also, to get the best appearance we should stroke a path between the two borders.
        graphicsContext.StrokePath(borderPath);
        return;
      }
      case BorderStyle::Double:
      {
        RectEdges<LayoutUnit> outerThirdInsets;
        RectEdges<LayoutUnit> innerThirdInsets;

        edges.At(BoxSide::Top).GetDoubleBorderStripeWidths(outerThirdInsets.Top(), innerThirdInsets.Top());
        edges.At(BoxSide::Right)
          .GetDoubleBorderStripeWidths(outerThirdInsets.Right(), innerThirdInsets.Right());
        edges.At(BoxSide::Bottom)
          .GetDoubleBorderStripeWidths(outerThirdInsets.Bottom(), innerThirdInsets.Bottom());
        edges.At(BoxSide::Left).GetDoubleBorderStripeWidths(outerThirdInsets.Left(), innerThirdInsets.Left());

        // Draw inner border line
        {
          GraphicsContextStateSaver stateSaver(graphicsContext);

          auto innerThirdShape = borderShape.ShapeWithBorderWidths(innerThirdInsets);
          // FIXME: Cache document().deviceScaleFactor().
          innerThirdShape.ClipToInnerShape(graphicsContext, Document().DeviceScaleFactor());

          DrawBoxSideFromPath(borderShape, borderPath, edges, thickness, drawThickness, side, color,
                              BorderStyle::Solid, bleedAvoidance);
        }

        // Draw outer border line
        {
          auto outerThirdShape = borderShape.ShapeWithBorderWidths(outerThirdInsets);
          outerThirdShape.ClipOutInnerShape(graphicsContext, Document().DeviceScaleFactor());

          DrawBoxSideFromPath(borderShape, borderPath, edges, thickness, drawThickness, side, color,
                              BorderStyle::Solid, bleedAvoidance);
        }
        return;
      }
      case BorderStyle::Ridge:
      case BorderStyle::Groove:
      {
        BorderStyle s1;
        BorderStyle s2;
        if (borderStyle == BorderStyle::Groove)
        {
          s1 = BorderStyle::Inset;
          s2 = BorderStyle::Outset;
        }
        else
        {
          s1 = BorderStyle::Outset;
          s2 = BorderStyle::Inset;
        }

        // Paint full border
        DrawBoxSideFromPath(borderShape, borderPath, edges, thickness, drawThickness, side, color, s1,
                            bleedAvoidance);

        // Paint inner only
        GraphicsContextStateSaver stateSaver(graphicsContext);

        RectEdges<LayoutUnit> midWidths = {
          LayoutUnit {edges.Top().WidthForPainting() / 2},
          LayoutUnit {edges.Right().WidthForPainting() / 2},
          LayoutUnit {edges.Bottom().WidthForPainting() / 2},
          LayoutUnit {edges.Left().WidthForPainting() / 2},
        };

        auto midBorderShape = borderShape.ShapeWithBorderWidths(midWidths);
        midBorderShape.ClipToInnerShape(graphicsContext, Document().DeviceScaleFactor());

        DrawBoxSideFromPath(borderShape, borderPath, edges, thickness, drawThickness, side, color, s2,
                            bleedAvoidance);
        return;
      }
      case BorderStyle::Inset:
      case BorderStyle::Outset: color = CalculateBorderStyleColor(borderStyle, side, color); break;
      default:                  break;
    }

    graphicsContext.SetStrokeStyle(StrokeStyle::NoStroke);
    graphicsContext.SetFillColor(color);

    auto borderRect = borderShape.SnappedOuterRect(Document().DeviceScaleFactor());
    graphicsContext.DrawRect(borderRect);
  }

  void BorderPainter::ClipBorderSidePolygon(const BorderShape &borderShape, BoxSide side,
                                            bool firstEdgeMatches, bool secondEdgeMatches) const noexcept
  {
    auto &graphicsContext = _paintInfo.Context();

    float deviceScaleFactor = Document().DeviceScaleFactor();
    auto outerRect = borderShape.SnappedOuterRect(deviceScaleFactor);

    auto innerRect = borderShape.SnappedInnerRect(deviceScaleFactor);
    auto innerBorder = borderShape.DeprecatedInnerRoundedRect();

    // For each side, create a quad that encompasses all parts of that side that may draw,
    // including areas inside the innerBorder.
    //
    //         0----------------3
    //       0  \              /  0
    //       |\  1----------- 2  /|
    //       | 1                1 |
    //       | |                | |
    //       | |                | |
    //       | 2                2 |
    //       |/  1------------2  \|
    //       3  /              \  3
    //         0----------------3
    //
    List<FloatPoint> quad;
    switch (side)
    {
      case BoxSide::Top:
        quad = {outerRect.MinXMinYCorner(), innerRect.MinXMinYCorner(), innerRect.MaxXMinYCorner(),
                outerRect.MaxXMinYCorner()};

        if (!innerBorder.GetRadii().TopLeft().IsZero())
        {
          (void)FindIntersection(outerRect.MinXMinYCorner(), innerRect.MinXMinYCorner(),
                                 innerRect.MinXMaxYCorner(), innerRect.MaxXMinYCorner(), quad[1]);
        }

        if (!innerBorder.GetRadii().TopRight().IsZero())
        {
          (void)FindIntersection(outerRect.MaxXMinYCorner(), innerRect.MaxXMinYCorner(),
                                 innerRect.MinXMinYCorner(), innerRect.MaxXMaxYCorner(), quad[2]);
        }
        break;

      case BoxSide::Left:
        quad = {outerRect.MinXMinYCorner(), innerRect.MinXMinYCorner(), innerRect.MinXMaxYCorner(),
                outerRect.MinXMaxYCorner()};

        if (!innerBorder.GetRadii().TopLeft().IsZero())
        {
          (void)FindIntersection(outerRect.MinXMinYCorner(), innerRect.MinXMinYCorner(),
                                 innerRect.MinXMaxYCorner(), innerRect.MaxXMinYCorner(), quad[1]);
        }

        if (!innerBorder.GetRadii().BottomLeft().IsZero())
        {
          (void)FindIntersection(outerRect.MinXMaxYCorner(), innerRect.MinXMaxYCorner(),
                                 innerRect.MinXMinYCorner(), innerRect.MaxXMaxYCorner(), quad[2]);
        }
        break;

      case BoxSide::Bottom:
        quad = {outerRect.MinXMaxYCorner(), innerRect.MinXMaxYCorner(), innerRect.MaxXMaxYCorner(),
                outerRect.MaxXMaxYCorner()};

        if (!innerBorder.GetRadii().BottomLeft().IsZero())
        {
          (void)FindIntersection(outerRect.MinXMaxYCorner(), innerRect.MinXMaxYCorner(),
                                 innerRect.MinXMinYCorner(), innerRect.MaxXMaxYCorner(), quad[1]);
        }

        if (!innerBorder.GetRadii().BottomRight().IsZero())
        {
          (void)FindIntersection(outerRect.MaxXMaxYCorner(), innerRect.MaxXMaxYCorner(),
                                 innerRect.MaxXMinYCorner(), innerRect.MinXMaxYCorner(), quad[2]);
        }
        break;

      case BoxSide::Right:
        quad = {outerRect.MaxXMinYCorner(), innerRect.MaxXMinYCorner(), innerRect.MaxXMaxYCorner(),
                outerRect.MaxXMaxYCorner()};

        if (!innerBorder.GetRadii().TopRight().IsZero())
        {
          (void)FindIntersection(outerRect.MaxXMinYCorner(), innerRect.MaxXMinYCorner(),
                                 innerRect.MinXMinYCorner(), innerRect.MaxXMaxYCorner(), quad[1]);
        }

        if (!innerBorder.GetRadii().BottomRight().IsZero())
        {
          (void)FindIntersection(outerRect.MaxXMaxYCorner(), innerRect.MaxXMaxYCorner(),
                                 innerRect.MaxXMinYCorner(), innerRect.MinXMaxYCorner(), quad[2]);
        }
        break;
    }

    // If the border matches both of its adjacent sides, don't anti-alias the clip, and
    // if neither side matches, anti-alias the clip.
    if (firstEdgeMatches == secondEdgeMatches)
    {
      bool wasAntialiased = graphicsContext.ShouldAntialias();
      graphicsContext.SetShouldAntialias(!firstEdgeMatches);
      graphicsContext.ClipPath(Path(quad), WindRule::NonZero);
      graphicsContext.SetShouldAntialias(wasAntialiased);
      return;
    }

    // Square off the end which shouldn't be affected by antialiasing, and clip.
    List<FloatPoint> firstQuad = {quad[0], quad[1], quad[2],
                                  side == BoxSide::Top || side == BoxSide::Bottom
                                    ? FloatPoint(quad[3].X(), quad[2].Y())
                                    : FloatPoint(quad[2].X(), quad[3].Y()),
                                  quad[3]};
    bool wasAntialiased = graphicsContext.ShouldAntialias();
    graphicsContext.SetShouldAntialias(!firstEdgeMatches);
    graphicsContext.ClipPath(Path(firstQuad), WindRule::NonZero);

    List<FloatPoint> secondQuad = {quad[0],
                                   side == BoxSide::Top || side == BoxSide::Bottom
                                     ? FloatPoint(quad[0].X(), quad[1].Y())
                                     : FloatPoint(quad[1].X(), quad[0].Y()),
                                   quad[1], quad[2], quad[3]};
    // Antialiasing affects the second side.
    graphicsContext.SetShouldAntialias(!secondEdgeMatches);
    graphicsContext.ClipPath(Path(secondQuad), WindRule::NonZero);

    graphicsContext.SetShouldAntialias(wasAntialiased);
  }

  void BorderPainter::DrawLineForBoxSide(GraphicsContext &graphicsContext, const dom::Document &document,
                                         const FloatRect &rect, BoxSide side, Color color,
                                         BorderStyle borderStyle, float adjacentWidth1, float adjacentWidth2,
                                         bool antialias) noexcept
  {
    auto drawBorderRect = [&graphicsContext](const FloatRect &rect)
    {
      if (rect.IsEmpty())
      {
        return;
      }
      graphicsContext.DrawRect(rect);
    };

    auto drawLineFor = [&graphicsContext, &document, color, antialias](const FloatRect &rect, BoxSide side,
                                                                       BorderStyle borderStyle,
                                                                       const FloatSize &adjacent)
    {
      if (rect.IsEmpty())
      {
        return;
      }
      DrawLineForBoxSide(graphicsContext, document, rect, side, color, borderStyle, adjacent.Width(),
                         adjacent.Height(), antialias);
    };

    float x1 = rect.X();
    float x2 = rect.MaxX();
    float y1 = rect.Y();
    float y2 = rect.MaxY();
    float thickness;
    float length;
    if (side == BoxSide::Top || side == BoxSide::Bottom)
    {
      thickness = y2 - y1;
      length = x2 - x1;
    }
    else
    {
      thickness = x2 - x1;
      length = y2 - y1;
    }
    // FIXME: We really would like this check to be an ASSERT as we don't want to draw empty borders. However
    // nothing guarantees that the following recursive calls to drawLineForBoxSide will have non-null
    // dimensions.
    if (!thickness || !length)
    {
      return;
    }

    float deviceScaleFactor = document.DeviceScaleFactor();
    if (borderStyle == BorderStyle::Double && (thickness * deviceScaleFactor) < 3)
    {
      borderStyle = BorderStyle::Solid;
    }

    switch (borderStyle)
    {
      case BorderStyle::None:
      case BorderStyle::Hidden: return;
      case BorderStyle::Dotted:
      case BorderStyle::Dashed:
      {
        bool wasAntialiased = graphicsContext.ShouldAntialias();
        StrokeStyle oldStrokeStyle = graphicsContext.GetStrokeStyle();
        graphicsContext.SetShouldAntialias(antialias);
        graphicsContext.SetStrokeColor(color);
        graphicsContext.SetStrokeThickness(thickness);
        graphicsContext.SetStrokeStyle(borderStyle == BorderStyle::Dashed ? StrokeStyle::DashedStroke
                                                                          : StrokeStyle::DottedStroke);
        graphicsContext.DrawLine(RoundPointToDevicePixels(LayoutPoint(x1, y1), deviceScaleFactor),
                                 RoundPointToDevicePixels(LayoutPoint(x2, y2), deviceScaleFactor));
        graphicsContext.SetShouldAntialias(wasAntialiased);
        graphicsContext.SetStrokeStyle(oldStrokeStyle);
        break;
      }
      case BorderStyle::Double:
      {
        float thirdOfThickness = CeilToDevicePixel(thickness / 3, deviceScaleFactor);
        krys_debug_assert(thirdOfThickness);

        if (!adjacentWidth1 && !adjacentWidth2)
        {
          StrokeStyle oldStrokeStyle = graphicsContext.GetStrokeStyle();
          graphicsContext.SetStrokeStyle(StrokeStyle::NoStroke);
          graphicsContext.SetFillColor(color);

          bool wasAntialiased = graphicsContext.ShouldAntialias();
          graphicsContext.SetShouldAntialias(antialias);

          switch (side)
          {
            case BoxSide::Top:
            case BoxSide::Bottom:
              drawBorderRect(
                SnapRectToDevicePixels(LayoutRect(x1, y1, length, thirdOfThickness), deviceScaleFactor));
              drawBorderRect(SnapRectToDevicePixels(
                LayoutRect(x1, y2 - thirdOfThickness, length, thirdOfThickness), deviceScaleFactor));
              break;
            case BoxSide::Left:
            case BoxSide::Right:
              drawBorderRect(
                SnapRectToDevicePixels(LayoutRect(x1, y1, thirdOfThickness, length), deviceScaleFactor));
              drawBorderRect(SnapRectToDevicePixels(
                LayoutRect(x2 - thirdOfThickness, y1, thirdOfThickness, length), deviceScaleFactor));
              break;
          }

          graphicsContext.SetShouldAntialias(wasAntialiased);
          graphicsContext.SetStrokeStyle(oldStrokeStyle);
        }
        else
        {
          float adjacent1BigThird = CeilToDevicePixel(adjacentWidth1 / 3, deviceScaleFactor);
          float adjacent2BigThird = CeilToDevicePixel(adjacentWidth2 / 3, deviceScaleFactor);

          float offset1 = FloorToDevicePixel(std::abs(adjacentWidth1) * 2 / 3, deviceScaleFactor);
          float offset2 = FloorToDevicePixel(std::abs(adjacentWidth2) * 2 / 3, deviceScaleFactor);

          float mitreOffset1 = adjacentWidth1 < 0 ? offset1 : 0;
          float mitreOffset2 = adjacentWidth1 > 0 ? offset1 : 0;
          float mitreOffset3 = adjacentWidth2 < 0 ? offset2 : 0;
          float mitreOffset4 = adjacentWidth2 > 0 ? offset2 : 0;

          FloatRect paintBorderRect;
          switch (side)
          {
            case BoxSide::Top:
              paintBorderRect = SnapRectToDevicePixels(LayoutRect(x1 + mitreOffset1, y1,
                                                                  (x2 - mitreOffset3) - (x1 + mitreOffset1),
                                                                  thirdOfThickness),
                                                       deviceScaleFactor);
              drawLineFor(paintBorderRect, side, BorderStyle::Solid,
                          FloatSize(adjacent1BigThird, adjacent2BigThird));

              paintBorderRect = SnapRectToDevicePixels(LayoutRect(x1 + mitreOffset2, y2 - thirdOfThickness,
                                                                  (x2 - mitreOffset4) - (x1 + mitreOffset2),
                                                                  thirdOfThickness),
                                                       deviceScaleFactor);
              drawLineFor(paintBorderRect, side, BorderStyle::Solid,
                          FloatSize(adjacent1BigThird, adjacent2BigThird));
              break;
            case BoxSide::Left:
              paintBorderRect = SnapRectToDevicePixels(LayoutRect(x1, y1 + mitreOffset1, thirdOfThickness,
                                                                  (y2 - mitreOffset3) - (y1 + mitreOffset1)),
                                                       deviceScaleFactor);
              drawLineFor(paintBorderRect, side, BorderStyle::Solid,
                          FloatSize(adjacent1BigThird, adjacent2BigThird));

              paintBorderRect =
                SnapRectToDevicePixels(LayoutRect(x2 - thirdOfThickness, y1 + mitreOffset2, thirdOfThickness,
                                                  (y2 - mitreOffset4) - (y1 + mitreOffset2)),
                                       deviceScaleFactor);
              drawLineFor(paintBorderRect, side, BorderStyle::Solid,
                          FloatSize(adjacent1BigThird, adjacent2BigThird));
              break;
            case BoxSide::Bottom:
              paintBorderRect = SnapRectToDevicePixels(LayoutRect(x1 + mitreOffset2, y1,
                                                                  (x2 - mitreOffset4) - (x1 + mitreOffset2),
                                                                  thirdOfThickness),
                                                       deviceScaleFactor);
              drawLineFor(paintBorderRect, side, BorderStyle::Solid,
                          FloatSize(adjacent1BigThird, adjacent2BigThird));

              paintBorderRect = SnapRectToDevicePixels(LayoutRect(x1 + mitreOffset1, y2 - thirdOfThickness,
                                                                  (x2 - mitreOffset3) - (x1 + mitreOffset1),
                                                                  thirdOfThickness),
                                                       deviceScaleFactor);
              drawLineFor(paintBorderRect, side, BorderStyle::Solid,
                          FloatSize(adjacent1BigThird, adjacent2BigThird));
              break;
            case BoxSide::Right:
              paintBorderRect = SnapRectToDevicePixels(LayoutRect(x1, y1 + mitreOffset2, thirdOfThickness,
                                                                  (y2 - mitreOffset4) - (y1 + mitreOffset2)),
                                                       deviceScaleFactor);
              drawLineFor(paintBorderRect, side, BorderStyle::Solid,
                          FloatSize(adjacent1BigThird, adjacent2BigThird));

              paintBorderRect =
                SnapRectToDevicePixels(LayoutRect(x2 - thirdOfThickness, y1 + mitreOffset1, thirdOfThickness,
                                                  (y2 - mitreOffset3) - (y1 + mitreOffset1)),
                                       deviceScaleFactor);
              drawLineFor(paintBorderRect, side, BorderStyle::Solid,
                          FloatSize(adjacent1BigThird, adjacent2BigThird));
              break;
            default: break;
          }
        }
        break;
      }
      case BorderStyle::Ridge:
      case BorderStyle::Groove:
      {
        BorderStyle s1;
        BorderStyle s2;
        if (borderStyle == BorderStyle::Groove)
        {
          s1 = BorderStyle::Inset;
          s2 = BorderStyle::Outset;
        }
        else
        {
          s1 = BorderStyle::Outset;
          s2 = BorderStyle::Inset;
        }

        float adjacent1BigHalf = CeilToDevicePixel(adjacentWidth1 / 2, deviceScaleFactor);
        float adjacent2BigHalf = CeilToDevicePixel(adjacentWidth2 / 2, deviceScaleFactor);

        float adjacent1SmallHalf = FloorToDevicePixel(adjacentWidth1 / 2, deviceScaleFactor);
        float adjacent2SmallHalf = FloorToDevicePixel(adjacentWidth2 / 2, deviceScaleFactor);

        float offset1 = 0;
        float offset2 = 0;
        float offset3 = 0;
        float offset4 = 0;

        if (((side == BoxSide::Top || side == BoxSide::Left) && adjacentWidth1 < 0)
            || ((side == BoxSide::Bottom || side == BoxSide::Right) && adjacentWidth1 > 0))
        {
          offset1 = FloorToDevicePixel(adjacentWidth1 / 2, deviceScaleFactor);
        }

        if (((side == BoxSide::Top || side == BoxSide::Left) && adjacentWidth2 < 0)
            || ((side == BoxSide::Bottom || side == BoxSide::Right) && adjacentWidth2 > 0))
        {
          offset2 = CeilToDevicePixel(adjacentWidth2 / 2, deviceScaleFactor);
        }

        if (((side == BoxSide::Top || side == BoxSide::Left) && adjacentWidth1 > 0)
            || ((side == BoxSide::Bottom || side == BoxSide::Right) && adjacentWidth1 < 0))
        {
          offset3 = FloorToDevicePixel(std::abs(adjacentWidth1) / 2, deviceScaleFactor);
        }

        if (((side == BoxSide::Top || side == BoxSide::Left) && adjacentWidth2 > 0)
            || ((side == BoxSide::Bottom || side == BoxSide::Right) && adjacentWidth2 < 0))
        {
          offset4 = CeilToDevicePixel(adjacentWidth2 / 2, deviceScaleFactor);
        }

        float adjustedX = CeilToDevicePixel(std::midpoint(x1, x2), deviceScaleFactor);
        float adjustedY = CeilToDevicePixel(std::midpoint(y1, y2), deviceScaleFactor);
        // Quads can't use the default snapping rect functions.
        x1 = RoundToDevicePixel(x1, deviceScaleFactor);
        x2 = RoundToDevicePixel(x2, deviceScaleFactor);
        y1 = RoundToDevicePixel(y1, deviceScaleFactor);
        y2 = RoundToDevicePixel(y2, deviceScaleFactor);

        switch (side)
        {
          case BoxSide::Top:
            drawLineFor(FloatRect(FloatPoint(x1 + offset1, y1), FloatPoint(x2 - offset2, adjustedY)), side,
                        s1, FloatSize(adjacent1BigHalf, adjacent2BigHalf));
            drawLineFor(FloatRect(FloatPoint(x1 + offset3, adjustedY), FloatPoint(x2 - offset4, y2)), side,
                        s2, FloatSize(adjacent1SmallHalf, adjacent2SmallHalf));
            break;
          case BoxSide::Left:
            drawLineFor(FloatRect(FloatPoint(x1, y1 + offset1), FloatPoint(adjustedX, y2 - offset2)), side,
                        s1, FloatSize(adjacent1BigHalf, adjacent2BigHalf));
            drawLineFor(FloatRect(FloatPoint(adjustedX, y1 + offset3), FloatPoint(x2, y2 - offset4)), side,
                        s2, FloatSize(adjacent1SmallHalf, adjacent2SmallHalf));
            break;
          case BoxSide::Bottom:
            drawLineFor(FloatRect(FloatPoint(x1 + offset1, y1), FloatPoint(x2 - offset2, adjustedY)), side,
                        s2, FloatSize(adjacent1BigHalf, adjacent2BigHalf));
            drawLineFor(FloatRect(FloatPoint(x1 + offset3, adjustedY), FloatPoint(x2 - offset4, y2)), side,
                        s1, FloatSize(adjacent1SmallHalf, adjacent2SmallHalf));
            break;
          case BoxSide::Right:
            drawLineFor(FloatRect(FloatPoint(x1, y1 + offset1), FloatPoint(adjustedX, y2 - offset2)), side,
                        s2, FloatSize(adjacent1BigHalf, adjacent2BigHalf));
            drawLineFor(FloatRect(FloatPoint(adjustedX, y1 + offset3), FloatPoint(x2, y2 - offset4)), side,
                        s1, FloatSize(adjacent1SmallHalf, adjacent2SmallHalf));
            break;
        }
        break;
      }
      case BorderStyle::Inset:
      case BorderStyle::Outset: color = CalculateBorderStyleColor(borderStyle, side, color); [[fallthrough]];
      case BorderStyle::Solid:
      {
        StrokeStyle oldStrokeStyle = graphicsContext.GetStrokeStyle();
        krys_debug_assert(x2 >= x1);
        krys_debug_assert(y2 >= y1);
        if (!adjacentWidth1 && !adjacentWidth2)
        {
          graphicsContext.SetStrokeStyle(StrokeStyle::NoStroke);
          graphicsContext.SetFillColor(color);
          bool wasAntialiased = graphicsContext.ShouldAntialias();
          graphicsContext.SetShouldAntialias(antialias);
          drawBorderRect(SnapRectToDevicePixels(LayoutRect(x1, y1, x2 - x1, y2 - y1), deviceScaleFactor));
          graphicsContext.SetShouldAntialias(wasAntialiased);
          graphicsContext.SetStrokeStyle(oldStrokeStyle);
          return;
        }

        // FIXME: These roundings should be replaced by ASSERT(device pixel positioned) when all the callers
        // have transitioned to device pixels.
        x1 = RoundToDevicePixel(x1, deviceScaleFactor);
        y1 = RoundToDevicePixel(y1, deviceScaleFactor);
        x2 = RoundToDevicePixel(x2, deviceScaleFactor);
        y2 = RoundToDevicePixel(y2, deviceScaleFactor);

        List<FloatPoint> quad;
        switch (side)
        {
          case BoxSide::Top:
            quad = {{x1 + std::max<float>(-adjacentWidth1, 0), y1},
                    {x1 + std::max<float>(adjacentWidth1, 0), y2},
                    {x2 - std::max<float>(adjacentWidth2, 0), y2},
                    {x2 - std::max<float>(-adjacentWidth2, 0), y1}};
            break;
          case BoxSide::Bottom:
            quad = {{x1 + std::max<float>(adjacentWidth1, 0), y1},
                    {x1 + std::max<float>(-adjacentWidth1, 0), y2},
                    {x2 - std::max<float>(-adjacentWidth2, 0), y2},
                    {x2 - std::max<float>(adjacentWidth2, 0), y1}};
            break;
          case BoxSide::Left:
            quad = {{x1, y1 + std::max<float>(-adjacentWidth1, 0)},
                    {x1, y2 - std::max<float>(-adjacentWidth2, 0)},
                    {x2, y2 - std::max<float>(adjacentWidth2, 0)},
                    {x2, y1 + std::max<float>(adjacentWidth1, 0)}};
            break;
          case BoxSide::Right:
            quad = {{x1, y1 + std::max<float>(adjacentWidth1, 0)},
                    {x1, y2 - std::max<float>(adjacentWidth2, 0)},
                    {x2, y2 - std::max<float>(-adjacentWidth2, 0)},
                    {x2, y1 + std::max<float>(-adjacentWidth1, 0)}};
            break;
        }

        graphicsContext.SetStrokeStyle(StrokeStyle::NoStroke);
        graphicsContext.SetFillColor(color);
        bool wasAntialiased = graphicsContext.ShouldAntialias();
        graphicsContext.SetShouldAntialias(antialias);
        graphicsContext.FillPath(Path(quad));
        graphicsContext.SetShouldAntialias(wasAntialiased);

        graphicsContext.SetStrokeStyle(oldStrokeStyle);
        break;
      }
    }
  }

  LayoutRect BorderPainter::BorderRectAdjustedForBleedAvoidance(const LayoutRect &rect,
                                                                BleedAvoidance bleedAvoidance) const noexcept
  {
    if (bleedAvoidance != BleedAvoidance::BackgroundOverBorder)
    {
      return rect;
    }

    // We shrink the rectangle by one device pixel on each side to make it fully overlap the anti-aliased
    // background border
    return ShrinkRectByOneDevicePixel(_paintInfo.Context(), rect, Document().DeviceScaleFactor());
  }

  bool BorderPainter::ShouldAntialiasLines(GraphicsContext &context) noexcept
  {
    // FIXME: We may want to not antialias when scaled by an integral value,
    // and we may want to antialias when translated by a non-integral value.
    return !context.GetCTM().IsIdentityOrTranslationOrFlipped();
  }

  // This never changes the alpha of the color, so it's OK that callers don't check for
  // PaintBehavior::ForceBlackBorder.
  Color BorderPainter::CalculateBorderStyleColor(const BorderStyle &style, const BoxSide &side,
                                                 const Color &color) noexcept
  {
    krys_debug_assert(style == BorderStyle::Inset || style == BorderStyle::Outset);

    // This values were derived empirically.
    constexpr float baseDarkColorLuminance {0.014443844f}; // Luminance of SRGBA<uint8_t> { 32, 32, 32 }
    constexpr float baseLightColorLuminance {0.83077f};    // Luminance of SRGBA<uint8_t> { 235, 235, 235 }

    enum Operation
    {
      Darken,
      Lighten
    };

    Operation operation =
      (side == BoxSide::Top || side == BoxSide::Left) == (style == BorderStyle::Inset) ? Darken : Lighten;

    bool isVeryDarkColor = color.Luminance() <= baseDarkColorLuminance;
    bool isVeryLightColor = color.Luminance() > baseLightColorLuminance;

    // Special case very dark colors to give them extra contrast.
    if (isVeryDarkColor)
    {
      return operation == Darken ? color.Lightened() : color.Lightened().Lightened();
    }

    // Here we will darken the border decoration color when needed.
    if (operation == Darken)
    {
      return color.Darkened();
    }

    krys_debug_assert(operation == Lighten);

    return isVeryLightColor ? color : color.Lightened();
  }

  const dom::Document &BorderPainter::Document() const noexcept
  {
    return _renderer.GetDocument();
  }
}
