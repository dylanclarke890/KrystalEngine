#include "Krystal.Booey/Rendering/BackgroundPainter.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/Image.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/BorderShape.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/FillLayers.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include <cmath>
#include <ranges>

namespace krys::boo
{
  BackgroundImageGeometry::BackgroundImageGeometry(const LayoutRect &destinationRect,
                                                   const LayoutSize &tileSizeWithoutPixelSnapping,
                                                   const LayoutSize &tileSize, const LayoutSize &phase,
                                                   const LayoutSize &spaceSize, bool fixedAttachment) noexcept
      : DestinationRect(destinationRect), DestinationOrigin(destinationRect.Location()),
        TileSizeWithoutPixelSnapping(tileSizeWithoutPixelSnapping), TileSize(tileSize), Phase(phase),
        SpaceSize(spaceSize), HasNonLocalGeometry(fixedAttachment)
  {
  }

  BackgroundPainter::BackgroundPainter(RenderBoxModelObject &renderer, const PaintInfo &paintInfo) noexcept
      : _renderer(renderer), _paintInfo(paintInfo)
  {
    // background-clip has no effect when painting the root background.
    // https://www.w3.org/TR/css-backgrounds-3/#background-clip
    if (_renderer.IsDocumentElementRenderer())
    {
      SetOverrideClip(FillBox::BorderBox);
    }
  }

  void BackgroundPainter::PaintBackground(const LayoutRect &paintRect,
                                          BleedAvoidance bleedAvoidance) const noexcept
  {
    if (_renderer.IsDocumentElementRenderer())
    {
      PaintRootBoxFillLayers();
      return;
    }

    if (!PaintsOwnBackground(_renderer))
    {
      return;
    }

    if (auto *renderBox = DynamicDowncast<RenderBox>(_renderer))
    {
      if (renderBox->BackgroundIsKnownToBeObscured(paintRect.Location())
          && !BoxShadowShouldBeAppliedToBackground(*renderBox, paintRect.Location(), bleedAvoidance, {}))
      {
        return;
      }
    }

    auto backgroundColor =
      _renderer.Style().VisitedDependentColorWithColorFilter(css::PropertyId::BackgroundColor);
    auto compositeOp = Document().CompositeOperatorForBackgroundColor(backgroundColor, _renderer);

    PaintFillLayers(backgroundColor, _renderer.Style().GetBackgroundLayers(), paintRect, bleedAvoidance,
                    compositeOp);
  }

  void BackgroundPainter::PaintRootBoxFillLayers() const noexcept
  {
    krys_debug_assert(_renderer.IsDocumentElementRenderer());
    if (_paintInfo.SkipRootBackground())
    {
      return;
    }

    auto *rootBackgroundRenderer = View().RendererForRootBackground();
    if (!rootBackgroundRenderer)
    {
      return;
    }

    const auto &style = rootBackgroundRenderer->Style();
    auto backgroundColor = style.VisitedDependentColorWithColorFilter(css::PropertyId::BackgroundColor);
    auto compositeOp = Document().CompositeOperatorForBackgroundColor(backgroundColor, _renderer);

    PaintFillLayers(backgroundColor, style.GetBackgroundLayers(), View().BackgroundRect(),
                    BleedAvoidance::None, compositeOp, rootBackgroundRenderer);
  }

  bool BackgroundPainter::PaintsOwnBackground(const RenderBoxModelObject &renderer) noexcept
  {
    if (!renderer.IsBody())
    {
      return true;
    }

    if (renderer.ShouldApplyAnyContainment())
    {
      return true;
    }

    // The <body> only paints its background if the root element has defined a background independent of the
    // body, or if the <body>'s parent is not the document element's renderer (e.g. inside SVG foreignObject).
    auto *documentElementRenderer = renderer.GetDocument().DocumentElement()->Renderer();
    return !documentElementRenderer || documentElementRenderer->ShouldApplyAnyContainment()
           || documentElementRenderer->HasBackground() || documentElementRenderer != renderer.Parent();
  }

  void BackgroundPainter::PaintFillLayers(const Color &color, const style::BackgroundLayers &fillLayerList,
                                          const LayoutRect &rect, BleedAvoidance bleedAvoidance,
                                          CompositeOperator op,
                                          RenderElement *backgroundObject) const noexcept
  {
    PaintFillLayersImpl(color, fillLayerList, rect, bleedAvoidance, op, backgroundObject);
  }

  template <typename TLayers>
  void BackgroundPainter::PaintFillLayersImpl(const Color &color, const TLayers &fillLayers,
                                              const LayoutRect &rect, BleedAvoidance bleedAvoidance,
                                              CompositeOperator op,
                                              RenderElement *backgroundObject) const noexcept
  {
    using TLayer = typename TLayers::value_type;
    bool shouldDrawBackgroundInSeparateBuffer = false;

    style::ComputeClipMax(fillLayers);

    for (const auto &layer : fillLayers.UsedValues())
    {
      if (layer.GetBlendMode() != BlendMode::Normal)
      {
        shouldDrawBackgroundInSeparateBuffer = true;
      }

      // Stop traversal when an opaque layer is encountered.
      // FIXME: It would be possible for the following occlusion culling test to be more aggressive
      // on layers with no repeat by testing whether the image covers the layout rect.
      // Testing that here would imply duplicating a lot of calculations that are currently done in
      // BackgroundPainter::paintFillLayer. A more efficient solution might be to move
      // the layer recursion into paintFillLayer, or to compute the layer geometry here
      // and pass it down.
      if (layer.ClipOccludesNextLayers() && layer.HasOpaqueImage(_renderer)
          && layer.GetImage().TryStyleImage()->CanRender(&_renderer, _renderer.Style().UsedZoom())
          && layer.HasRepeatXY() && layer.GetBlendMode() == BlendMode::Normal
          && !BoxShadowShouldBeAppliedToBackground(_renderer, rect.Location(), bleedAvoidance, {}))
      {
        break;
      }
    }

    auto &context = _paintInfo.Context();
    auto baseBgColorUsage = BaseBackgroundColorUse;

    if (shouldDrawBackgroundInSeparateBuffer)
    {
      PaintFillLayerImpl(color, FillLayerToPaint<TLayer> {.Layer = fillLayers.UsedLast(), .IsLast = true},
                         rect, bleedAvoidance, {}, {}, op, backgroundObject, BaseBackgroundColorOnly);
      baseBgColorUsage = BaseBackgroundColorSkip;
      context.BeginTransparencyLayer(1);
    }

    for (const auto &layer : fillLayers.UsedValues() | std::views::reverse)
    {
      PaintFillLayerImpl(
        color, FillLayerToPaint<TLayer> {.Layer = layer, .IsLast = &layer == &fillLayers.UsedLast()}, rect,
        bleedAvoidance, {}, {}, op, backgroundObject, baseBgColorUsage);
    }

    if (shouldDrawBackgroundInSeparateBuffer)
    {
      context.EndTransparencyLayer();
    }
  }

  namespace
  {
    void ApplyBoxShadowForBackground(GraphicsContext &context, const RenderStyle &style) noexcept
    {
      for (const auto &shadow : style.GetBoxShadow())
      {
        if (style::IsInset(shadow))
        {
          continue;
        }

        const auto &zoomFactor = style.UsedZoomForLength();
        FloatSize shadowOffset(shadow.Location.x().ResolveZoom(zoomFactor),
                               shadow.Location.y().ResolveZoom(zoomFactor));
        context.SetDropShadow(
          {shadowOffset, shadow.Blur.ResolveZoom(zoomFactor), style.ColorWithColorFilter(shadow.Color),
           shadow.IsWebkitBoxShadow ? ShadowRadiusMode::Legacy : ShadowRadiusMode::Default});
        break;
      }
    }
  }

  void BackgroundPainter::PaintFillLayer(const Color &color,
                                         const FillLayerToPaint<style::BackgroundLayer> &layer,
                                         const LayoutRect &rect, BleedAvoidance bleedAvoidance,
                                         const inline_iterator::InlineBoxIterator &inlineBoxIterator,
                                         const LayoutRect &backgroundImageStrip, CompositeOperator op,
                                         RenderElement *backgroundObject,
                                         BaseBackgroundColorUsage baseBgColorUsage) const noexcept
  {
    PaintFillLayerImpl(color, layer, rect, bleedAvoidance, inlineBoxIterator, backgroundImageStrip, op,
                       backgroundObject, baseBgColorUsage);
  }

  template <typename TLayer>
  void BackgroundPainter::PaintFillLayerImpl(const Color &color, const FillLayerToPaint<TLayer> &layer,
                                             const LayoutRect &rect, BleedAvoidance bleedAvoidance,
                                             const inline_iterator::InlineBoxIterator &inlineBoxIterator,
                                             const LayoutRect &backgroundImageStrip, CompositeOperator op,
                                             RenderElement *backgroundObject,
                                             BaseBackgroundColorUsage baseBgColorUsage) const noexcept
  {
    GraphicsContext &context = _paintInfo.Context();

    if ((context.PaintingDisabled() && !context.DetectingContentfulPaint()) || rect.IsEmpty())
    {
      return;
    }

    auto closedEdges = inlineBoxIterator ? inlineBoxIterator->ClosedEdges() : RectEdges<bool> {true};

    const auto &style = _renderer.Style();
    auto layerClip = _overrideClip.value_or(layer.Layer.GetClip());

    bool hasRoundedBorder =
      style.HasBorderRadius()
      && (closedEdges.Start(style.GetWritingMode()) || closedEdges.End(style.GetWritingMode()));
    bool clippedWithLocalScrolling =
      _renderer.HasNonVisibleOverflow() && layer.Layer.GetAttachment() == FillAttachment::LocalBackground;
    bool isBorderFill = layerClip == FillBox::BorderBox;
    bool isRoot = _renderer.IsDocumentElementRenderer();

    Color bgColor = color;
    auto bgImage = layer.Layer.GetImage().TryStyleImage();
    bool shouldPaintBackgroundImage = bgImage && bgImage->CanRender(&_renderer, style.UsedZoom());

    // FIXME: Contentful paint detection and asynchronous image decoding (detectingContentfulPaint,
    // invalidatingImagesWithAsyncDecodes) come with loaded images; printing's forced white backgrounds come
    // with printing.

    bool baseBgColorOnly = (baseBgColorUsage == BaseBackgroundColorOnly);
    if (baseBgColorOnly && (!isRoot || !layer.IsLast || bgColor.IsOpaque()))
    {
      return;
    }

    bool colorVisible = bgColor.IsVisible();
    float deviceScaleFactor = Document().DeviceScaleFactor();
    FloatRect pixelSnappedRect = SnapRectToDevicePixels(rect, deviceScaleFactor);

    auto borderShapeRespectingBleedAvoidance =
      [&](RectEdges<bool> closedEdges, bool shrinkForBleedAvoidance = true)
    {
      auto borderRect = rect;
      if (shrinkForBleedAvoidance && bleedAvoidance == BleedAvoidance::ShrinkBackground)
      {
        // Ideally we'd use the border rect, but add a device pixel of additional inset to preserve corner
        // shape.
        borderRect = ShrinkRectByOneDevicePixel(_paintInfo.Context(), borderRect, deviceScaleFactor);
      }

      return BorderShape::ShapeForBorderRect(style, borderRect, closedEdges);
    };

    // Fast path for drawing simple color backgrounds.
    if (!isRoot && !clippedWithLocalScrolling && !shouldPaintBackgroundImage && isBorderFill && layer.IsLast)
    {
      if (!colorVisible)
      {
        return;
      }

      bool applyBoxShadowToBackground =
        BoxShadowShouldBeAppliedToBackground(_renderer, rect.Location(), bleedAvoidance, inlineBoxIterator);
      GraphicsContextStateSaver shadowStateSaver(context, applyBoxShadowToBackground);
      if (applyBoxShadowToBackground)
      {
        ApplyBoxShadowForBackground(context, style);
      }

      if (hasRoundedBorder && bleedAvoidance != BleedAvoidance::UseTransparencyLayer)
      {
        auto borderShape = borderShapeRespectingBleedAvoidance(closedEdges);
        auto previousOperator = context.CompositeOperation();
        bool saveRestoreCompositeOp = op != previousOperator;
        if (saveRestoreCompositeOp)
        {
          context.SetCompositeOperation(op);
        }

        if (bleedAvoidance == BleedAvoidance::BackgroundOverBorder)
        {
          borderShape.FillInnerShape(context, bgColor, deviceScaleFactor);
        }
        else
        {
          borderShape.FillOuterShape(context, bgColor, deviceScaleFactor);
        }

        if (saveRestoreCompositeOp)
        {
          context.SetCompositeOperation(previousOperator);
        }
      }
      else
      {
        context.FillRect(pixelSnappedRect, bgColor, op);
      }

      return;
    }

    auto clipForBorder = [&]() -> Maybe<FillBox>
    {
      if (!hasRoundedBorder)
      {
        return {};
      }

      // FillBox::BorderBox radius clipping is taken care of by BleedAvoidance::UseTransparencyLayer
      if (layerClip == FillBox::BorderBox && bleedAvoidance == BleedAvoidance::UseTransparencyLayer)
      {
        return {};
      }

      if (bleedAvoidance == BleedAvoidance::BackgroundOverBorder)
      {
        return FillBox::PaddingBox;
      }

      return layerClip;
    }();

    GraphicsContextStateSaver clipToBorderStateSaver(context, clipForBorder.has_value());
    if (clipForBorder)
    {
      switch (*clipForBorder)
      {
        case FillBox::BorderBox:
        case FillBox::BorderArea:
        case FillBox::Text:
        case FillBox::NoClip:
        {
          auto borderShape = borderShapeRespectingBleedAvoidance(closedEdges, isBorderFill);
          borderShape.ClipToOuterShape(context, deviceScaleFactor);
          break;
        }
        case FillBox::PaddingBox:
        {
          auto borderShape = borderShapeRespectingBleedAvoidance(closedEdges, isBorderFill);
          borderShape.ClipToInnerShape(context, deviceScaleFactor);
          break;
        }
        case FillBox::ContentBox:
        {
          auto borderShape = _renderer.BorderShapeForContentClipping(rect);
          borderShape.ClipToInnerShape(context, deviceScaleFactor);
          break;
        }
      }
    }

    LayoutUnit bLeft = closedEdges.Left() ? _renderer.BorderLeft() : 0_lu;
    LayoutUnit bRight = closedEdges.Right() ? _renderer.BorderRight() : 0_lu;
    LayoutUnit pLeft = closedEdges.Left() ? _renderer.PaddingLeft() : 0_lu;
    LayoutUnit pRight = closedEdges.Right() ? _renderer.PaddingRight() : 0_lu;

    GraphicsContextStateSaver clipWithScrollingStateSaver(context, clippedWithLocalScrolling);
    LayoutRect scrolledPaintRect = rect;
    if (clippedWithLocalScrolling)
    {
      // Clip to the overflow area.
      auto &renderBox = Downcast<RenderBox>(_renderer);
      context.Clip(FloatRect(renderBox.OverflowClipRect(rect.Location())));

      // Adjust the paint rect to reflect a scrolled content box with borders at the ends.
      scrolledPaintRect.MoveBy(-renderBox.GetScrollPosition());
      scrolledPaintRect.SetWidth(bLeft + renderBox.Layer()->ScrollWidth() + bRight);
      scrolledPaintRect.SetHeight(renderBox.BorderTop() + renderBox.Layer()->ScrollHeight()
                                  + renderBox.BorderBottom());
    }

    GraphicsContextStateSaver backgroundClipStateSaver(context, false);

    switch (layerClip)
    {
      case FillBox::BorderBox: break;
      case FillBox::PaddingBox:
      case FillBox::ContentBox:
      {
        // Clip to the padding or content boxes as necessary.
        if (!clipForBorder)
        {
          bool includePadding = layerClip == FillBox::ContentBox;
          LayoutRect clipRect = LayoutRect(
            scrolledPaintRect.X() + bLeft + (includePadding ? pLeft : 0_lu),
            scrolledPaintRect.Y() + _renderer.BorderTop() + (includePadding ? _renderer.PaddingTop() : 0_lu),
            scrolledPaintRect.Width() - bLeft - bRight - (includePadding ? pLeft + pRight : 0_lu),
            scrolledPaintRect.Height() - _renderer.BorderTop() - _renderer.BorderBottom()
              - (includePadding ? _renderer.PaddingTop() + _renderer.PaddingBottom() : 0_lu));
          backgroundClipStateSaver.Save();
          context.Clip(FloatRect(clipRect));
        }
        break;
      }
      case FillBox::Text:
        // FIXME: Clipping to the text (a mask of the box's text, painted with the text clip phase) comes with
        // text painting; the background isn't clipped meanwhile.
        break;
      case FillBox::BorderArea:
        // FIXME: Clipping to the border area (the border's path, or a mask of the border) comes with borders;
        // the background isn't clipped meanwhile.
        break;
      case FillBox::NoClip: break;
    }

    auto isOpaqueRoot = false;
    if (isRoot)
    {
      bool shouldPaintBaseBackground = View().RootElementShouldPaintBaseBackground();
      isOpaqueRoot = !layer.IsLast || bgColor.IsOpaque() || shouldPaintBaseBackground;
      if (!shouldPaintBaseBackground)
      {
        baseBgColorUsage = BaseBackgroundColorSkip;
      }

      View().FrameView().SetContentIsOpaque(isOpaqueRoot);
    }

    // Paint the color first underneath all images, culled if background image occludes it.
    // FIXME: In the bgLayer.hasFiniteBounds() case, we could improve the culling test
    // by verifying whether the background image covers the entire layout rect.
    if (layer.IsLast)
    {
      LayoutRect backgroundRect(scrolledPaintRect);
      bool applyBoxShadowToBackground =
        BoxShadowShouldBeAppliedToBackground(_renderer, rect.Location(), bleedAvoidance, inlineBoxIterator);
      if (applyBoxShadowToBackground || !shouldPaintBackgroundImage || !layer.Layer.HasOpaqueImage(_renderer)
          || !layer.Layer.HasRepeatXY() || layer.Layer.GetSize().IsEmpty())
      {
        if (!applyBoxShadowToBackground)
        {
          backgroundRect.Intersect(_paintInfo.Rect);
        }

        // If we have an alpha and we are painting the root element, blend with the base background color.
        Color baseColor;
        bool shouldClearBackground = false;
        if ((baseBgColorUsage != BaseBackgroundColorSkip) && isOpaqueRoot)
        {
          baseColor = View().FrameView().BaseBackgroundColor();
          if (!baseColor.IsVisible())
          {
            shouldClearBackground = true;
          }
        }

        GraphicsContextStateSaver shadowStateSaver(context, applyBoxShadowToBackground);
        if (applyBoxShadowToBackground)
        {
          ApplyBoxShadowForBackground(context, style);
        }

        FloatRect backgroundRectForPainting = SnapRectToDevicePixels(backgroundRect, deviceScaleFactor);
        if (baseColor.IsVisible())
        {
          if (!baseBgColorOnly && bgColor.IsVisible())
          {
            baseColor = BlendSourceOver(baseColor, bgColor);
          }
          context.FillRect(backgroundRectForPainting, baseColor, CompositeOperator::Copy);
        }
        else if (!baseBgColorOnly && bgColor.IsVisible())
        {
          auto operation = context.CompositeOperation();
          if (shouldClearBackground)
          {
            if (op == CompositeOperator::DestinationOut) // We're punching out the background.
            {
              operation = op;
            }
            else
            {
              operation = CompositeOperator::Copy;
            }
          }
          context.FillRect(backgroundRectForPainting, bgColor, operation);
        }
        else if (shouldClearBackground)
        {
          context.ClearRect(backgroundRectForPainting);
        }
      }
    }

    // no progressive loading of the background image
    if (!baseBgColorOnly && shouldPaintBackgroundImage)
    {
      // Multiline inline boxes paint like the image was one long strip spanning lines. The
      // backgroundImageStrip is this fictional rectangle.
      auto imageRect = backgroundImageStrip.IsEmpty() ? scrolledPaintRect : backgroundImageStrip;
      auto paintOffset = backgroundImageStrip.IsEmpty() ? rect.Location() : backgroundImageStrip.Location();
      auto geometry = CalculateFillLayerImageGeometryImpl(_renderer, _paintInfo.PaintContainer, layer.Layer,
                                                          paintOffset, imageRect, _overrideOrigin);

      auto &clientForBackgroundImage =
        backgroundObject ? *backgroundObject : static_cast<RenderElement &>(_renderer);
      bgImage->SetContainerContextForRenderer(clientForBackgroundImage,
                                              FloatSize(geometry.TileSizeWithoutPixelSnapping),
                                              _renderer.Style().UsedZoom());

      geometry.Clip(LayoutRect(pixelSnappedRect));
      RefPtr<Image> image;
      bool isFirstLine = inlineBoxIterator && inlineBoxIterator->GetLineBox()->IsFirst();
      if (!geometry.DestinationRect.IsEmpty()
          && (image = bgImage->GetImage(backgroundObject ? backgroundObject : &_renderer,
                                        FloatSize(geometry.TileSize), context, isFirstLine)))
      {
        context.SetDrawLuminanceMask(layer.Layer.GetMaskMode() == style::MaskMode::Luminance);

        ImagePaintingOptions options = {
          op == CompositeOperator::SourceOver ? layer.Layer.CompositeForPainting(layer.IsLast) : op,
          layer.Layer.GetBlendMode(), ImageOrientation::Orientation::FromImage,
          _renderer.ChooseInterpolationQuality(context, *image, &layer.Layer, geometry.TileSize)};

        (void)context.DrawTiledImage(*image, FloatRect(geometry.DestinationRect),
                                     FloatPoint(ToLayoutPoint(geometry.RelativePhase())),
                                     FloatSize(geometry.TileSize), FloatSize(geometry.SpaceSize), options);

        // FIXME: Asynchronous decoding and painted image bookkeeping (setHasEverPaintedImages, didPaintImage)
        // come with loaded images.
      }
    }
  }

  void BackgroundPainter::ClipRoundedInnerRect(GraphicsContext &context,
                                               const FloatRoundedRect &clipRect) noexcept
  {
    if (!clipRect.IsRenderable()) KRYS_UNLIKELY
    {
      auto adjustedClipRect = clipRect;
      adjustedClipRect.AdjustRadii();
      context.ClipRoundedRect(adjustedClipRect);
      return;
    }

    context.ClipRoundedRect(clipRect);
  }

  namespace
  {
    KRYS_NODISCARD Maybe<LayoutUnit> GetSpace(LayoutUnit areaSize, LayoutUnit tileSize) noexcept
    {
      if (int32 numberOfTiles = (areaSize / tileSize).ToInt(); numberOfTiles > 1)
      {
        return (areaSize - numberOfTiles * tileSize) / (numberOfTiles - 1);
      }
      return std::nullopt;
    }

    void PixelSnapBackgroundImageGeometryForPainting(LayoutRect &destinationRect, LayoutSize &tileSize,
                                                     LayoutSize &phase, LayoutSize &space,
                                                     float scaleFactor) noexcept
    {
      tileSize = LayoutSize(
        SnapRectToDevicePixels(LayoutRect(destinationRect.Location(), tileSize), scaleFactor).Size());
      phase =
        LayoutSize(SnapRectToDevicePixels(LayoutRect(destinationRect.Location(), phase), scaleFactor).Size());
      space = LayoutSize(SnapRectToDevicePixels(LayoutRect(LayoutPoint(), space), scaleFactor).Size());
      destinationRect = LayoutRect(SnapRectToDevicePixels(destinationRect, scaleFactor));
    }
  }

  BackgroundImageGeometry BackgroundPainter::CalculateFillLayerImageGeometry(
    const RenderBoxModelObject &renderer, const RenderLayerModelObject *paintContainer,
    const style::BackgroundLayer &fillLayer, const LayoutPoint &paintOffset, const LayoutRect &borderBoxRect,
    Maybe<FillBox> overrideOrigin) noexcept
  {
    return CalculateFillLayerImageGeometryImpl(renderer, paintContainer, fillLayer, paintOffset,
                                               borderBoxRect, overrideOrigin);
  }

  template <typename TLayer>
  BackgroundImageGeometry BackgroundPainter::CalculateFillLayerImageGeometryImpl(
    const RenderBoxModelObject &renderer, const RenderLayerModelObject *paintContainer,
    const TLayer &fillLayer, const LayoutPoint &paintOffset, const LayoutRect &borderBoxRect,
    Maybe<FillBox> overrideOrigin) noexcept
  {
    auto &view = renderer.View();

    LayoutUnit left;
    LayoutUnit top;
    LayoutSize positioningAreaSize;
    // Determine the background positioning area and set destination rect to the background painting area.
    // Destination rect will be adjusted later if the background is non-repeating.
    auto *enclosingLayer = renderer.EnclosingLayer();
    bool isTransformed =
      renderer.IsTransformed() || (enclosingLayer && enclosingLayer->HasTransformedAncestor());
    bool fixedAttachment = fillLayer.GetAttachment() == FillAttachment::FixedBackground && !isTransformed;

    LayoutRect destinationRect(borderBoxRect);
    float deviceScaleFactor = renderer.GetDocument().DeviceScaleFactor();
    if (!fixedAttachment)
    {
      LayoutUnit right;
      LayoutUnit bottom;
      // Scroll and Local.
      auto fillLayerOrigin = overrideOrigin.value_or(fillLayer.GetOrigin());
      if (fillLayerOrigin != FillBox::BorderBox)
      {
        left = renderer.BorderLeft();
        right = renderer.BorderRight();
        top = renderer.BorderTop();
        bottom = renderer.BorderBottom();
        if (fillLayerOrigin == FillBox::ContentBox)
        {
          left += renderer.PaddingLeft();
          right += renderer.PaddingRight();
          top += renderer.PaddingTop();
          bottom += renderer.PaddingBottom();
        }
      }

      // The background of the box generated by the root element covers the entire canvas including
      // its margins. Since those were added in already, we have to factor them out when computing
      // the background positioning area.
      if (renderer.IsDocumentElementRenderer())
      {
        positioningAreaSize = Downcast<RenderBox>(renderer).Size() - LayoutSize(left + right, top + bottom);
        positioningAreaSize =
          LayoutSize(SnapSizeToDevicePixel(positioningAreaSize, LayoutPoint(), deviceScaleFactor));
        if (view.FrameView().HasExtendedBackgroundRectForPainting())
        {
          LayoutRect extendedBackgroundRect =
            LayoutRect(view.FrameView().ExtendedBackgroundRectForPainting());
          left += (renderer.MarginLeft() - extendedBackgroundRect.X());
          top += (renderer.MarginTop() - extendedBackgroundRect.Y());
        }
      }
      else
      {
        positioningAreaSize = borderBoxRect.Size() - LayoutSize(left + right, top + bottom);
        positioningAreaSize = LayoutSize(
          SnapRectToDevicePixels(LayoutRect(paintOffset, positioningAreaSize), deviceScaleFactor).Size());
      }
    }
    else
    {
      // FIXME: Fixed backgrounds painted relative to the document (fixedBackgroundsPaintRelativeToDocument, a
      // setting off by default), and the obscured content insets and page scale the viewport doesn't have.
      LayoutRect viewportRect;
      auto &frameView = view.FrameView();
      bool useFixedLayout = frameView.UseFixedLayout() && !frameView.FixedLayoutSize().IsEmpty();

      if (useFixedLayout)
      {
        // Use the fixedLayoutSize() when useFixedLayout() because the rendering will scale
        // down the frameView to to fit in the current viewport.
        viewportRect.SetSize(LayoutSize(frameView.FixedLayoutSize()));
      }
      else
      {
        viewportRect.SetSize(LayoutSize(frameView.SizeForVisibleContent()));
      }

      if (!renderer.FixedBackgroundPaintsInLocalCoordinates())
      {
        viewportRect.SetLocation(LayoutPoint(frameView.DocumentScrollPositionRelativeToViewOrigin()));
      }

      if (paintContainer)
      {
        viewportRect.MoveBy(LayoutPoint(-paintContainer->LocalToAbsolute(FloatPoint())));
      }

      destinationRect = viewportRect;
      positioningAreaSize = destinationRect.Size();
      positioningAreaSize = LayoutSize(
        SnapRectToDevicePixels(LayoutRect(destinationRect.Location(), positioningAreaSize), deviceScaleFactor)
          .Size());
    }

    LayoutSize tileSize = CalculateFillTileSize(renderer, fillLayer, positioningAreaSize);

    auto backgroundRepeatX = fillLayer.GetRepeat().X();
    auto backgroundRepeatY = fillLayer.GetRepeat().Y();
    LayoutUnit availableWidth = positioningAreaSize.Width() - tileSize.Width();
    LayoutUnit availableHeight = positioningAreaSize.Height() - tileSize.Height();

    LayoutSize spaceSize;
    LayoutSize phase;
    auto computedXPosition =
      style::Evaluate<LayoutUnit>(fillLayer.GetPositionX(), availableWidth, style::ZoomNeeded {});
    if (backgroundRepeatX == FillRepeat::Round && positioningAreaSize.Width() > 0 && tileSize.Width() > 0)
    {
      int32 numTiles = std::max(1, RoundToInt(positioningAreaSize.Width() / tileSize.Width()));
      if (!fillLayer.GetSize().SpecifiedHeight() && backgroundRepeatY != FillRepeat::Round)
      {
        tileSize.SetHeight(tileSize.Height() * positioningAreaSize.Width() / (numTiles * tileSize.Width()));
      }

      tileSize.SetWidth(positioningAreaSize.Width() / numTiles);
      phase.SetWidth(tileSize.Width()
                       ? tileSize.Width()
                           - std::fmodf((computedXPosition + left).ToFloat(), tileSize.Width().ToFloat())
                       : 0);
    }

    auto computedYPosition =
      style::Evaluate<LayoutUnit>(fillLayer.GetPositionY(), availableHeight, style::ZoomNeeded {});
    if (backgroundRepeatY == FillRepeat::Round && positioningAreaSize.Height() > 0 && tileSize.Height() > 0)
    {
      int32 numTiles = std::max(1, RoundToInt(positioningAreaSize.Height() / tileSize.Height()));
      if (!fillLayer.GetSize().SpecifiedWidth() && backgroundRepeatX != FillRepeat::Round)
      {
        tileSize.SetWidth(tileSize.Width() * positioningAreaSize.Height() / (numTiles * tileSize.Height()));
      }

      tileSize.SetHeight(positioningAreaSize.Height() / numTiles);
      phase.SetHeight(tileSize.Height()
                        ? tileSize.Height()
                            - std::fmodf((computedYPosition + top).ToFloat(), tileSize.Height().ToFloat())
                        : 0);
    }

    if (backgroundRepeatX == FillRepeat::Repeat)
    {
      phase.SetWidth(tileSize.Width()
                       ? tileSize.Width()
                           - std::fmodf((computedXPosition + left).ToFloat(), tileSize.Width().ToFloat())
                       : 0);
      spaceSize.SetWidth(0);
    }
    else if (backgroundRepeatX == FillRepeat::Space && tileSize.Width() > 0)
    {
      if (auto space = GetSpace(positioningAreaSize.Width(), tileSize.Width()))
      {
        LayoutUnit actualWidth = tileSize.Width() + *space;
        computedXPosition = 0;
        spaceSize.SetWidth(*space);
        spaceSize.SetHeight(0);
        phase.SetWidth(
          actualWidth ? actualWidth - std::fmodf((computedXPosition + left).ToFloat(), actualWidth.ToFloat())
                      : 0);
      }
      else
      {
        backgroundRepeatX = FillRepeat::NoRepeat;
      }
    }

    if (backgroundRepeatX == FillRepeat::NoRepeat)
    {
      LayoutUnit xOffset = left + computedXPosition;
      if (xOffset > 0)
      {
        destinationRect.Move(xOffset, 0_lu);
      }
      xOffset = std::min<LayoutUnit>(xOffset, 0);
      phase.SetWidth(-xOffset);
      destinationRect.SetWidth(tileSize.Width() + xOffset);
      spaceSize.SetWidth(0);
    }

    if (backgroundRepeatY == FillRepeat::Repeat)
    {
      phase.SetHeight(tileSize.Height()
                        ? tileSize.Height()
                            - std::fmodf((computedYPosition + top).ToFloat(), tileSize.Height().ToFloat())
                        : 0);
      spaceSize.SetHeight(0);
    }
    else if (backgroundRepeatY == FillRepeat::Space && tileSize.Height() > 0)
    {
      if (auto space = GetSpace(positioningAreaSize.Height(), tileSize.Height()))
      {
        LayoutUnit actualHeight = tileSize.Height() + *space;
        computedYPosition = 0;
        spaceSize.SetHeight(*space);
        phase.SetHeight(actualHeight
                          ? actualHeight
                              - std::fmodf((computedYPosition + top).ToFloat(), actualHeight.ToFloat())
                          : 0);
      }
      else
      {
        backgroundRepeatY = FillRepeat::NoRepeat;
      }
    }

    if (backgroundRepeatY == FillRepeat::NoRepeat)
    {
      LayoutUnit yOffset = top + computedYPosition;
      if (yOffset > 0)
      {
        destinationRect.Move(0_lu, yOffset);
      }
      yOffset = std::min<LayoutUnit>(yOffset, 0);
      phase.SetHeight(-yOffset);
      destinationRect.SetHeight(tileSize.Height() + yOffset);
      spaceSize.SetHeight(0);
    }

    if (fixedAttachment)
    {
      LayoutPoint attachmentPoint = borderBoxRect.Location();
      phase.Expand(std::max<LayoutUnit>(attachmentPoint.X() - destinationRect.X(), 0),
                   std::max<LayoutUnit>(attachmentPoint.Y() - destinationRect.Y(), 0));
    }

    destinationRect.Intersect(borderBoxRect);

    auto tileSizeWithoutPixelSnapping = tileSize;
    PixelSnapBackgroundImageGeometryForPainting(destinationRect, tileSize, phase, spaceSize,
                                                deviceScaleFactor);

    return BackgroundImageGeometry(destinationRect, tileSizeWithoutPixelSnapping, tileSize, phase, spaceSize,
                                   fixedAttachment);
  }

  template <typename TLayer>
  LayoutSize BackgroundPainter::CalculateFillTileSize(const RenderBoxModelObject &renderer,
                                                      const TLayer &fillLayer,
                                                      const LayoutSize &positioningAreaSize) noexcept
  {
    auto image = fillLayer.GetImage().TryStyleImage();
    auto devicePixelSize = LayoutUnit {1.0 / renderer.GetDocument().DeviceScaleFactor()};

    LayoutSize imageIntrinsicSize;
    if (image)
    {
      imageIntrinsicSize = renderer.CalculateImageIntrinsicDimensions(
        image.get(), positioningAreaSize, RenderBoxModelObject::ScaleByUsedZoom::Yes);
      imageIntrinsicSize.Scale(1 / image->ImageScaleFactor(), 1 / image->ImageScaleFactor());
    }
    else
    {
      imageIntrinsicSize = positioningAreaSize;
    }

    auto handleKeyword = [&](bool isContain) -> LayoutSize
    {
      // Scale computation needs higher precision than what LayoutUnit can offer.
      FloatSize localImageIntrinsicSize = FloatSize(imageIntrinsicSize);
      FloatSize localPositioningAreaSize = FloatSize(positioningAreaSize);

      float horizontalScaleFactor = localImageIntrinsicSize.Width()
                                      ? (localPositioningAreaSize.Width() / localImageIntrinsicSize.Width())
                                      : 1;
      float verticalScaleFactor = localImageIntrinsicSize.Height()
                                    ? (localPositioningAreaSize.Height() / localImageIntrinsicSize.Height())
                                    : 1;
      float scaleFactor = isContain ? std::min(horizontalScaleFactor, verticalScaleFactor)
                                    : std::max(horizontalScaleFactor, verticalScaleFactor);

      if (localImageIntrinsicSize.IsEmpty())
      {
        return {};
      }

      return LayoutSize(localImageIntrinsicSize.Scaled(scaleFactor)
                          .ExpandedTo(FloatSize(devicePixelSize.ToFloat(), devicePixelSize.ToFloat())));
    };

    return fillLayer.GetSize().SwitchOn(
      [&](const css::keywords::Cover &) { return handleKeyword(false); },
      [&](const css::keywords::Contain &) { return handleKeyword(true); },
      [&](const style::BackgroundSize::LengthSize &size)
      {
        auto tileSize = positioningAreaSize;

        const auto &layerWidth = size.Width();
        const auto &layerHeight = size.Height();

        if (auto fixed = layerWidth.TryFixed())
        {
          tileSize.SetWidth(style::Evaluate<LayoutUnit>(*fixed, style::ZoomNeeded {}));
        }
        else if (layerWidth.IsPercentOrCalculated())
        {
          auto resolvedWidth =
            style::Evaluate<LayoutUnit>(layerWidth, positioningAreaSize.Width(), style::ZoomNeeded {});
          // Non-zero resolved value should always produce some content.
          tileSize.SetWidth(!resolvedWidth ? resolvedWidth : std::max(devicePixelSize, resolvedWidth));
        }

        if (auto fixed = layerHeight.TryFixed())
        {
          tileSize.SetHeight(style::Evaluate<LayoutUnit>(*fixed, style::ZoomNeeded {}));
        }
        else if (layerHeight.IsPercentOrCalculated())
        {
          auto resolvedHeight =
            style::Evaluate<LayoutUnit>(layerHeight, positioningAreaSize.Height(), style::ZoomNeeded {});
          // Non-zero resolved value should always produce some content.
          tileSize.SetHeight(!resolvedHeight ? resolvedHeight : std::max(devicePixelSize, resolvedHeight));
        }

        // If one of the values is auto we have to use the appropriate
        // scale to maintain our aspect ratio.
        bool hasNaturalAspectRatio = image && image->ImageHasNaturalDimensions();
        if (layerWidth.IsAuto() && !layerHeight.IsAuto())
        {
          if (hasNaturalAspectRatio && imageIntrinsicSize.Height())
          {
            tileSize.SetWidth(imageIntrinsicSize.Width() * tileSize.Height() / imageIntrinsicSize.Height());
          }
        }
        else if (!layerWidth.IsAuto() && layerHeight.IsAuto())
        {
          if (hasNaturalAspectRatio && imageIntrinsicSize.Width())
          {
            tileSize.SetHeight(imageIntrinsicSize.Height() * tileSize.Width() / imageIntrinsicSize.Width());
          }
        }
        else if (layerWidth.IsAuto() && layerHeight.IsAuto())
        {
          // If both width and height are auto, use the image's intrinsic size.
          tileSize = imageIntrinsicSize;
        }

        tileSize.ClampNegativeToZero();
        return tileSize;
      });
  }

  bool BackgroundPainter::BoxShadowShouldBeAppliedToBackground(
    const RenderBoxModelObject &renderer, const LayoutPoint &, BleedAvoidance bleedAvoidance,
    const inline_iterator::InlineBoxIterator &inlineBox) noexcept
  {
    if (bleedAvoidance != BleedAvoidance::None)
    {
      return false;
    }

    const auto &style = renderer.Style();

    if (style.HasUsedAppearance())
    {
      return false;
    }

    bool hasOneNormalBoxShadow = false;
    for (const auto &currentShadow : style.GetBoxShadow())
    {
      if (style::IsInset(currentShadow))
      {
        continue;
      }

      if (hasOneNormalBoxShadow)
      {
        return false;
      }
      hasOneNormalBoxShadow = true;

      if (!currentShadow.Spread.IsZero())
      {
        return false;
      }
    }

    if (!hasOneNormalBoxShadow)
    {
      return false;
    }

    Color backgroundColor = style.VisitedDependentColorWithColorFilter(css::PropertyId::BackgroundColor);
    if (!backgroundColor.IsOpaque())
    {
      return false;
    }

    const auto &lastBackgroundLayer = style.GetBackgroundLayers().UsedLast();

    if (lastBackgroundLayer.GetClip() != FillBox::BorderBox)
    {
      return false;
    }

    auto image = lastBackgroundLayer.GetImage().TryStyleImage();
    if (image && style.HasBorderRadius())
    {
      return false;
    }

    auto applyToInlineBox = [&]
    {
      // The checks here match how paintFillLayer() decides whether to clip (if it does, the shadow
      // would be clipped out, so it has to be drawn separately).
      if (inlineBox->IsRootInlineBox())
      {
        return true;
      }
      if (!inlineBox->NextInlineBoxLineLeftward() && !inlineBox->NextInlineBoxLineRightward())
      {
        return true;
      }
      const auto &renderer = inlineBox->Renderer();
      bool hasFillImage = image && image->CanRender(&renderer, renderer.Style().UsedZoom());
      return !hasFillImage && !renderer.Style().HasBorderRadius();
    };

    if (inlineBox && !applyToInlineBox())
    {
      return false;
    }

    if (renderer.HasNonVisibleOverflow()
        && lastBackgroundLayer.GetAttachment() == FillAttachment::LocalBackground)
    {
      return false;
    }

    if (Is<RenderTableCell>(renderer))
    {
      return false;
    }

    // FIXME: Images whose foreground obscures their background (RenderImage::backgroundIsKnownToBeObscured)
    // come with images.

    return true;
  }

  const dom::Document &BackgroundPainter::Document() const noexcept
  {
    return _renderer.GetDocument();
  }

  const RenderView &BackgroundPainter::View() const noexcept
  {
    return _renderer.View();
  }

  namespace
  {
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

  LayoutRect ShrinkRectByOneDevicePixel(const GraphicsContext &context, const LayoutRect &rect,
                                        float devicePixelRatio) noexcept
  {
    auto shrunkRect = rect;
    auto devicePixelUnits = SizeForDevicePixel(context, devicePixelRatio);
    shrunkRect.Inflate(-devicePixelUnits);
    return shrunkRect;
  }
}
