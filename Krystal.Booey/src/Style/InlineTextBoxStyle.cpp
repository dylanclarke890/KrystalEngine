#include "Krystal.Booey/Style/InlineTextBoxStyle.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/HTML/Enums/TagName.hpp"
#include "Krystal.Booey/HTML/HTMLAnchorElement.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorInlineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  namespace
  {
    struct UnderlineOffsetArguments
    {
      const RenderStyle &LineStyle;
      Maybe<TextUnderlinePositionUnder> TextUnderlinePositionUnder {};
    };

    KRYS_NODISCARD bool IsAncestorAndWithinBlock(const RenderInline &ancestor,
                                                 const RenderObject *child) noexcept
    {
      const RenderObject *object = child;
      while (object && (!object->IsRenderBlock() || object->IsInline()))
      {
        if (object == &ancestor)
        {
          return true;
        }
        object = object->Parent();
      }
      return false;
    }

    KRYS_NODISCARD float
      MinLogicalTopForTextDecorationLineUnder(const inline_iterator::LineBoxIterator &lineBox,
                                              float textRunLogicalTop,
                                              const RenderElement &decoratingBoxRendererForUnderline) noexcept
    {
      auto minLogicalTop = textRunLogicalTop;
      for (auto run = lineBox->LineLeftmostLeafBox(); run; run.TraverseLineRightwardOnLine())
      {
        if (run->Renderer().IsOutOfFlowPositioned())
        {
          continue; // Positioned placeholders don't affect calculations.
        }

        if (!run->Style().TextDecorationLineInEffect().HasUnderline())
        {
          continue; // If the text decoration isn't in effect on the child, then it must be outside of
                    // |decoratingBoxRendererForUnderline|'s hierarchy.
        }

        if (auto *renderInline = DynamicDowncast<RenderInline>(decoratingBoxRendererForUnderline);
            renderInline && !IsAncestorAndWithinBlock(*renderInline, &run->Renderer()))
        {
          continue;
        }

        if (run->IsText() || run->Style().GetTextDecorationSkipInk() == TextDecorationSkipInk::None)
        {
          minLogicalTop = std::min<float>(minLogicalTop, run->LogicalTop());
        }
      }
      return minLogicalTop;
    }

    KRYS_NODISCARD float MaxLogicalBottomForTextDecorationLineUnder(
      const inline_iterator::LineBoxIterator &lineBox, float textRunLogicalBottom,
      const RenderElement &decoratingBoxRendererForUnderline) noexcept
    {
      auto maxLogicalBottom = textRunLogicalBottom;
      for (auto run = lineBox->LineLeftmostLeafBox(); run; run.TraverseLineRightwardOnLine())
      {
        if (run->Renderer().IsOutOfFlowPositioned())
        {
          continue; // Positioned placeholders don't affect calculations.
        }

        if (!run->Style().TextDecorationLineInEffect().HasUnderline())
        {
          continue; // If the text decoration isn't in effect on the child, then it must be outside of
                    // |decoratingBoxRendererForUnderline|'s hierarchy.
        }

        if (auto *renderInline = DynamicDowncast<RenderInline>(decoratingBoxRendererForUnderline);
            renderInline && !IsAncestorAndWithinBlock(*renderInline, &run->Renderer()))
        {
          continue;
        }

        if (run->IsText() || run->Style().GetTextDecorationSkipInk() == TextDecorationSkipInk::None)
        {
          maxLogicalBottom = std::max<float>(maxLogicalBottom, run->LogicalBottom());
        }
      }
      return maxLogicalBottom;
    }

    KRYS_NODISCARD const RenderElement *
      EnclosingRendererWithTextDecoration(const RenderText &renderer) noexcept
    {
      for (auto *ancestor = renderer.Parent(); ancestor; ancestor = ancestor->Parent())
      {
        if (ancestor->IsRenderBlock())
        {
          return ancestor;
        }

        if (!ancestor->IsRenderInline())
        {
          // We should always find either the block container or an inline box ancestor inbetween.
          return nullptr;
        }

        auto isDecoratingInlineBox = [&]
        {
          auto *element = DynamicDowncast<html::HTMLElement>(ancestor->GetElement());
          if (element
              && (Is<html::HTMLAnchorElement>(*element)
                  || element->TagQualifiedName().TagName() == html::TagName::font))
          {
            // <font> and <a> are always considered decorating boxes.
            return true;
          }
          return ancestor->Style().GetTextDecorationLine().HasUnderline();
        };
        if (isDecoratingInlineBox())
        {
          return ancestor;
        }
      }

      return nullptr;
    }

    KRYS_NODISCARD float BoxOffsetFromBottomMost(const inline_iterator::LineBoxIterator &lineBox,
                                                 const RenderElement &decoratingInlineBoxRenderer,
                                                 float boxLogicalTop, float boxLogicalBottom) noexcept
    {
      if (decoratingInlineBoxRenderer.GetWritingMode().IsLineInverted())
      {
        return boxLogicalTop
               - MinLogicalTopForTextDecorationLineUnder(lineBox, boxLogicalTop, decoratingInlineBoxRenderer);
      }
      return MaxLogicalBottomForTextDecorationLineUnder(lineBox, boxLogicalBottom,
                                                        decoratingInlineBoxRenderer)
             - boxLogicalBottom;
    }

    KRYS_NODISCARD float TextRunOffsetFromBottomMost(const inline_iterator::LineBoxIterator &lineBox,
                                                     const RenderText &renderer, float textBoxLogicalTop,
                                                     float textBoxLogicalBottom) noexcept
    {
      auto *decoratingBoxRendererForUnderline = EnclosingRendererWithTextDecoration(renderer);
      if (!decoratingBoxRendererForUnderline)
      {
        return 0.f;
      }

      return BoxOffsetFromBottomMost(lineBox, *decoratingBoxRendererForUnderline, textBoxLogicalTop,
                                     textBoxLogicalBottom);
    }

    KRYS_NODISCARD float InlineBoxContentBoxHeight(const inline_iterator::InlineBox &inlineBox) noexcept
    {
      auto contentBoxHeight = inlineBox.LogicalHeight();
      if (!inlineBox.IsRootInlineBox())
      {
        contentBoxHeight -=
          (inlineBox.Renderer().BorderAndPaddingBefore() + inlineBox.Renderer().BorderAndPaddingAfter());
      }
      return contentBoxHeight;
    }

    KRYS_NODISCARD float DefaultGap(const RenderStyle &style) noexcept
    {
      // This represents the gap between the baseline and the closest edge of the underline.
      const float textDecorationBaseFontSize = 16.f;
      return std::max(1.f, std::ceil(style.ComputedFontSize() / textDecorationBaseFontSize / 2.f));
    }

    KRYS_NODISCARD float ComputedUnderlineOffset(const UnderlineOffsetArguments &context) noexcept
    {
      // FIXME: The code for visual overflow detection passes in a null inline text box. This means it is now
      // broken for the case where auto needs to behave like "under".

      // According to the specification `text-underline-position: auto` should avoid drawing through glyphs in
      // scripts where it would not be appropriate (e.g., ideographs).
      // Strictly speaking this can occur whenever the line contains ideographs
      // even if it is horizontal, but detecting this has performance implications. For now we only work with
      // vertical text, since we already determined the baseline type to be ideographic in that case.
      auto &styleToUse = context.LineStyle;
      auto &fontMetrics = styleToUse.MetricsOfPrimaryFont();
      auto underlineOffset = 0.f;
      auto textUnderlinePosition = styleToUse.GetTextUnderlinePosition();

      if (IsAlignedForUnder(styleToUse))
      {
        krys_debug_assert(context.TextUnderlinePositionUnder);
        // FIXME: This needs to be flipped for sideways-lr.
        if (styleToUse.GetWritingMode().IsVerticalTypographic()
            && textUnderlinePosition.VerticalTypographySide() == style::TextUnderlinePosition::Side::Right)
        {
          // In vertical typographic modes, the underline is aligned as for under, except it is always aligned
          // to the right edge of the text.
          underlineOffset =
            0.f - (styleToUse.GetTextUnderlineOffset().Resolve(styleToUse) + DefaultGap(styleToUse));
        }
        else
        {
          // Position underline relative to the bottom edge of the lowest element's content box.
          auto desiredOffset =
            context.TextUnderlinePositionUnder->TextRunLogicalHeight
            + std::max(context.TextUnderlinePositionUnder->TextRunOffsetFromBottomMost, 0.f);
          desiredOffset += styleToUse.GetTextUnderlineOffset().Resolve(styleToUse) + DefaultGap(styleToUse);
          underlineOffset = std::max<float>(desiredOffset, static_cast<float>(fontMetrics.IntAscent()));
        }
      }
      else if (textUnderlinePosition.IsFromFont())
      {
        underlineOffset = static_cast<float>(fontMetrics.IntAscent())
                          + fontMetrics.UnderlinePosition().value_or(0.f)
                          + styleToUse.GetTextUnderlineOffset().Resolve(styleToUse);
      }
      else
      {
        underlineOffset = static_cast<float>(fontMetrics.IntAscent())
                          + styleToUse.GetTextUnderlineOffset().Resolve(styleToUse, DefaultGap(styleToUse));
      }
      return underlineOffset;
    }

    KRYS_NODISCARD GlyphOverflow ComputedInkOverflowForDecorations(const RenderStyle &lineStyle,
                                                                   Maybe<float> underlineOffset) noexcept
    {
      // Compensate for the integral ceiling in GraphicsContext::computeLineBoundsAndAntialiasingModeForText()
      if (underlineOffset)
      {
        *underlineOffset += *underlineOffset >= 0 ? 1 : -1;
      }

      auto decoration = lineStyle.TextDecorationLineInEffect();
      if (decoration.IsNone())
      {
        return GlyphOverflow();
      }

      float strokeThickness = lineStyle.GetTextDecorationThickness().Resolve(lineStyle);
      WavyStrokeParameters wavyStrokeParameters;
      float wavyOffset = 0;

      TextDecorationStyle decorationStyle = lineStyle.GetTextDecorationStyle();
      float height = lineStyle.GetFontCascade().MetricsOfPrimaryFont().Height();
      GlyphOverflow overflowResult;

      if (decorationStyle == TextDecorationStyle::Wavy)
      {
        wavyStrokeParameters = GetWavyStrokeParameters(lineStyle.ComputedFontSize());
        wavyOffset = WavyOffsetFromDecoration();
        overflowResult.Left = LayoutUnit(strokeThickness);
        overflowResult.Right = LayoutUnit(strokeThickness);
      }

      // These metrics must match where underlines get drawn.
      // FIXME: Share the code in TextDecorationPainter::paintBackgroundDecorations() so we can just query it
      // for the painted geometry.
      if (decoration.HasUnderline())
      {
        krys_debug_assert(underlineOffset);
        if (decorationStyle == TextDecorationStyle::Wavy)
        {
          overflowResult.ExtendBottom(*underlineOffset + wavyOffset
                                      + wavyStrokeParameters.ControlPointDistance + strokeThickness - height);
          overflowResult.ExtendTop(
            -(*underlineOffset + wavyOffset - wavyStrokeParameters.ControlPointDistance - strokeThickness));
        }
        else
        {
          overflowResult.ExtendBottom(*underlineOffset + strokeThickness - height);
          overflowResult.ExtendTop(-*underlineOffset);
        }
      }
      if (decoration.HasOverline())
      {
        FloatRect rect(FloatPoint(), FloatSize(1, strokeThickness));
        float autoTextDecorationThickness =
          style::TextDecorationThickness {css::keywords::Auto {}}.Resolve(lineStyle);
        rect.Move(0, autoTextDecorationThickness - strokeThickness - wavyOffset);
        if (decorationStyle == TextDecorationStyle::Wavy)
        {
          FloatBoxExtent wavyExpansion;
          wavyExpansion.SetTop(wavyStrokeParameters.ControlPointDistance);
          wavyExpansion.SetBottom(wavyStrokeParameters.ControlPointDistance);
          rect.Expand(wavyExpansion);
        }
        overflowResult.ExtendTop(-rect.Y());
        overflowResult.ExtendBottom(rect.MaxY() - height);
      }
      if (decoration.HasLineThrough())
      {
        FloatRect rect(FloatPoint(), FloatSize(1, strokeThickness));
        float autoTextDecorationThickness =
          style::TextDecorationThickness {css::keywords::Auto {}}.Resolve(lineStyle);
        auto center = 2 * lineStyle.MetricsOfPrimaryFont().Ascent() / 3 + autoTextDecorationThickness / 2;
        rect.Move(0, center - strokeThickness / 2);
        if (decorationStyle == TextDecorationStyle::Wavy)
        {
          FloatBoxExtent wavyExpansion;
          wavyExpansion.SetTop(wavyStrokeParameters.ControlPointDistance);
          wavyExpansion.SetBottom(wavyStrokeParameters.ControlPointDistance);
          rect.Expand(wavyExpansion);
        }
        overflowResult.ExtendTop(-rect.Y());
        overflowResult.ExtendBottom(rect.MaxY() - height);
      }
      return overflowResult;
    }
  }

  WavyStrokeParameters GetWavyStrokeParameters(float fontSize) noexcept
  {
    WavyStrokeParameters result;
    // More information is in the WavyStrokeParameters definition.
    result.ControlPointDistance = static_cast<float>(fontSize * 1.5 / 16);
    result.Step = static_cast<float>(fontSize / 4.5);
    return result;
  }

  bool IsAlignedForUnder(const RenderStyle &decoratingBoxStyle) noexcept
  {
    auto underlinePosition = decoratingBoxStyle.GetTextUnderlinePosition();
    if (underlinePosition.IsUnder())
    {
      return true;
    }
    if (!decoratingBoxStyle.GetWritingMode().IsVerticalTypographic()
        || decoratingBoxStyle.GetWritingMode().IsSidewaysOrientation())
    {
      return false;
    }

    switch (underlinePosition.VerticalTypographySide())
    {
      case style::TextUnderlinePosition::Side::Left:
      case style::TextUnderlinePosition::Side::Right:
        // In vertical typographic modes, the underline is aligned as for under for 'left' and 'right'.
        return true;
      case style::TextUnderlinePosition::Side::NoPreference:
        // When left/right support is not enabled.
        // FIXME: The offset check is mostly about visual overflow, consider splitting out.
        return underlinePosition.IsAuto() && decoratingBoxStyle.GetTextUnderlineOffset().IsAuto();
    }
    krys_unreachable();
  }

  GlyphOverflow InkOverflowForDecorations(const RenderStyle &style,
                                          TextUnderlinePositionUnder textUnderlinePositionUnder) noexcept
  {
    auto underlineOffset = style.TextDecorationLineInEffect().HasUnderline()
                             ? Maybe<float>(ComputedUnderlineOffset({style, textUnderlinePositionUnder}))
                             : null;
    return ComputedInkOverflowForDecorations(style, underlineOffset);
  }

  GlyphOverflow InkOverflowForDecorations(const inline_iterator::LineBoxIterator &lineBox,
                                          const RenderText &renderer, float textBoxLogicalTop,
                                          float textBoxLogicalBottom) noexcept
  {
    auto &style = lineBox->IsFirst() ? renderer.FirstLineStyle() : renderer.Style();
    auto textUnderlinePositionUnder = Maybe<TextUnderlinePositionUnder> {};

    if (IsAlignedForUnder(style))
    {
      auto textRunOffset =
        TextRunOffsetFromBottomMost(lineBox, renderer, textBoxLogicalTop, textBoxLogicalBottom);
      textUnderlinePositionUnder =
        TextUnderlinePositionUnder {textBoxLogicalBottom - textBoxLogicalTop, textRunOffset};
    }

    auto underlineOffset = style.TextDecorationLineInEffect().HasUnderline()
                             ? Maybe<float>(ComputedUnderlineOffset({style, textUnderlinePositionUnder}))
                             : null;
    return ComputedInkOverflowForDecorations(style, underlineOffset);
  }

  GlyphOverflow InkOverflowForDecorations(const RenderStyle &style) noexcept
  {
    auto underlineOffset = style.TextDecorationLineInEffect().HasUnderline()
                             ? Maybe<float>(ComputedUnderlineOffset({style, {}}))
                             : null;
    return ComputedInkOverflowForDecorations(style, underlineOffset);
  }

  float TextBoxEdgeAdjustmentForUnderline(const RenderStyle &style) noexcept
  {
    if (!style.GetWritingMode().IsHorizontal())
    {
      // FIXME: In TextBoxPainter, we need to figure out how logical coords work in vertical writing mode
      // (when context is rotated).
      return 0.f;
    }

    if (style.GetTextBoxTrim() != TextBoxTrim::TrimStart && style.GetTextBoxTrim() != TextBoxTrim::TrimBoth)
    {
      return 0.f;
    }

    auto textEdge = style.GetTextBoxEdge().TryTextEdgePair();
    if (!textEdge)
    {
      return 0.f;
    }

    auto &fontMetrics = style.MetricsOfPrimaryFont();
    switch (textEdge->Over)
    {
      case TextEdgeOver::Text: return 0.f;
      case TextEdgeOver::Cap:  return static_cast<float>(fontMetrics.IntAscent() - fontMetrics.IntCapHeight());
      case TextEdgeOver::Ex:   return std::round(fontMetrics.XHeight().value_or(0.f));
      case TextEdgeOver::Ideographic:
        return static_cast<float>(fontMetrics.IntAscent(FontBaseline::Ideographic));
      case TextEdgeOver::IdeographicInk:
        // FIXME: Not implemented yet.
        return 0.f;
    }
    krys_debug_assert(false);
    return 0.f;
  }

  float UnderlineOffsetForTextBoxPainting(const inline_iterator::InlineBox &inlineBox,
                                          const RenderStyle &style) noexcept
  {
    auto underlineOffset = 0.f;
    if (!IsAlignedForUnder(style))
    {
      underlineOffset = ComputedUnderlineOffset({style, {}});
    }
    else
    {
      auto textRunOffset = BoxOffsetFromBottomMost(inlineBox.GetLineBox(), inlineBox.Renderer(),
                                                   inlineBox.LogicalTop(), inlineBox.LogicalBottom());
      underlineOffset = ComputedUnderlineOffset(
        {style, TextUnderlinePositionUnder {InlineBoxContentBoxHeight(inlineBox), textRunOffset}});
    }

    return underlineOffset - (!inlineBox.IsRootInlineBox() ? TextBoxEdgeAdjustmentForUnderline(style) : 0.f);
  }

  float OverlineOffsetForTextBoxPainting(const inline_iterator::InlineBox &inlineBox,
                                         const RenderStyle &style) noexcept
  {
    if (!style.GetWritingMode().IsVerticalTypographic())
    {
      return {};
    }

    // If 'right' causes the underline to be drawn on the "over" side of the text, then an overline also
    // switches sides and is drawn on the "under" side.
    auto underlinePosition = style.GetTextUnderlinePosition();
    return underlinePosition.VerticalTypographySide() == style::TextUnderlinePosition::Side::Right
             ? InlineBoxContentBoxHeight(inlineBox) + DefaultGap(style)
             : (0.f - DefaultGap(style));
  }
}
