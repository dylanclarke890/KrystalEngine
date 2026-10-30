#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineBoxBuilder.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingContext.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineBoxVerticalAligner.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Ruby/RubyFormattingContext.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::layout
{
  namespace
  {
    KRYS_NODISCARD InlineLevelBox::AscentAndDescent
      PrimaryFontMetricsForInlineBox(const InlineLevelBox &inlineBox,
                                     FontBaseline fontBaseline = FontBaseline::Alphabetic) noexcept
    {
      krys_debug_assert(inlineBox.IsInlineBox());
      auto &fontMetrics = inlineBox.PrimarymetricsOfPrimaryFont();
      auto ascent = InlineFormattingUtils::Ascent(fontMetrics, fontBaseline, inlineBox);
      auto descent = InlineFormattingUtils::Descent(fontMetrics, fontBaseline, inlineBox);
      return {ascent, descent};
    }

    KRYS_NODISCARD bool IsLineFitEdgeLeading(const InlineLevelBox &inlineBox) noexcept
    {
      krys_debug_assert(inlineBox.IsInlineBox());
      return inlineBox.GetLineFitEdge().IsLeading();
    }

    KRYS_NODISCARD InlineLayoutUnit AscentForTextEdgeOver(TextEdgeOver textEdgeOver,
                                                          const FontMetrics &fontMetrics,
                                                          FontBaseline fontBaseline,
                                                          const InlineLevelBox &inlineBox) noexcept
    {
      switch (textEdgeOver)
      {
        case TextEdgeOver::Text: return InlineFormattingUtils::Ascent(fontMetrics, fontBaseline, inlineBox);
        case TextEdgeOver::Cap:
          return InlineFormattingUtils::SnapToInt(fontMetrics.CapHeight().value_or(0.f), inlineBox);
        case TextEdgeOver::Ex:
          return InlineFormattingUtils::SnapToInt(fontMetrics.XHeight().value_or(0.f), inlineBox);
        case TextEdgeOver::Ideographic:
          return InlineFormattingUtils::Ascent(fontMetrics, FontBaseline::Ideographic, inlineBox);
        case TextEdgeOver::IdeographicInk:
          // FIXME: Not implemented yet.
          return InlineFormattingUtils::Ascent(fontMetrics, FontBaseline::Ideographic, inlineBox);
      }
      krys_debug_assert(false);
      return {};
    }

    KRYS_NODISCARD InlineLayoutUnit DescentForTextEdgeUnder(TextEdgeUnder textEdgeUnder,
                                                            const FontMetrics &fontMetrics,
                                                            FontBaseline fontBaseline,
                                                            const InlineLevelBox &inlineBox) noexcept
    {
      switch (textEdgeUnder)
      {
        case TextEdgeUnder::Text:       return InlineFormattingUtils::Descent(fontMetrics, fontBaseline, inlineBox);
        case TextEdgeUnder::Alphabetic: return 0.f;
        case TextEdgeUnder::Ideographic:
          return InlineFormattingUtils::Descent(fontMetrics, FontBaseline::Ideographic, inlineBox);
        case TextEdgeUnder::IdeographicInk:
          // FIXME: Not implemented yet.
          return InlineFormattingUtils::Descent(fontMetrics, FontBaseline::Ideographic, inlineBox);
      }
      krys_debug_assert(false);
      return {};
    }

    KRYS_NODISCARD InlineLevelBox::AscentAndDescent LayoutBoundstWithEdgeAdjustmentForInlineBox(
      const InlineLevelBox &inlineBox, const FontMetrics &fontMetrics, FontBaseline fontBaseline) noexcept
    {
      krys_debug_assert(inlineBox.IsInlineBox());

      if (inlineBox.IsRootInlineBox())
      {
        return {InlineFormattingUtils::Ascent(fontMetrics, fontBaseline, inlineBox),
                InlineFormattingUtils::Descent(fontMetrics, fontBaseline, inlineBox)};
      }

      auto edgePair = inlineBox.GetLineFitEdge().TryTextEdgePair();
      if (!edgePair)
      {
        // leading
        return {InlineFormattingUtils::Ascent(fontMetrics, fontBaseline, inlineBox),
                InlineFormattingUtils::Descent(fontMetrics, fontBaseline, inlineBox)};
      }
      return {AscentForTextEdgeOver(edgePair->Over, fontMetrics, fontBaseline, inlineBox),
              DescentForTextEdgeUnder(edgePair->Under, fontMetrics, fontBaseline, inlineBox)};
    }

    KRYS_NODISCARD InlineLevelBox::AscentAndDescent
      TextBoxAdjustedInlineBoxHeight(const InlineLevelBox &inlineBox, const FontMetrics &fontMetrics,
                                     FontBaseline fontBaseline) noexcept
    {
      krys_debug_assert(inlineBox.IsInlineBox());

      if (inlineBox.IsRootInlineBox())
      {
        return {InlineFormattingUtils::Ascent(fontMetrics, fontBaseline, inlineBox),
                InlineFormattingUtils::Descent(fontMetrics, fontBaseline, inlineBox)};
      }

      auto inlineBoxTextBoxTrim = inlineBox.GetTextBoxTrim();
      auto ascent = [&]() -> InlineLayoutUnit
      {
        auto textBoxEdge =
          inlineBoxTextBoxTrim == TextBoxTrim::TrimStart || inlineBoxTextBoxTrim == TextBoxTrim::TrimBoth
            ? inlineBox.GetTextBoxEdge().TryTextEdgePair()
            : null;
        if (!textBoxEdge)
        {
          return InlineFormattingUtils::Ascent(fontMetrics, fontBaseline, inlineBox);
        }
        return AscentForTextEdgeOver(textBoxEdge->Over, fontMetrics, fontBaseline, inlineBox);
      };

      auto descent = [&]() -> InlineLayoutUnit
      {
        auto textBoxEdge =
          inlineBoxTextBoxTrim == TextBoxTrim::TrimEnd || inlineBoxTextBoxTrim == TextBoxTrim::TrimBoth
            ? inlineBox.GetTextBoxEdge().TryTextEdgePair()
            : null;
        if (!textBoxEdge)
        {
          return InlineFormattingUtils::Descent(fontMetrics, fontBaseline, inlineBox);
        }
        return DescentForTextEdgeUnder(textBoxEdge->Under, fontMetrics, fontBaseline, inlineBox);
      };
      return {ascent(), descent()};
    }

    KRYS_NODISCARD style::TextEdgePair EffectiveTextBoxEdge(const InlineLevelBox &rootInlineBox,
                                                            const BlockLayoutState &blockLayoutState) noexcept
    {
      // TextBoxEdge property specifies the metrics to use for text-box-trim effects. Values have the same
      // meanings as for line-fit-edge; the auto keyword uses the value of line-fit-edge on the root inline of
      // the the affected line box, interpreting leading (the initial value) as text.
      // https://drafts.csswg.org/css-inline-3/#text-box-edge
      if (auto edgePair = blockLayoutState.GetTextBoxEdge().TryTextEdgePair())
      {
        return *edgePair;
      }
      if (auto edgePair = rootInlineBox.GetLineFitEdge().TryTextEdgePair())
      {
        return *edgePair;
      }
      return style::TextEdgePair {TextEdgeOver::Text, TextEdgeUnder::Text};
    }
  }

  LineBoxBuilder::LineBoxBuilder(const InlineFormattingContext &inlineFormattingContext,
                                 LineLayoutResult &lineLayoutResult) noexcept
      : _inlineFormattingContext(inlineFormattingContext), _lineLayoutResult(lineLayoutResult)
  {
  }

  const ElementBox &LineBoxBuilder::RootBox() const noexcept
  {
    return GetFormattingContext().Root();
  }

  const RenderStyle &LineBoxBuilder::RootStyle() const noexcept
  {
    return IsFirstFormattedLine() ? RootBox().FirstLineStyle() : RootBox().Style();
  }

  const InlineLayoutState &LineBoxBuilder::GetLayoutState() const noexcept
  {
    return GetFormattingContext().GetLayoutState();
  }

  const BlockLayoutState &LineBoxBuilder::GetBlockLayoutState() const noexcept
  {
    return GetLayoutState().ParentBlockLayoutState();
  }

  LineBox LineBoxBuilder::Build(size_t lineIndex) noexcept
  {
    auto &lineLayoutResult = GetLineLayoutResult();
    // FIXME: The overflowing hanging content should be part of the ink overflow.
    auto contentLogicalWidth = [&]
    {
      if (lineLayoutResult.BidiDirectionality.InlineBaseDirection == TextDirection::LTR)
      {
        return lineLayoutResult.Content.LogicalWidth - lineLayoutResult.Hanging.LogicalWidth;
      }
      // FIXME: Currently clients of the inline iterator interface (editing, selection, DOM etc) can't deal
      // with hanging content offsets when they affect the rest of the content. In left-to-right inline
      // direction, hanging content is always trailing hence the width does not impose offset on the rest of
      // the content while with right-to-left, the hanging content is visually leading (left side of the
      // content) and it does offset the rest of the line. What's missing is a way to tell that while the
      // content starts at the left side of the (visually leading) hanging content the root inline box has an
      // offset, the width of the hanging content (essentially decoupling the content and the root inline box
      // visual left). For now just include the hanging content in the root inline box as if it was not
      // hanging.
      return lineLayoutResult.Content.LogicalWidth;
    };
    auto lineBox = LineBox {RootBox(),
                            lineLayoutResult.Content.LogicalLeft,
                            contentLogicalWidth(),
                            lineIndex,
                            IsFirstFormattedLine(),
                            lineLayoutResult.NonSpanningInlineLevelBoxCount};
    if (lineLayoutResult.IsBlockContent())
    {
      ConstructBlockContent(lineBox);
    }
    else
    {
      ConstructInlineLevelBoxes(lineBox);
      if (lineBox.HasContent())
      {
        AdjustIdeographicBaselineIfApplicable(lineBox);
        AdjustInlineBoxHeightsForLineBoxContainIfApplicable(lineBox);
      }
      else
      {
        // Collapse all inline boxes (they are supposed to be empty as well).
        for (auto &inlineBox : lineBox.NonRootInlineLevelBoxes())
        {
          if (!inlineBox.IsInlineBox())
          {
            krys_debug_assert(inlineBox.LayoutBox().IsWordBreakOpportunity());
            krys_debug_assert(!inlineBox.LogicalHeight());
            continue;
          }
          krys_debug_assert(!inlineBox.HasContent());
          inlineBox.SetLogicalHeight({});
        }
      }
      if (_lineHasNonLineSpanningRubyContent)
      {
        RubyFormattingContext::ApplyAnnotationContributionToLayoutBounds(lineBox, GetFormattingContext());
      }
      ComputeLineBoxGeometry(lineBox);
      AdjustOutsideListMarkersPosition(lineBox);
    }

    return lineBox;
  }

  TextUtil::FallbackFontList LineBoxBuilder::CollectFallbackFonts(const InlineLevelBox &parentInlineBox,
                                                                  const Line::Run &run,
                                                                  const RenderStyle &style) noexcept
  {
    krys_debug_assert(parentInlineBox.IsInlineBox());
    auto &inlineTextBox = Downcast<InlineTextBox>(run.LayoutBox());
    if (inlineTextBox.CanUseSimplifiedContentMeasuring())
    {
      // Simplified text measuring works with primary font only.
      return {};
    }
    auto text = *run.TextContent();
    auto fallbackFonts = TextUtil::FallbackFontsForText(
      utf8_stringview {inlineTextBox.Content()}.substr(text.Start, text.Length), style,
      text.NeedsHyphen ? TextUtil::IncludeHyphen::Yes : TextUtil::IncludeHyphen::No);
    if (fallbackFonts.empty())
    {
      return {};
    }

    auto &fallbackFontsForInlineBox = _fallbackFontsForInlineBoxes[&parentInlineBox];
    for (auto &font : fallbackFonts)
    {
      if (!font)
      {
        continue;
      }
      fallbackFontsForInlineBox.insert(font);
      _fallbackFontRequiresIdeographicBaseline =
        _fallbackFontRequiresIdeographicBaseline || font->HasVerticalGlyphs();
    }
    return fallbackFonts;
  }

  InlineLevelBox::AscentAndDescent LineBoxBuilder::EnclosingAscentDescentWithFallbackFonts(
    const InlineLevelBox &inlineBox, const TextUtil::FallbackFontList &fallbackFontsForContent,
    FontBaseline fontBaseline) const noexcept
  {
    krys_debug_assert(!fallbackFontsForContent.empty());
    krys_debug_assert(inlineBox.IsInlineBox());

    // https://www.w3.org/TR/css-inline-3/#inline-height
    // When the computed line-height is normal, the layout bounds of an inline box encloses all its glyphs,
    // going from the highest A to the deepest D.
    auto maxAscent = InlineLayoutUnit {};
    auto maxDescent = InlineLayoutUnit {};
    // If line-height computes to normal and either line-fit-edge is leading or this is the root inline box,
    // the font's line gap metric may also be incorporated into A and D by adding half to each side as
    // half-leading.
    auto shouldUseLineGapToAdjustAscentDescent =
      (inlineBox.IsRootInlineBox() || IsLineFitEdgeLeading(inlineBox)) && !RootBox().IsRubyAnnotationBox();
    for (auto &font : fallbackFontsForContent)
    {
      if (!font)
      {
        continue;
      }
      auto &fontMetrics = font->GetFontMetrics();
      auto [ascent, descent] = TextBoxAdjustedInlineBoxHeight(inlineBox, fontMetrics, fontBaseline);
      if (shouldUseLineGapToAdjustAscentDescent)
      {
        auto halfLeading =
          (InlineFormattingUtils::SnapToInt(fontMetrics.LineSpacing(), inlineBox) - (ascent + descent)) / 2;
        ascent += halfLeading;
        descent += halfLeading;
      }
      maxAscent = std::max(maxAscent, ascent);
      maxDescent = std::max(maxDescent, descent);
    }
    // We need floor/ceil to match integral positioning.
    return {
      InlineFormattingUtils::SnapToInt(maxAscent, inlineBox, InlineFormattingUtils::SnapDirection::Floor),
      InlineFormattingUtils::SnapToInt(maxDescent, inlineBox, InlineFormattingUtils::SnapDirection::Ceil)};
  }

  void LineBoxBuilder::SetLayoutBoundsForInlineBox(InlineLevelBox &inlineBox,
                                                   FontBaseline fontBaseline) const noexcept
  {
    krys_debug_assert(inlineBox.IsInlineBox());

    auto layoutBounds = [&]() -> InlineLevelBox::AscentAndDescent
    {
      auto [ascent, descent] = LayoutBoundstWithEdgeAdjustmentForInlineBox(
        inlineBox, inlineBox.PrimarymetricsOfPrimaryFont(), fontBaseline);

      // FIXME: Annotation root should not have any impact here with the proper annotation box handling
      // (dedicated IFC line for annotations and line-height is 1).
      if (RootBox().IsRubyAnnotationBox())
      {
        return {ascent, descent};
      }

      if (!inlineBox.IsPreferredLineHeightFontMetricsBased())
      {
        // https://www.w3.org/TR/css-inline-3/#inline-height
        // When computed line-height is not normal, calculate the leading L as L = line-height - (A + D).
        // Half the leading (its half-leading) is added above A, and the other half below D,
        // giving an effective ascent above the baseline of A' = A + L/2, and an effective descent of D' = D +
        // L/2.
        auto halfLeading = (InlineFormattingUtils::SnapToInt(inlineBox.PreferredLineHeight(), inlineBox,
                                                             InlineFormattingUtils::SnapDirection::Floor)
                            - (ascent + descent))
                           / 2;
        if (!IsLineFitEdgeLeading(inlineBox) && !inlineBox.IsRootInlineBox())
        {
          // However, if text-box-edge is not leading and this is not the root inline box, if the half-leading
          // is positive, treat it as zero.
          halfLeading = std::min(halfLeading, 0.f);
        }
        ascent += halfLeading;
        descent += halfLeading;
      }
      else
      {
        // https://www.w3.org/TR/css-inline-3/#inline-height
        // If line-height computes to normal and either line-fit-edge is leading or this is the root inline
        // box, the font's line gap metric may also be incorporated into A and D by adding half to each side
        // as half-leading.
        auto shouldIncorporateHalfLeading = inlineBox.IsRootInlineBox() || IsLineFitEdgeLeading(inlineBox);
        if (shouldIncorporateHalfLeading)
        {
          auto lineGap = InlineFormattingUtils::SnapToInt(
            inlineBox.PrimarymetricsOfPrimaryFont().LineSpacing(), inlineBox);
          auto halfLeading = (lineGap - (ascent + descent)) / 2;
          ascent += halfLeading;
          descent += halfLeading;
        }
      }
      return {ascent, descent};
    }();

    auto inflateWithMarginBorderAndPaddingIfApplicable = [&]
    {
      if (IsLineFitEdgeLeading(inlineBox) || inlineBox.IsRootInlineBox())
      {
        return;
      }
      // Additionally, when line-fit-edge is not leading, the layout bounds are inflated by the sum of the
      // margin, border, and padding on each side.
      krys_debug_assert(!inlineBox.IsRootInlineBox());
      auto &inlineBoxGeometry = GetFormattingContext().GeometryForBox(inlineBox.LayoutBox());
      layoutBounds.Ascent += inlineBoxGeometry.MarginBorderAndPaddingBefore().ToFloat();
      layoutBounds.Descent += inlineBoxGeometry.MarginBorderAndPaddingAfter().ToFloat();
    };
    inflateWithMarginBorderAndPaddingIfApplicable();

    layoutBounds = {InlineFormattingUtils::SnapToInt(layoutBounds.Ascent, inlineBox,
                                                     InlineFormattingUtils::SnapDirection::Floor),
                    InlineFormattingUtils::SnapToInt(layoutBounds.Descent, inlineBox,
                                                     InlineFormattingUtils::SnapDirection::Ceil)};
    inlineBox.SetLayoutBounds(layoutBounds);
  }

  void LineBoxBuilder::SetVerticalPropertiesForInlineLevelBox(const LineBox &lineBox,
                                                              InlineLevelBox &inlineLevelBox) const noexcept
  {
    auto setVerticalProperties =
      [&](InlineLevelBox::AscentAndDescent ascentAndDescent, bool applyRounding = true)
    {
      if (applyRounding)
      {
        ascentAndDescent = {InlineFormattingUtils::SnapToInt(ascentAndDescent.Ascent, inlineLevelBox,
                                                             InlineFormattingUtils::SnapDirection::Floor),
                            InlineFormattingUtils::SnapToInt(ascentAndDescent.Descent, inlineLevelBox,
                                                             InlineFormattingUtils::SnapDirection::Ceil)};
      }
      inlineLevelBox.SetAscentAndDescent(ascentAndDescent);
      inlineLevelBox.SetLayoutBounds(ascentAndDescent);
      inlineLevelBox.SetLogicalHeight(ascentAndDescent.Height());
    };

    if (inlineLevelBox.IsInlineBox())
    {
      auto ascentAndDescent = [&]() -> InlineLevelBox::AscentAndDescent
      {
        auto fontBaseline = lineBox.BaselineType();
        if (inlineLevelBox.IsRootInlineBox())
        {
          return PrimaryFontMetricsForInlineBox(inlineLevelBox, fontBaseline);
        }

        auto &fontMetrics = inlineLevelBox.PrimarymetricsOfPrimaryFont();
        return TextBoxAdjustedInlineBoxHeight(inlineLevelBox, fontMetrics, fontBaseline);
      }();

      setVerticalProperties(ascentAndDescent);
      // Override default layout bounds.
      SetLayoutBoundsForInlineBox(inlineLevelBox, lineBox.BaselineType());

      // With text-box-trim, the inline box top is not always where the content starts.
      auto fontMetricBasedAscent =
        PrimaryFontMetricsForInlineBox(inlineLevelBox, lineBox.BaselineType()).Ascent;
      inlineLevelBox.SetInlineBoxContentOffsetForTextBoxTrim(fontMetricBasedAscent - ascentAndDescent.Ascent);
      return;
    }
    if (inlineLevelBox.IsLineBreakBox())
    {
      auto parentAscentAndDescent =
        PrimaryFontMetricsForInlineBox(lineBox.ParentInlineBox(inlineLevelBox), lineBox.BaselineType());
      setVerticalProperties(parentAscentAndDescent);
      return;
    }
    if (inlineLevelBox.IsListMarker())
    {
      auto &layoutBox = Downcast<ElementBox>(inlineLevelBox.LayoutBox());
      auto &listMarkerBoxGeometry = GetFormattingContext().GeometryForBox(layoutBox);
      auto marginBoxHeight = listMarkerBoxGeometry.MarginBoxHeight().ToFloat();

      if (lineBox.BaselineType() == FontBaseline::Ideographic)
      {
        // FIXME: We should rely on the integration baseline.
        setVerticalProperties(
          PrimaryFontMetricsForInlineBox(lineBox.ParentInlineBox(inlineLevelBox), lineBox.BaselineType()));
        inlineLevelBox.SetLogicalHeight(marginBoxHeight);
        return;
      }
      if (auto ascent = layoutBox.BaselineForIntegration())
      {
        if (layoutBox.IsListMarkerImage())
        {
          setVerticalProperties({ascent->ToFloat(), marginBoxHeight - ascent->ToFloat()});
          return;
        }
        // Special list marker handling. Text driven list markers behave as text when it comes to layout
        // bounds/ascent descent. This needs to consult the list marker's style (and not the root) because we
        // don't follow the DOM insertion point in case like this: <li><div>content</div></li> where the list
        // marker ends up inside the <div> and the <div>'s style != <li>'s style.
        auto listMarkerLayoutBounds = layoutBox.LayoutBoundsForListMarker();
        inlineLevelBox.SetLayoutBounds({static_cast<InlineLayoutUnit>(listMarkerLayoutBounds.first),
                                        static_cast<InlineLayoutUnit>(listMarkerLayoutBounds.second)});

        auto &fontMetrics = inlineLevelBox.PrimarymetricsOfPrimaryFont();
        auto fontBaseline = lineBox.BaselineType();
        inlineLevelBox.SetAscentAndDescent(
          {InlineFormattingUtils::Ascent(fontMetrics, fontBaseline, inlineLevelBox),
           InlineFormattingUtils::Descent(fontMetrics, fontBaseline, inlineLevelBox)});

        inlineLevelBox.SetLogicalHeight(marginBoxHeight);
        return;
      }
      setVerticalProperties({marginBoxHeight, {}});
      return;
    }
    if (inlineLevelBox.IsAtomicInlineBox())
    {
      auto &layoutBox = inlineLevelBox.LayoutBox();
      auto &inlineLevelBoxGeometry = GetFormattingContext().GeometryForBox(layoutBox);
      auto marginBoxHeight = inlineLevelBoxGeometry.MarginBoxHeight();
      auto ascent = [&]() -> InlineLayoutUnit
      {
        if (GetLayoutState().ShouldNotSynthesizeInlineBlockBaseline())
        {
          return Downcast<ElementBox>(layoutBox).BaselineForIntegration().value_or(marginBoxHeight).ToFloat();
        }

        if (layoutBox.IsInlineBlockBox())
        {
          // The baseline of an 'inline-block' is the baseline of its last line box in the normal flow, unless
          // it has either no in-flow line boxes or if its 'overflow' property has a computed value other than
          // 'visible', in which case the baseline is the bottom margin edge.
          auto synthesizeBaseline =
            !layoutBox.EstablishesInlineFormattingContext() || !layoutBox.Style().IsOverflowVisible();
          if (synthesizeBaseline)
          {
            return marginBoxHeight.ToFloat();
          }

          // FIXME: Grab the first/last baseline off of the inline formatting context (display content).
        }
        return marginBoxHeight.ToFloat();
      }();
      setVerticalProperties({ascent, marginBoxHeight.ToFloat() - ascent}, false);
      return;
    }
    krys_debug_assert(false);
  }

  void LineBoxBuilder::ConstructInlineLevelBoxes(LineBox &lineBox) noexcept
  {
    auto &formattingContext = GetFormattingContext();
    auto &rootInlineBox = lineBox.RootInlineBox();
    SetVerticalPropertiesForInlineLevelBox(lineBox, rootInlineBox);

    auto styleToUse = [&](const Box &layoutBox) -> const RenderStyle &
    {
      return IsFirstFormattedLine() ? layoutBox.FirstLineStyle() : layoutBox.Style();
    };

    auto lineHasContent = false;
    auto &inlineContent = GetLineLayoutResult().Runs;
    for (size_t index = 0; index < inlineContent.size(); ++index)
    {
      auto &run = inlineContent[index];
      auto &layoutBox = run.LayoutBox();
      auto &style = styleToUse(layoutBox);
      lineHasContent = lineHasContent || Line::Run::IsContentfulOrHasDecoration(run, formattingContext);
      auto logicalLeft = rootInlineBox.LogicalLeft() + run.LogicalLeft();

      if (run.IsText())
      {
        auto &parentInlineBox = lineBox.ParentInlineBox(run);
        parentInlineBox.SetHasContent();
        if (auto fallbackFonts = CollectFallbackFonts(parentInlineBox, run, style); !fallbackFonts.empty())
        {
          // Adjust non-empty inline box height when glyphs from the non-primary font stretch the box.
          if (parentInlineBox.IsPreferredLineHeightFontMetricsBased())
          {
            auto enclosingAscentAndDescent = EnclosingAscentDescentWithFallbackFonts(
              parentInlineBox, fallbackFonts, FontBaseline::Alphabetic);
            auto layoutBounds = parentInlineBox.LayoutBounds();
            parentInlineBox.SetLayoutBounds(
              {std::max(layoutBounds.Ascent, enclosingAscentAndDescent.Ascent),
               std::max(layoutBounds.Descent, enclosingAscentAndDescent.Descent)});
          }
        }
        continue;
      }
      if (run.IsSoftLineBreak())
      {
        lineBox.ParentInlineBox(run).SetHasContent();
        continue;
      }
      if (run.IsHardLineBreak())
      {
        auto lineBreakBox = InlineLevelBox::CreateLineBreakBox(layoutBox, style, logicalLeft);
        SetVerticalPropertiesForInlineLevelBox(lineBox, lineBreakBox);
        lineBox.AddInlineLevelBox(krys::move(lineBreakBox));

        if (GetLayoutState().InStandardsMode() || InlineQuirks::LineBreakBoxAffectsParentInlineBox(lineBox))
        {
          lineBox.ParentInlineBox(run).SetHasContent();
        }
        continue;
      }
      if (run.IsAtomicInlineBox())
      {
        auto &inlineLevelBoxGeometry = formattingContext.GeometryForBox(layoutBox);
        logicalLeft += std::max(LayoutUnit {}, inlineLevelBoxGeometry.MarginStart()).ToFloat();
        auto atomicInlineBox = InlineLevelBox::CreateAtomicInlineBox(
          layoutBox, style, logicalLeft, inlineLevelBoxGeometry.BorderBoxWidth().ToFloat());
        SetVerticalPropertiesForInlineLevelBox(lineBox, atomicInlineBox);
        lineBox.AddInlineLevelBox(krys::move(atomicInlineBox));
        continue;
      }
      if (run.IsInlineBoxStart() || run.IsLineSpanningInlineBoxStart())
      {
        auto marginStart =
          run.IsInlineBoxStart() || style.GetBoxDecorationBreak() == BoxDecorationBreak::Clone
            ? formattingContext.GeometryForBox(layoutBox).MarginStart()
            : LayoutUnit {};
        // At this point we don't know yet how wide this inline box is. Let's assume it's as long as the line
        // is and adjust it later if we come across an inlineBoxEnd run (see below). Inline box run is based
        // on margin box. Let's convert it to border box.
        logicalLeft += std::max(LayoutUnit {}, marginStart).ToFloat();
        auto initialLogicalWidth = [&]
        {
          auto logicalWidth = rootInlineBox.LogicalRight() - logicalLeft;
          // We (editing, DOM etc) can't yet handle RTL hanging content (see contentLogicalWidth in
          // LineBoxBuilder::Build).
          if (GetLineLayoutResult().BidiDirectionality.InlineBaseDirection == TextDirection::LTR)
          {
            logicalWidth += GetLineLayoutResult().Hanging.LogicalWidth;
          }
          return logicalWidth;
        }();
        initialLogicalWidth = std::max(initialLogicalWidth, 0.f);
        auto inlineBox = InlineLevelBox::CreateInlineBox(layoutBox, style, logicalLeft, initialLogicalWidth,
                                                         run.IsInlineBoxStart()
                                                           ? InlineLevelBox::LineSpanningInlineBox::No
                                                           : InlineLevelBox::LineSpanningInlineBox::Yes);
        inlineBox.SetTextEmphasis(InlineFormattingUtils::TextEmphasisForInlineBox(layoutBox, RootBox()));
        SetVerticalPropertiesForInlineLevelBox(lineBox, inlineBox);
        if (run.IsInlineBoxStart())
        {
          inlineBox.SetIsFirstBox();
          _lineHasNonLineSpanningRubyContent = _lineHasNonLineSpanningRubyContent || layoutBox.IsRubyBase();
        }
        lineBox.AddInlineLevelBox(krys::move(inlineBox));
        continue;
      }
      if (run.IsInlineBoxEnd())
      {
        // Adjust the logical width when the inline box closes on this line.
        // Note that margin end does not affect the logical width (e.g. positive margin right does not make
        // the run wider).
        auto &inlineBox = lineBox.InlineLevelBoxFor(run);
        krys_debug_assert(inlineBox.IsInlineBox());
        // Inline box run is based on margin box. Let's convert it to border box.
        // Negative margin end makes the run have negative width.
        auto marginEndAdjustemnt = -formattingContext.GeometryForBox(layoutBox).MarginEnd().ToFloat();
        auto logicalWidth = run.LogicalWidth() + marginEndAdjustemnt;
        auto inlineBoxLogicalRight = logicalLeft + logicalWidth;
        // When the content pulls the </span> to the logical left direction (e.g. negative letter space)
        // make sure we don't end up with negative logical width on the inline box.
        inlineBox.SetLogicalWidth(std::max(0.f, inlineBoxLogicalRight - inlineBox.LogicalLeft()));
        inlineBox.SetIsLastBox();
        continue;
      }
      if (run.IsListMarker())
      {
        auto &listMarkerBox = Downcast<ElementBox>(layoutBox);
        if (!listMarkerBox.IsListMarkerImage())
        {
          // Non-image type of list markers make their parent inline boxes (e.g. root inline box) contentful
          // (and stretch them vertically).
          lineBox.ParentInlineBox(run).SetHasContent();
        }

        if (run.IsListMarkerOutside())
        {
          _outsideListMarkers.push_back(index);
        }

        auto atomicInlineBox = InlineLevelBox::CreateAtomicInlineBox(
          listMarkerBox, style, logicalLeft,
          formattingContext.GeometryForBox(listMarkerBox).BorderBoxWidth().ToFloat());
        SetVerticalPropertiesForInlineLevelBox(lineBox, atomicInlineBox);
        lineBox.AddInlineLevelBox(krys::move(atomicInlineBox));
        continue;
      }
      if (run.IsWordBreakOpportunity())
      {
        lineBox.AddInlineLevelBox(InlineLevelBox::CreateGenericInlineLevelBox(layoutBox, style, logicalLeft));
        continue;
      }
      krys_debug_assert(run.IsOpaque());
    }
    lineBox.SetHasContent(lineHasContent);
  }

  void LineBoxBuilder::ConstructBlockContent(LineBox &lineBox) noexcept
  {
    auto &lineLayoutResult = GetLineLayoutResult();
    auto &runs = lineLayoutResult.Runs;
    if (runs.empty() || !runs.back().IsBlock())
    {
      krys_debug_assert(false);
      return;
    }

    // Since we don't need to position and align block content inside the line, we don't need to create any
    // boxes for this block content.
    auto &blockGeometry = GetFormattingContext().GeometryForBox(runs.back().LayoutBox());
    for (size_t index = 0; index < runs.size() - 1; ++index)
    {
      auto &run = runs[index];
      if (run.IsLineSpanningInlineBoxStart())
      {
        auto lineSpanningInlineBox = InlineLevelBox::CreateInlineBox(
          run.LayoutBox(), run.LayoutBox().Style(), lineLayoutResult.Content.LogicalLeft,
          lineLayoutResult.Geometry.LogicalWidth, InlineLevelBox::LineSpanningInlineBox::Yes);
        SetVerticalPropertiesForInlineLevelBox(lineBox, lineSpanningInlineBox);
        lineSpanningInlineBox.SetLogicalTop(blockGeometry.MarginBefore().ToFloat());
        lineSpanningInlineBox.SetLogicalHeight(blockGeometry.BorderBoxHeight().ToFloat());
        lineBox.AddInlineLevelBox(krys::move(lineSpanningInlineBox));
        continue;
      }
      krys_debug_assert(false);
    }

    auto blockLineLogicalTopLeft = InlineLayoutPoint {lineLayoutResult.Geometry.InitialLogicalLeft,
                                                      lineLayoutResult.Geometry.LogicalTopLeft.Y()};
    lineBox.SetLogicalRect({blockLineLogicalTopLeft, lineLayoutResult.Geometry.LogicalWidth,
                            blockGeometry.MarginBoxHeight().ToFloat()});
    SetVerticalPropertiesForInlineLevelBox(lineBox, lineBox.RootInlineBox());
    // FIXME: Let's considered collapsed block boxes contentful for now.
    lineBox.SetHasContent(true);
  }

  void LineBoxBuilder::AdjustInlineBoxHeightsForLineBoxContainIfApplicable(LineBox &lineBox) noexcept
  {
    // While line-box-contain normally tells whether a certain type of content should be included when
    // computing the line box height, font and Glyphs values affect the "size" of the associated inline boxes
    // (which then affect the line box height).
    auto lineBoxContain = RootBox().Style().LineBoxContain();
    // Collect layout bounds based on the contain property and set them on the inline boxes when they are
    // applicable.
    HashMap<InlineLevelBox *, TextUtil::EnclosingAscentDescent> inlineBoxBoundsMap;

    if (lineBoxContain.Contains(style::WebkitLineBoxContainValue::InlineBox))
    {
      for (auto &inlineLevelBox : lineBox.NonRootInlineLevelBoxes())
      {
        if (!inlineLevelBox.IsInlineBox())
        {
          continue;
        }
        auto &inlineBoxGeometry = GetFormattingContext().GeometryForBox(inlineLevelBox.LayoutBox());
        auto ascent = inlineLevelBox.Ascent() + inlineBoxGeometry.MarginBorderAndPaddingBefore().ToFloat();
        auto descent = inlineLevelBox.Descent() + inlineBoxGeometry.MarginBorderAndPaddingAfter().ToFloat();
        inlineBoxBoundsMap.insert_or_assign(&inlineLevelBox,
                                            TextUtil::EnclosingAscentDescent {ascent, descent});
      }
    }

    if (lineBoxContain.Contains(style::WebkitLineBoxContainValue::Font))
    {
      // Assign font based layout bounds to all inline boxes.
      auto ensureFontMetricsBasedHeight = [&](InlineLevelBox &inlineBox)
      {
        krys_debug_assert(inlineBox.IsInlineBox());
        auto [ascent, descent] = PrimaryFontMetricsForInlineBox(inlineBox, lineBox.BaselineType());
        auto lineGap =
          InlineFormattingUtils::SnapToInt(inlineBox.PrimarymetricsOfPrimaryFont().LineSpacing(), inlineBox);
        auto halfLeading = !RootBox().IsRubyAnnotationBox() ? (lineGap - (ascent + descent)) / 2 : 0.f;
        ascent += halfLeading;
        descent += halfLeading;
        if (auto entry = _fallbackFontsForInlineBoxes.find(&inlineBox);
            entry != _fallbackFontsForInlineBoxes.end() && !entry->second.empty())
        {
          auto enclosingAscentAndDescent =
            EnclosingAscentDescentWithFallbackFonts(inlineBox, entry->second, lineBox.BaselineType());
          ascent = std::max(ascent, enclosingAscentAndDescent.Ascent);
          descent = std::max(descent, enclosingAscentAndDescent.Descent);
        }
        inlineBoxBoundsMap.insert_or_assign(&inlineBox, TextUtil::EnclosingAscentDescent {ascent, descent});
      };

      ensureFontMetricsBasedHeight(lineBox.RootInlineBox());
      for (auto &inlineLevelBox : lineBox.NonRootInlineLevelBoxes())
      {
        if (!inlineLevelBox.IsInlineBox())
        {
          continue;
        }
        ensureFontMetricsBasedHeight(inlineLevelBox);
      }
    }

    auto enclosingGlyphBoundsForRun = [&](const Line::Run &run)
    {
      auto &textBox = Downcast<InlineTextBox>(run.LayoutBox());
      auto &textContent = run.TextContent();
      auto &style = IsFirstFormattedLine() ? textBox.FirstLineStyle() : textBox.Style();
      return TextUtil::EnclosingGlyphBoundsForText(
        utf8_stringview {textBox.Content()}.substr(textContent->Start, textContent->Length), style,
        textBox.ShouldUseSimpleGlyphOverflowCodePath() ? TextUtil::ShouldUseSimpleGlyphOverflowCodePath::Yes
                                                       : TextUtil::ShouldUseSimpleGlyphOverflowCodePath::No);
    };

    if (lineBoxContain.Contains(style::WebkitLineBoxContainValue::Glyphs))
    {
      // Compute text content (glyphs) hugging inline box layout bounds.
      for (auto &run : GetLineLayoutResult().Runs)
      {
        if (!run.IsText())
        {
          continue;
        }

        auto enclosingAscentDescentForRun = enclosingGlyphBoundsForRun(run);

        auto &parentInlineBox = lineBox.ParentInlineBox(run);
        auto &enclosingAscentDescentForInlineBox = inlineBoxBoundsMap[&parentInlineBox];
        enclosingAscentDescentForInlineBox.Ascent =
          std::max(enclosingAscentDescentForInlineBox.Ascent, -enclosingAscentDescentForRun.Ascent);
        enclosingAscentDescentForInlineBox.Descent =
          std::max(enclosingAscentDescentForInlineBox.Descent, enclosingAscentDescentForRun.Descent);
      }
    }

    if (lineBoxContain.Contains(style::WebkitLineBoxContainValue::InitialLetter))
    {
      // Initial letter contain is based on the font metrics cap geometry and we hug descent.
      auto &rootInlineBox = lineBox.RootInlineBox();
      auto &fontMetrics = rootInlineBox.PrimarymetricsOfPrimaryFont();
      auto initialLetterAscent =
        InlineFormattingUtils::SnapToInt(fontMetrics.CapHeight().value_or(0.f), rootInlineBox);
      auto initialLetterDescent = InlineLayoutUnit {};

      for (auto &run : GetLineLayoutResult().Runs)
      {
        // We really should only have one text run for initial letter.
        if (!run.IsText())
        {
          continue;
        }

        auto ascentAndDescent = enclosingGlyphBoundsForRun(run);

        initialLetterDescent = ascentAndDescent.Descent;
        if (lineBox.BaselineType() != FontBaseline::Alphabetic)
        {
          initialLetterAscent = -ascentAndDescent.Ascent;
        }
        break;
      }
      inlineBoxBoundsMap.insert_or_assign(
        &rootInlineBox, TextUtil::EnclosingAscentDescent {initialLetterAscent, initialLetterDescent});
    }

    for (auto &[inlineBox, enclosingAscentDescentForInlineBox] : inlineBoxBoundsMap)
    {
      auto inlineBoxLayoutBounds = inlineBox->LayoutBounds();

      // "line-box-container: block" The extended block progression dimension of the root inline box must fit
      // within the line box.
      auto mayShrinkLineBox = inlineBox->IsRootInlineBox()
                                ? !lineBoxContain.Contains(style::WebkitLineBoxContainValue::Block)
                                : true;
      auto ascent = mayShrinkLineBox
                      ? enclosingAscentDescentForInlineBox.Ascent
                      : std::max(enclosingAscentDescentForInlineBox.Ascent, inlineBoxLayoutBounds.Ascent);
      auto descent = mayShrinkLineBox
                       ? enclosingAscentDescentForInlineBox.Descent
                       : std::max(enclosingAscentDescentForInlineBox.Descent, inlineBoxLayoutBounds.Descent);
      inlineBox->SetLayoutBounds(
        {InlineFormattingUtils::SnapToInt(ascent, *inlineBox, InlineFormattingUtils::SnapDirection::Ceil),
         InlineFormattingUtils::SnapToInt(descent, *inlineBox, InlineFormattingUtils::SnapDirection::Ceil)});
    }
  }

  void LineBoxBuilder::AdjustIdeographicBaselineIfApplicable(LineBox &lineBox) noexcept
  {
    // Re-compute the ascent/descent values for the inline boxes on the line (including the root inline box)
    // when the style/content needs ideographic baseline setup in vertical writing mode.
    auto &rootInlineBox = lineBox.RootInlineBox();

    auto lineNeedsIdeographicBaseline = [&]
    {
      auto styleToUse = [&](const InlineLevelBox &inlineLevelBox) -> const RenderStyle &
      {
        return IsFirstFormattedLine() ? inlineLevelBox.LayoutBox().FirstLineStyle()
                                      : inlineLevelBox.LayoutBox().Style();
      };
      auto &rootInlineBoxStyle = styleToUse(rootInlineBox);
      if (rootInlineBoxStyle.GetWritingMode().IsHorizontal())
      {
        return false;
      }

      auto primaryFontRequiresIdeographicBaseline = [&](const RenderStyle &style)
      {
        return style.GetFontDescription().Orientation() == FontOrientation::Vertical
               || style.GetFontCascade().PrimaryFont()->HasVerticalGlyphs();
      };

      if (_fallbackFontRequiresIdeographicBaseline
          || primaryFontRequiresIdeographicBaseline(rootInlineBoxStyle))
      {
        return true;
      }
      for (auto &inlineLevelBox : lineBox.NonRootInlineLevelBoxes())
      {
        if (inlineLevelBox.IsInlineBox()
            && primaryFontRequiresIdeographicBaseline(styleToUse(inlineLevelBox)))
        {
          return true;
        }
      }
      return false;
    };

    if (!lineNeedsIdeographicBaseline())
    {
      return;
    }

    lineBox.SetBaselineType(FontBaseline::Ideographic);

    auto adjustLayoutBoundsWithIdeographicBaseline = [&](InlineLevelBox &inlineLevelBox)
    {
      auto initiatesLayoutBoundsChange =
        inlineLevelBox.IsInlineBox() || inlineLevelBox.IsAtomicInlineBox() || inlineLevelBox.IsLineBreakBox();
      if (!initiatesLayoutBoundsChange)
      {
        return;
      }

      if (inlineLevelBox.IsInlineBox() || inlineLevelBox.IsLineBreakBox()
          || (inlineLevelBox.IsListMarker()
              && !Downcast<ElementBox>(inlineLevelBox.LayoutBox()).IsListMarkerImage()))
      {
        SetVerticalPropertiesForInlineLevelBox(lineBox, inlineLevelBox);
      }
      else if (inlineLevelBox.IsAtomicInlineBox())
      {
        auto inlineLevelBoxHeight = inlineLevelBox.LogicalHeight();
        auto ideographicBaseline = static_cast<InlineLayoutUnit>(std::round(inlineLevelBoxHeight / 2));
        // Move the baseline position but keep the same logical height.
        inlineLevelBox.SetAscentAndDescent({ideographicBaseline, inlineLevelBoxHeight - ideographicBaseline});
        inlineLevelBox.SetLayoutBounds({ideographicBaseline, inlineLevelBoxHeight - ideographicBaseline});
      }

      auto needsFontFallbackAdjustment = inlineLevelBox.IsInlineBox();
      if (needsFontFallbackAdjustment)
      {
        if (auto entry = _fallbackFontsForInlineBoxes.find(&inlineLevelBox);
            entry != _fallbackFontsForInlineBoxes.end() && !entry->second.empty()
            && inlineLevelBox.IsPreferredLineHeightFontMetricsBased())
        {
          auto enclosingAscentAndDescent =
            EnclosingAscentDescentWithFallbackFonts(inlineLevelBox, entry->second, FontBaseline::Ideographic);
          auto layoutBounds = inlineLevelBox.LayoutBounds();
          inlineLevelBox.SetLayoutBounds({std::max(layoutBounds.Ascent, enclosingAscentAndDescent.Ascent),
                                          std::max(layoutBounds.Descent, enclosingAscentAndDescent.Descent)});
        }
      }
    };

    adjustLayoutBoundsWithIdeographicBaseline(rootInlineBox);
    for (auto &inlineLevelBox : lineBox.NonRootInlineLevelBoxes())
    {
      if (inlineLevelBox.IsAtomicInlineBox())
      {
        auto &layoutBox = inlineLevelBox.LayoutBox();
        if (layoutBox.IsInlineTableBox())
        {
          // This is the integration codepath where inline table boxes are represented as atomic inline boxes.
          // Integration codepath sets ideographic baseline by default for non-horizontal content.
          continue;
        }
        auto isInlineBlockWithNonSyntheticBaseline =
          layoutBox.IsInlineBlockBox()
          && Downcast<ElementBox>(layoutBox).BaselineForIntegration().has_value();
        if (isInlineBlockWithNonSyntheticBaseline && !layoutBox.GetWritingMode().IsHorizontal())
        {
          continue;
        }
      }
      adjustLayoutBoundsWithIdeographicBaseline(inlineLevelBox);
    }
  }

  InlineLayoutUnit LineBoxBuilder::ApplyTextBoxTrimOnLineBoxIfNeeded(InlineLayoutUnit lineBoxLogicalHeight,
                                                                     LineBox &lineBox) const noexcept
  {
    auto &rootInlineBox = lineBox.RootInlineBox();
    auto textBoxTrim = GetBlockLayoutState().GetTextBoxTrim();
    auto textBoxEdge = EffectiveTextBoxEdge(rootInlineBox, GetBlockLayoutState());
    auto shouldTrimBlockStartOfLineBox =
      IsFirstFormattedLine() && textBoxTrim.Contains(BlockLayoutState::TextBoxTrimSide::Start);
    auto shouldTrimBlockEndOfLineBox =
      IsLastLine() && textBoxTrim.Contains(BlockLayoutState::TextBoxTrimSide::End);
    if (!shouldTrimBlockStartOfLineBox && !shouldTrimBlockEndOfLineBox)
    {
      return lineBoxLogicalHeight;
    }

    auto &primaryFontMetrics = rootInlineBox.PrimarymetricsOfPrimaryFont();
    if (shouldTrimBlockEndOfLineBox)
    {
      auto textBoxEdgeUnderForRootInlineBox = [&]() -> InlineLayoutUnit
      {
        switch (textBoxEdge.Under)
        {
          case TextEdgeUnder::Text: return 0.f;
          case TextEdgeUnder::Alphabetic:
            return InlineFormattingUtils::Descent(primaryFontMetrics, FontBaseline::Alphabetic,
                                                  rootInlineBox);
          case TextEdgeUnder::Ideographic:
          case TextEdgeUnder::IdeographicInk:
            // FIXME: Not implemented yet.
            return 0.f;
        }
        krys_debug_assert(false);
        return 0.f;
      }();
      auto needToTrimThisMuch = std::max(0.f, (lineBoxLogicalHeight - rootInlineBox.LogicalBottom())
                                                + textBoxEdgeUnderForRootInlineBox);
      lineBoxLogicalHeight -= needToTrimThisMuch;
    }
    if (shouldTrimBlockStartOfLineBox)
    {
      auto textBoxEdgeOverForRootInlineBox = [&]() -> InlineLayoutUnit
      {
        switch (textBoxEdge.Over)
        {
          case TextEdgeOver::Text: return 0.f;
          case TextEdgeOver::Cap:
            return InlineFormattingUtils::Ascent(primaryFontMetrics, FontBaseline::Alphabetic, rootInlineBox)
                   - InlineFormattingUtils::SnapToInt(primaryFontMetrics.CapHeight().value_or(0.f),
                                                      rootInlineBox);
          case TextEdgeOver::Ex:
            return InlineFormattingUtils::Ascent(primaryFontMetrics, FontBaseline::Alphabetic, rootInlineBox)
                   - InlineFormattingUtils::SnapToInt(primaryFontMetrics.XHeight().value_or(0.f),
                                                      rootInlineBox);
          case TextEdgeOver::Ideographic:
          case TextEdgeOver::IdeographicInk:
            // FIXME: Not implemented yet.
            return 0.f;
        }
        krys_debug_assert(false);
        return 0.f;
      };
      auto needToTrimThisMuch =
        std::max(0.f, GetLineLayoutResult().Geometry.InitialLetterClearGap.value_or(0.f)
                        + rootInlineBox.LogicalTop() + textBoxEdgeOverForRootInlineBox());
      lineBoxLogicalHeight -= needToTrimThisMuch;

      auto adjustRootInlineAndBottomAlignedBoxes = [&]
      {
        // When trimming makes the line box move up, bottom aligned boxes has to follow the root inline box.
        // All other boxes can keep their vertical positions relative to the line box top.
        for (auto &inlineLevelBox : lineBox.NonRootInlineLevelBoxes())
        {
          if (!HoldsAlternative<css::keywords::Bottom>(inlineLevelBox.VerticalAlign()))
          {
            continue;
          }
          inlineLevelBox.SetLogicalTop(inlineLevelBox.LogicalTop() - needToTrimThisMuch);
        }
        rootInlineBox.SetLogicalTop(rootInlineBox.LogicalTop() - needToTrimThisMuch);
      };
      adjustRootInlineAndBottomAlignedBoxes();
      _lineLayoutResult.FirstLineStartTrim = needToTrimThisMuch;
    }
    return lineBoxLogicalHeight;
  }

  void LineBoxBuilder::ComputeLineBoxGeometry(LineBox &lineBox) const noexcept
  {
    auto lineBoxLogicalHeight = ApplyTextBoxTrimOnLineBoxIfNeeded(
      LineBoxVerticalAligner {GetFormattingContext()}.ComputeLogicalHeightAndAlign(lineBox), lineBox);
    if (GetFormattingContext().Quirks().ShouldCollapseLineBoxHeight(GetLineLayoutResult().Runs,
                                                                    _outsideListMarkers.size()))
    {
      lineBoxLogicalHeight = {};
    }
    lineBox.SetLogicalRect({GetLineLayoutResult().Geometry.LogicalTopLeft,
                            GetLineLayoutResult().Geometry.LogicalWidth, lineBoxLogicalHeight});
  }

  void LineBoxBuilder::AdjustOutsideListMarkersPosition(LineBox &lineBox) noexcept
  {
    auto lineBoxRect = lineBox.LogicalRect();
    auto floatConstraints = GetFormattingContext().GetFloatingContext().GetConstraints(
      LayoutUnit {lineBoxRect.Top()}, LayoutUnit {lineBoxRect.Bottom()},
      FloatingContext::MayBeAboveLastFloat::No);

    auto lineBoxOffset = lineBoxRect.Left()
                         - (GetLineLayoutResult().Geometry.InitialLogicalLeft
                            + GetLineLayoutResult().Geometry.IntrusiveFloatsOffset);
    auto rootInlineBoxLogicalLeft = lineBox.LogicalRectForRootInlineBox().Left();
    auto rootInlineBoxOffsetFromContentBoxOrIntrusiveFloat = lineBoxOffset + rootInlineBoxLogicalLeft;
    for (auto listMarkerBoxIndex : _outsideListMarkers)
    {
      auto &listMarkerRun = GetLineLayoutResult().Runs[listMarkerBoxIndex];
      krys_debug_assert(listMarkerRun.IsListMarkerOutside());
      auto &listMarkerBox = Downcast<ElementBox>(listMarkerRun.LayoutBox());
      auto &listMarkerInlineLevelBox = lineBox.InlineLevelBoxFor(listMarkerRun);
      // Move it to the logical left of the line box (from the logical left of the root inline box).
      auto listMarkerInitialOffsetFromRootInlineBox =
        listMarkerInlineLevelBox.LogicalLeft() - rootInlineBoxOffsetFromContentBoxOrIntrusiveFloat;
      auto logicalLeft = listMarkerInitialOffsetFromRootInlineBox;
      auto nestedListMarkerMarginStart = [&]
      {
        // A list marker without a nested offset reports zero.
        auto nestedOffset = GetLayoutState().NestedListMarkerOffset(listMarkerBox);
        if (!nestedOffset)
        {
          return LayoutUnit {};
        }
        // Nested list markers (in standards mode) share the same line and have offsets as if they had
        // dedicated lines.
        // <!DOCTYPE html>
        // <ul><li><ul><li>markers on the same line in standards mode
        // vs.
        // <ul><li><ul><li>markers with dedicated lines in quirks mode
        // or
        // <!DOCTYPE html>
        // <ul><li>markers<ul><li>with dedicated lines
        // While a float may not constrain the line, it could constrain the nested list marker (being it
        // outside of the line box to the logical left).
        // FIXME: We may need to do this in a post-process task after the line box geometry is computed.
        return floatConstraints.Start
                 ? std::min(LayoutUnit {}, std::max(floatConstraints.Start->X, nestedOffset))
                 : nestedOffset;
      }();
      AdjustMarginStartForListMarker(listMarkerBox, nestedListMarkerMarginStart,
                                     rootInlineBoxOffsetFromContentBoxOrIntrusiveFloat);
      logicalLeft += nestedListMarkerMarginStart.ToFloat();
      listMarkerInlineLevelBox.SetLogicalLeft(logicalLeft);
    }
  }

  void LineBoxBuilder::AdjustMarginStartForListMarker(const ElementBox &listMarkerBox,
                                                      LayoutUnit nestedListMarkerMarginStart,
                                                      InlineLayoutUnit rootInlineBoxOffset) const noexcept
  {
    if (!nestedListMarkerMarginStart && !rootInlineBoxOffset)
    {
      return;
    }
    auto &listMarkerGeometry =
      const_cast<InlineFormattingContext &>(GetFormattingContext()).GeometryForBox(listMarkerBox);
    // Make sure that the line content does not get pulled in to logical left direction due to
    // the large negative margin (i.e. this ensures that logical left of the list content stays at the line
    // start)
    listMarkerGeometry.SetHorizontalMargin(
      {listMarkerGeometry.MarginStart() + nestedListMarkerMarginStart - LayoutUnit {rootInlineBoxOffset},
       listMarkerGeometry.MarginEnd() - nestedListMarkerMarginStart + LayoutUnit {rootInlineBoxOffset}});
  }
}
