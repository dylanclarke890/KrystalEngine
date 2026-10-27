#include "Krystal.Booey/Rendering/SVG/SVGTextBoxPainter.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorInlineBox.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/OutlinePainter.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceSolidColor.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGText.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextFragment.hpp"
#include "Krystal.Booey/Rendering/TextPainter.hpp"
#include "Krystal.Booey/Style/Values/Borders/Shadow.hpp"
#include <algorithm>

namespace krys::boo
{
  LegacySVGTextBoxPainter::LegacySVGTextBoxPainter(const SVGInlineTextBox &textBox, PaintInfo &paintInfo,
                                                   const LayoutPoint &paintOffset) noexcept
      : SVGTextBoxPainter(inline_iterator::BoxLegacyPath {&textBox}, paintInfo, paintOffset)
  {
  }

  ModernSVGTextBoxPainter::ModernSVGTextBoxPainter(const layout_integration::InlineContent &inlineContent,
                                                   size_t boxIndex, PaintInfo &paintInfo,
                                                   const LayoutPoint &paintOffset) noexcept
      : SVGTextBoxPainter(inline_iterator::BoxModernPath {inlineContent, boxIndex}, paintInfo, paintOffset)
  {
  }

  template <typename TTextBoxPath>
  SVGTextBoxPainter<TTextBoxPath>::SVGTextBoxPainter(TTextBoxPath &&textBox, PaintInfo &paintInfo,
                                                     const LayoutPoint &paintOffset) noexcept
      : _textBox(krys::move(textBox)), _renderer(Downcast<RenderSVGInlineText>(_textBox.Renderer())),
        _paintInfo(paintInfo), _selectableRange(_textBox.SelectableRange()), _paintOffset(paintOffset),
        _haveSelection(!_renderer.GetDocument().Printing()
                       && _renderer.View().Selection().HighlightStateForTextBox(_renderer, _selectableRange)
                            != RenderObject::HighlightState::None)
  {
  }

  template <typename TTextBoxPath>
  inline_iterator::SVGTextBoxIterator SVGTextBoxPainter<TTextBoxPath>::TextBoxIterator() const noexcept
  {
    return inline_iterator::SVGTextBoxIterator {TTextBoxPath {_textBox}};
  }

  template <typename TTextBoxPath>
  const RenderBoxModelObject &SVGTextBoxPainter<TTextBoxPath>::ParentRenderer() const noexcept
  {
    return TextBoxIterator()->ParentInlineBox()->Renderer();
  }

  FloatRect SelectionRectForTextFragment(const RenderSVGInlineText &renderer, TextDirection direction,
                                         const SVGTextFragment &fragment, uint32 startPosition,
                                         uint32 endPosition, const RenderStyle &style) noexcept
  {
    krys_debug_assert(startPosition < endPosition);

    float scalingFactor = renderer.ScalingFactor();
    krys_debug_assert(scalingFactor);

    const FontCascade &scaledFont = renderer.ScaledFont();
    const FontMetrics &scaledFontMetrics = scaledFont.MetricsOfPrimaryFont();
    FloatPoint textOrigin(fragment.X, fragment.Y);
    if (scalingFactor != 1)
    {
      textOrigin.Scale(scalingFactor);
    }

    textOrigin.Move(0, -scaledFontMetrics.Ascent());

    LayoutRect selectionRect {LayoutPoint(textOrigin),
                              LayoutSize(0, LayoutUnit(fragment.Height * scalingFactor))};
    TextRun run = ConstructTextRun(renderer.Text(), direction, style, fragment);
    scaledFont.AdjustSelectionRectForText(renderer.CanUseSimplifiedTextMeasuring().value_or(false), run,
                                          selectionRect, startPosition, endPosition);
    FloatRect snappedSelectionRect = SnapRectToDevicePixelsWithWritingDirection(
      selectionRect, renderer.GetDocument().DeviceScaleFactor(), run.Ltr());
    if (scalingFactor == 1)
    {
      return snappedSelectionRect;
    }

    snappedSelectionRect.Scale(1 / scalingFactor);
    return snappedSelectionRect;
  }

  namespace
  {
    KRYS_NODISCARD bool TextShouldBePainted(const RenderSVGInlineText &textRenderer) noexcept
    {
      return textRenderer.ScaledFont().Size() >= 0.5;
    }
  }

  template <typename TTextBoxPath>
  void SVGTextBoxPainter<TTextBoxPath>::PaintSelectionBackground() noexcept
  {
    krys_debug_assert(_paintInfo.ShouldPaintWithinRoot(Renderer()));
    krys_debug_assert(_paintInfo.Phase == PaintPhase::Foreground
                      || _paintInfo.Phase == PaintPhase::Selection);

    if (Renderer().Style().UsedVisibility() != Visibility::Visible)
    {
      return;
    }

    auto &parentRenderer = ParentRenderer();
    krys_debug_assert(!parentRenderer.GetDocument().Printing());

    // Determine whether or not we're selected.
    bool paintSelectedTextOnly = _paintInfo.Phase == PaintPhase::Selection;
    if (!_haveSelection || paintSelectedTextOnly)
    {
      return;
    }

    Color backgroundColor = Renderer().SelectionBackgroundColor();
    if (!backgroundColor.IsVisible())
    {
      return;
    }

    if (!TextShouldBePainted(Renderer()))
    {
      return;
    }

    auto &style = parentRenderer.Style();

    auto [startPosition, endPosition] = SelectionStartEnd();

    uint32 fragmentStartPosition = 0;
    uint32 fragmentEndPosition = 0;
    AffineTransform fragmentTransform;
    for (auto &fragment : TextBoxIterator()->TextFragments())
    {
      krys_debug_assert(!_legacyPaintingResource);

      fragmentStartPosition = startPosition;
      fragmentEndPosition = endPosition;
      if (!MapStartEndPositionsIntoFragmentCoordinates(_textBox.Start(), fragment, fragmentStartPosition,
                                                       fragmentEndPosition))
      {
        continue;
      }

      GraphicsContextStateSaver stateSaver(_paintInfo.Context());
      fragment.BuildFragmentTransform(fragmentTransform);
      if (!fragmentTransform.IsIdentity())
      {
        _paintInfo.Context().ConcatCTM(fragmentTransform);
      }

      _paintInfo.Context().SetFillColor(backgroundColor);
      _paintInfo.Context().FillRect(SelectionRectForTextFragment(Renderer(), _textBox.Direction(), fragment,
                                                                 fragmentStartPosition, fragmentEndPosition,
                                                                 style),
                                    backgroundColor);

      _paintingResourceMode = {};
    }

    krys_debug_assert(!_legacyPaintingResource);
  }

  template <typename TTextBoxPath>
  void SVGTextBoxPainter<TTextBoxPath>::Paint() noexcept
  {
    krys_debug_assert(_paintInfo.ShouldPaintWithinRoot(Renderer()));
    krys_debug_assert(_paintInfo.Phase == PaintPhase::Foreground
                      || _paintInfo.Phase == PaintPhase::Selection);

    if (Renderer().Style().UsedVisibility() != Visibility::Visible)
    {
      return;
    }

    // Note: We're explicitly not supporting composition & custom underlines and custom highlighters - unlike
    // LegacyInlineTextBox. If we ever need that for SVG, it's very easy to refactor and reuse the code.

    auto &parentRenderer = ParentRenderer();

    bool paintSelectedTextOnly = _paintInfo.Phase == PaintPhase::Selection;
    bool shouldPaintSelectionHighlight =
      !_paintInfo.PaintBehavior.Contains(PaintBehavior::SkipSelectionHighlight);
    bool hasSelection = !parentRenderer.GetDocument().Printing() && _haveSelection;
    if (!hasSelection && paintSelectedTextOnly)
    {
      return;
    }

    if (!TextShouldBePainted(Renderer()))
    {
      return;
    }

    auto &style = parentRenderer.Style();

    bool hasFill = style.HasFill();
    bool hasVisibleStroke = style.HasStroke() && style.GetStrokeWidth().IsPossiblyPositive();

    const RenderStyle *selectionStyle = &style;
    if (hasSelection && shouldPaintSelectionHighlight)
    {
      selectionStyle =
        parentRenderer.GetCachedPseudoStyle(style::PseudoElementIdentifier {PseudoElementType::Selection});
      if (selectionStyle)
      {
        if (!hasFill)
        {
          hasFill = selectionStyle->HasFill();
        }
        if (!hasVisibleStroke)
        {
          hasVisibleStroke =
            selectionStyle->HasStroke() && selectionStyle->GetStrokeWidth().IsPossiblyPositive();
        }
      }
      else
      {
        selectionStyle = &style;
      }
    }

    if (Renderer().View().FrameView().GetPaintBehavior().Contains(PaintBehavior::RenderingSVGClipOrMask))
    {
      hasFill = true;
      hasVisibleStroke = false;
    }

    AffineTransform fragmentTransform;
    for (auto &fragment : TextBoxIterator()->TextFragments())
    {
      krys_debug_assert(!_legacyPaintingResource);

      GraphicsContextStateSaver stateSaver(_paintInfo.Context());
      fragment.BuildFragmentTransform(fragmentTransform);
      if (!fragmentTransform.IsIdentity())
      {
        _paintInfo.Context().ConcatCTM(fragmentTransform);
      }

      // Spec: All text decorations except line-through should be drawn before the text is filled and stroked;
      // thus, the text is rendered on top of these decorations.
      auto decorations = style.TextDecorationLineInEffect();
      if (decorations.HasUnderline())
      {
        PaintDecoration(style::TextDecorationLine::Flag::Underline, fragment);
      }
      if (decorations.HasOverline())
      {
        PaintDecoration(style::TextDecorationLine::Flag::Overline, fragment);
      }

      for (auto type : style.PaintOrder())
      {
        switch (type)
        {
          case style::PaintType::Fill:
            if (!hasFill)
            {
              continue;
            }
            _paintingResourceMode = {RenderSVGResourceMode::ApplyToFill, RenderSVGResourceMode::ApplyToText};
            krys_debug_assert(selectionStyle);
            PaintText(style, *selectionStyle, fragment, hasSelection, paintSelectedTextOnly);
            break;
          case style::PaintType::Stroke:
            if (!hasVisibleStroke)
            {
              continue;
            }
            _paintingResourceMode = {RenderSVGResourceMode::ApplyToStroke,
                                     RenderSVGResourceMode::ApplyToText};
            krys_debug_assert(selectionStyle);
            PaintText(style, *selectionStyle, fragment, hasSelection, paintSelectedTextOnly);
            break;
          case style::PaintType::Markers: continue;
        }
      }

      // Spec: Line-through should be drawn after the text is filled and stroked; thus, the line-through is
      // rendered on top of the text.
      if (decorations.HasLineThrough())
      {
        PaintDecoration(style::TextDecorationLine::Flag::LineThrough, fragment);
      }

      _paintingResourceMode = {};
    }

    // Finally, paint the outline if any.
    if (Renderer().Style().HasOutline())
    {
      if (auto *renderInline = DynamicDowncast<RenderInline>(parentRenderer))
      {
        OutlinePainter {_paintInfo}.PaintOutline(*renderInline, _paintOffset);
      }
    }

    krys_debug_assert(!_legacyPaintingResource);
  }

  template <typename TTextBoxPath>
  bool SVGTextBoxPainter<TTextBoxPath>::AcquireLegacyPaintingResource(GraphicsContext *&context,
                                                                      float scalingFactor,
                                                                      RenderBoxModelObject &renderer,
                                                                      const RenderStyle &style) noexcept
  {
    krys_debug_assert(scalingFactor);
    krys_debug_assert(PaintingResourceMode().ContainsAny(
      {RenderSVGResourceMode::ApplyToFill, RenderSVGResourceMode::ApplyToStroke}));

    Color fallbackColor;
    if (PaintingResourceMode().Contains(RenderSVGResourceMode::ApplyToFill))
    {
      _legacyPaintingResource = LegacyRenderSVGResource::FillPaintingResource(renderer, style, fallbackColor);
    }
    else if (PaintingResourceMode().Contains(RenderSVGResourceMode::ApplyToStroke))
    {
      _legacyPaintingResource =
        LegacyRenderSVGResource::StrokePaintingResource(renderer, style, fallbackColor);
    }

    if (!_legacyPaintingResource)
    {
      return false;
    }

    if (!ResourceWasApplied(
          _legacyPaintingResource->ApplyResource(renderer, style, context, PaintingResourceMode())))
    {
      if (!fallbackColor.IsValid())
      {
        _legacyPaintingResource = nullptr;
        return false;
      }

      auto *fallbackResource = LegacyRenderSVGResource::SharedSolidPaintingResource();
      fallbackResource->SetColor(fallbackColor);

      _legacyPaintingResource = fallbackResource;
      if (!ResourceWasApplied(
            _legacyPaintingResource->ApplyResource(renderer, style, context, PaintingResourceMode())))
      {
        _legacyPaintingResource = nullptr;
        return false;
      }
    }

    if (PaintingResourceMode().Contains(RenderSVGResourceMode::ApplyToStroke))
    {
      if (style.GetVectorEffect() == VectorEffect::NonScalingStroke)
      {
        if (style.GetFontDescription().GetTextRenderingMode() == TextRenderingMode::GeometricPrecision)
        {
          scalingFactor = 1.0f / RenderSVGInlineText::ComputeScalingFactorForRenderer(renderer);
        }
        else
        {
          scalingFactor = 1.0f;
        }

        if (auto zoomFactor = renderer.Style().UsedZoom(); zoomFactor != 1.0f)
        {
          scalingFactor *= zoomFactor;
        }

        if (auto deviceScaleFactor = renderer.GetDocument().DeviceScaleFactor(); deviceScaleFactor != 1.0f)
        {
          scalingFactor *= deviceScaleFactor;
        }
      }

      if (scalingFactor != 1.0f)
      {
        context->SetStrokeThickness(context->StrokeThickness() * scalingFactor);
      }
    }

    return true;
  }

  template <typename TTextBoxPath>
  void SVGTextBoxPainter<TTextBoxPath>::ReleaseLegacyPaintingResource(GraphicsContext *&context,
                                                                      const Path *path) noexcept
  {
    krys_debug_assert(_legacyPaintingResource);

    _legacyPaintingResource->PostApplyResource(const_cast<RenderBoxModelObject &>(ParentRenderer()), context,
                                               PaintingResourceMode(), path,
                                               /* LegacyRenderSVGShape */ nullptr);
    _legacyPaintingResource = nullptr;
  }

  bool MapStartEndPositionsIntoFragmentCoordinates(uint32 textBoxStart, const SVGTextFragment &fragment,
                                                   uint32 &startPosition, uint32 &endPosition) noexcept
  {
    uint32 startFragment = fragment.CharacterOffset - textBoxStart;
    uint32 endFragment = startFragment + fragment.Length;

    // Find intersection between the intervals: [startFragment..endFragment) and [startPosition..endPosition)
    startPosition = std::max(startFragment, startPosition);
    endPosition = std::min(endFragment, endPosition);

    if (startPosition >= endPosition)
    {
      return false;
    }

    startPosition -= startFragment;
    endPosition -= startFragment;

    return true;
  }

  namespace
  {
    KRYS_NODISCARD float PositionOffsetForDecoration(style::TextDecorationLine decoration,
                                                     const FontMetrics &fontMetrics, float thickness) noexcept
    {
      // FIXME: For SVG Fonts we need to use the attributes defined in the <font-face> if specified.
      // Compatible with Batik/Opera.
      const float ascent = fontMetrics.Ascent();
      if (decoration == style::TextDecorationLine::Flag::Underline)
      {
        return ascent + thickness * 1.5f;
      }
      if (decoration == style::TextDecorationLine::Flag::Overline)
      {
        return thickness;
      }
      if (decoration == style::TextDecorationLine::Flag::LineThrough)
      {
        return ascent * 5 / 8.0f;
      }

      krys_debug_assert(false);
      return 0.0f;
    }

    KRYS_NODISCARD float ThicknessForDecoration(style::TextDecorationLine, const FontCascade &font) noexcept
    {
      // FIXME: For SVG Fonts we need to use the attributes defined in the <font-face> if specified.
      // Compatible with Batik/Opera
      return font.Size() / 20.0f;
    }
  }

  template <typename TTextBoxPath>
  void SVGTextBoxPainter<TTextBoxPath>::PaintDecoration(style::TextDecorationLine decoration,
                                                        const SVGTextFragment &fragment) noexcept
  {
    if (Renderer().Style().TextDecorationLineInEffect().IsNone())
    {
      return;
    }

    // Find out which render style defined the text-decoration, as its fill/stroke properties have to be used
    // for drawing instead of ours.

    auto decorationRenderer = [&]
    {
      auto parentBox = TextBoxIterator()->ParentInlineBox();

      // Lookup first render object in parent hierarchy which has text-decoration set.
      const RenderBoxModelObject *renderer = nullptr;
      while (parentBox)
      {
        renderer = &parentBox->Renderer();

        if (!renderer->Style().GetTextDecorationLine().IsNone())
        {
          break;
        }

        parentBox = parentBox->ParentInlineBox();
      }

      return renderer;
    }();

    krys_debug_assert(decorationRenderer);

    const RenderStyle &decorationStyle = decorationRenderer->Style();

    if (decorationStyle.UsedVisibility() == Visibility::Hidden)
    {
      return;
    }

    for (auto type : Renderer().Style().PaintOrder())
    {
      switch (type)
      {
        case style::PaintType::Fill:
          if (decorationStyle.HasFill())
          {
            _paintingResourceMode = RenderSVGResourceMode::ApplyToFill;
            PaintDecorationWithStyle(decoration, fragment, *decorationRenderer);
          }
          break;
        case style::PaintType::Stroke:
          if (decorationStyle.HasStroke() && decorationStyle.GetStrokeWidth().IsPossiblyPositive())
          {
            _paintingResourceMode = RenderSVGResourceMode::ApplyToStroke;
            PaintDecorationWithStyle(decoration, fragment, *decorationRenderer);
          }
          break;
        case style::PaintType::Markers: break;
      }
    }
  }

  template <typename TTextBoxPath>
  void SVGTextBoxPainter<TTextBoxPath>::PaintDecorationWithStyle(
    style::TextDecorationLine decoration, const SVGTextFragment &fragment,
    const RenderBoxModelObject &decorationRenderer) noexcept
  {
    krys_debug_assert(!_legacyPaintingResource);
    krys_debug_assert(!PaintingResourceMode().IsEmpty());

    auto &context = _paintInfo.Context();
    auto &decorationStyle = decorationRenderer.Style();

    float scalingFactor = 1;
    FontCascade scaledFont;
    RenderSVGInlineText::ComputeNewScaledFontForStyle(decorationRenderer, decorationStyle, scalingFactor,
                                                      scaledFont);
    krys_debug_assert(scalingFactor);

    // The initial y value refers to overline position.
    float thickness = ThicknessForDecoration(decoration, scaledFont);

    if (fragment.Width <= 0 && thickness <= 0)
    {
      return;
    }

    FloatPoint decorationOrigin(fragment.X, fragment.Y);
    float width = fragment.Width;
    const FontMetrics &scaledFontMetrics = scaledFont.MetricsOfPrimaryFont();

    GraphicsContextStateSaver stateSaver(context);
    if (scalingFactor != 1)
    {
      width *= scalingFactor;
      decorationOrigin.Scale(scalingFactor);
      context.Scale(1 / scalingFactor);
    }

    decorationOrigin.Move(0, -scaledFontMetrics.Ascent()
                               + PositionOffsetForDecoration(decoration, scaledFontMetrics, thickness));

    Path path;
    path.AddRect(FloatRect(decorationOrigin, FloatSize(width, thickness)));

    GraphicsContext *usedContext = &context;
    if (AcquireLegacyPaintingResource(usedContext, scalingFactor,
                                      const_cast<RenderBoxModelObject &>(decorationRenderer),
                                      decorationStyle))
    {
      ReleaseLegacyPaintingResource(usedContext, &path);
    }
  }

  template <typename TTextBoxPath>
  void SVGTextBoxPainter<TTextBoxPath>::PaintTextWithShadows(const RenderStyle &style, TextRun &textRun,
                                                             const SVGTextFragment &fragment,
                                                             uint32 startPosition,
                                                             uint32 endPosition) noexcept
  {
    auto &context = _paintInfo.Context();

    float scalingFactor = Renderer().ScalingFactor();
    krys_debug_assert(scalingFactor);

    const auto &scaledFont = Renderer().ScaledFont();
    const auto &shadows = style.GetTextShadow();

    FloatPoint textOrigin(fragment.X, fragment.Y);
    FloatSize textSize(fragment.Width, fragment.Height);

    if (scalingFactor != 1)
    {
      textOrigin.Scale(scalingFactor);
      textSize.Scale(scalingFactor);
    }

    FloatRect shadowRect(
      FloatPoint(textOrigin.X(), textOrigin.Y() - scaledFont.MetricsOfPrimaryFont().Ascent()), textSize);

    GraphicsContext *usedContext = &context;

    auto draw = [&](const style::TextShadow *shadow, bool lastShadowInIteration) -> bool
    {
      if (!AcquireLegacyPaintingResource(usedContext, scalingFactor,
                                         const_cast<RenderBoxModelObject &>(ParentRenderer()), style))
      {
        return false;
      }

      {
        ShadowApplier shadowApplier(style, *usedContext, shadow, shadowRect, lastShadowInIteration);

        if (!shadowApplier.DidSaveContext())
        {
          usedContext->Save();
        }

        usedContext->Scale(1 / scalingFactor);
        scaledFont.DrawText(*usedContext, textRun, textOrigin + shadowApplier.ExtraOffset(), startPosition,
                            endPosition);

        if (!shadowApplier.DidSaveContext())
        {
          usedContext->Restore();
        }
      }

      ReleaseLegacyPaintingResource(usedContext, /* path */ nullptr);

      return true;
    };

    if (shadows.IsNone())
    {
      draw(nullptr, false);
      return;
    }

    for (const auto &shadow : shadows)
    {
      if (!draw(&shadow, &shadow == &shadows.last()))
      {
        return;
      }
    }
  }

  template <typename TTextBoxPath>
  void SVGTextBoxPainter<TTextBoxPath>::PaintText(const RenderStyle &style, const RenderStyle &selectionStyle,
                                                  const SVGTextFragment &fragment, bool hasSelection,
                                                  bool paintSelectedTextOnly) noexcept
  {
    uint32 startPosition = 0;
    uint32 endPosition = 0;
    if (hasSelection)
    {
      std::tie(startPosition, endPosition) = SelectionStartEnd();
      hasSelection =
        MapStartEndPositionsIntoFragmentCoordinates(_textBox.Start(), fragment, startPosition, endPosition);
    }

    // Fast path if there is no selection, just draw the whole chunk part using the regular style
    TextRun textRun = ConstructTextRun(Renderer().Text(), _textBox.Direction(), style, fragment);
    if (!hasSelection || startPosition >= endPosition)
    {
      PaintTextWithShadows(style, textRun, fragment, 0, fragment.Length);
      return;
    }

    // Eventually draw text using regular style until the start position of the selection
    if (startPosition > 0 && !paintSelectedTextOnly)
    {
      PaintTextWithShadows(style, textRun, fragment, 0, startPosition);
    }

    // Draw text using selection style from the start to the end position of the selection
    {
      SVGResourcesCache::SetStyleForScope temporaryStyleChange(
        const_cast<RenderBoxModelObject &>(ParentRenderer()), style, selectionStyle);
      PaintTextWithShadows(selectionStyle, textRun, fragment, startPosition, endPosition);
    }

    // Eventually draw text using regular style from the end position of the selection to the end of the
    // current chunk part
    if (endPosition < fragment.Length && !paintSelectedTextOnly)
    {
      PaintTextWithShadows(style, textRun, fragment, endPosition, fragment.Length);
    }
  }

  TextRun ConstructTextRun(utf8_stringview text, TextDirection direction, const RenderStyle &style,
                           const SVGTextFragment &fragment) noexcept
  {
    TextRun run(text.substr(fragment.CharacterOffset, fragment.Length),
                0, /* xPos, only relevant with allowTabs=true */
                0, /* padding, only relevant for justified text, not relevant for SVG */
                ExpansionBehavior::AllowRightOnly(), direction,
                style.RtlOrdering() == Order::Visual /* directionalOverride */);

    // We handle letter & word spacing ourselves.
    run.DisableSpacing();
    return run;
  }

  template <typename TTextBoxPath>
  std::pair<uint32, uint32> SVGTextBoxPainter<TTextBoxPath>::SelectionStartEnd() const noexcept
  {
    return _renderer.View().Selection().RangeForTextBox(_renderer, _selectableRange);
  }

  template class SVGTextBoxPainter<inline_iterator::BoxModernPath>;
  template class SVGTextBoxPainter<inline_iterator::BoxLegacyPath>;
}
