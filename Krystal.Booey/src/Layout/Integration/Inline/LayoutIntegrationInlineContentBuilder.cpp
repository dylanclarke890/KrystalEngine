#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationInlineContentBuilder.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Invalidation/InlineDamage.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationInlineContent.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Text/Unicode.hpp"

namespace krys::boo::layout_integration
{
  namespace
  {
    KRYS_NODISCARD inline float EndPaddingQuirkValue(const RenderBlockFlow &flow) noexcept
    {
      // FIXME: The caret width at the end of a root editable element (endPaddingWidthForCaret), once there's
      // editing.
      auto endPadding = flow.HasNonVisibleOverflow() ? flow.PaddingEnd() : 0_lu;
      return endPadding.ToFloat();
    }

    KRYS_NODISCARD Pair<float, float> GlyphOverflowInInlineDirection(size_t firstTextBoxIndex,
                                                                     size_t lastTextBoxIndex,
                                                                     const inline_display::Boxes &boxes,
                                                                     const FloatRect &inkOverflowRect,
                                                                     bool isLeftToRightDirection) noexcept
    {
      // FIXME: This should be on the text box level and taking all characters into account (maybe consider
      // utilizing the measuring pass if turns out to be a perf hit)
      if (firstTextBoxIndex >= boxes.size() || lastTextBoxIndex >= boxes.size())
      {
        krys_debug_assert(false);
        return {};
      }

      auto bounds = [&](const inline_display::Box &textBox, bool isLeading)
      {
        auto textContent = textBox.GetText().RenderedContent();
        if (textContent.empty())
        {
          krys_debug_assert(false);
          return FloatRect {};
        }
        auto character = [&]
        {
          if (isLeading)
          {
            return text::DecodeUTF8(Span<const char8>(textContent.data(), textContent.size())).CodePoint;
          }
          auto position = textContent.size() - 1;
          while (position && text::IsTrailingUTF8(textContent[position]))
          {
            --position;
          }
          return text::DecodeUTF8(
                   Span<const char8>(textContent.data() + position, textContent.size() - position))
            .CodePoint;
        }();
        auto &fontCascade = textBox.Style().GetFontCascade();
        auto glyphData = fontCascade.GlyphDataForCharacter(character, !isLeftToRightDirection);
        auto *font = glyphData.Font ? glyphData.Font.get() : fontCascade.PrimaryFont().get();
        return font->BoundsForGlyph(glyphData.Glyph);
      };

      auto leadingOverflow = [&]
      {
        auto &firstTextBox = boxes[firstTextBoxIndex];
        krys_debug_assert(firstTextBox.IsText());
        if (Downcast<layout::InlineTextBox>(firstTextBox.LayoutBox()).CanUseSimpleFontCodePath())
        {
          return 0.f;
        }
        if (auto boundsX = bounds(firstTextBox, true).X(); boundsX < 0)
        {
          return std::max(0.f, inkOverflowRect.X() - (firstTextBox.Left() + boundsX));
        }
        return 0.f;
      };

      auto trailingOverflow = [&]
      {
        auto &lastTextBox = boxes[lastTextBoxIndex];
        krys_debug_assert(lastTextBox.IsText());
        if (Downcast<layout::InlineTextBox>(lastTextBox.LayoutBox()).CanUseSimpleFontCodePath())
        {
          return 0.f;
        }
        if (auto boundsMaxX = bounds(lastTextBox, false).MaxX(); boundsMaxX > lastTextBox.Width())
        {
          return std::max(0.f, (lastTextBox.Left() + boundsMaxX) - inkOverflowRect.MaxX());
        }
        return 0.f;
      };

      return {leadingOverflow(), trailingOverflow()};
    }
  }

  InlineContentBuilder::InlineContentBuilder(const RenderBlockFlow &blockFlow) noexcept
      : _blockFlow(blockFlow)
  {
  }

  FloatRect InlineContentBuilder::Build(layout::InlineLayoutResult &&layoutResult,
                                        InlineContent &inlineContent,
                                        const layout::InlineDamage *lineDamage) const noexcept
  {
    inlineContent.ReleaseCaches();
    auto damageRect = FloatRect {};

    if (layoutResult.ContentRange == layout::InlineLayoutResult::Range::Full)
    {
      for (auto &line : inlineContent.DisplayContent().LineList)
      {
        damageRect.Unite(line.InkOverflow());
      }

      inlineContent.DisplayContent().Set(krys::move(layoutResult.DisplayContent));
      AdjustDisplayLines(inlineContent, 0);

      for (auto &line : inlineContent.DisplayContent().LineList)
      {
        damageRect.Unite(line.InkOverflow());
      }
    }
    else
    {
      damageRect = HandlePartialDisplayContentUpdate(krys::move(layoutResult), inlineContent, lineDamage);
    }

    ComputeIsFirstIsLastBoxAndBidiReorderingForInlineContent(inlineContent.DisplayContent().BoxList);
    return damageRect;
  }

  void InlineContentBuilder::UpdateLineOverflow(InlineContent &inlineContent) const noexcept
  {
    AdjustDisplayLines(inlineContent, 0);
  }

  void InlineContentBuilder::AdjustDisplayLines(InlineContent &inlineContent,
                                                size_t startIndex) const noexcept
  {
    auto &lines = inlineContent.DisplayContent().LineList;
    auto &boxes = inlineContent.DisplayContent().BoxList;
    size_t boxIndex = !startIndex ? 0 : lines[startIndex - 1].LastBoxIndex() + 1;

    auto &rootBoxStyle = _blockFlow.Style();
    auto isLeftToRightInlineDirection = rootBoxStyle.IsLeftToRightDirection();
    auto isHorizontalWritingMode = rootBoxStyle.GetWritingMode().IsHorizontal();

    auto blockScrollableOverflowRect = FloatRect {};
    auto blockInkOverflowRect = FloatRect {};

    for (size_t lineIndex = 0; lineIndex < startIndex; ++lineIndex)
    {
      auto &line = lines[lineIndex];
      blockScrollableOverflowRect.Unite(line.ScrollableOverflow());
      blockInkOverflowRect.Unite(line.InkOverflow());
    }

    for (size_t lineIndex = startIndex; lineIndex < lines.size(); ++lineIndex)
    {
      auto &line = lines[lineIndex];
      auto lineScrollableOverflowRect = line.ContentOverflow();
      auto adjustOverflowLogicalWidthWithBlockFlowQuirk = [&]
      {
        auto scrollableOverflowLogicalWidth =
          isHorizontalWritingMode ? lineScrollableOverflowRect.Width() : lineScrollableOverflowRect.Height();
        if (!isLeftToRightInlineDirection && line.ContentLogicalWidth() > scrollableOverflowLogicalWidth)
        {
          // The only time when scrollable overflow here could be shorter than
          // the content width is when hanging RTL trailing content is applied (and ignored as scrollable
          // overflow. See LineBoxBuilder::Build.
          return;
        }
        auto adjustedOverflowLogicalWidth = line.ContentLogicalWidth() + EndPaddingQuirkValue(_blockFlow);
        if (adjustedOverflowLogicalWidth > scrollableOverflowLogicalWidth)
        {
          auto overflowValue = adjustedOverflowLogicalWidth - scrollableOverflowLogicalWidth;
          if (isHorizontalWritingMode)
          {
            if (isLeftToRightInlineDirection)
            {
              lineScrollableOverflowRect.ShiftMaxXEdgeBy(overflowValue);
            }
            else
            {
              lineScrollableOverflowRect.ShiftXEdgeBy(-overflowValue);
            }
          }
          else if (isLeftToRightInlineDirection)
          {
            lineScrollableOverflowRect.ShiftMaxYEdgeBy(overflowValue);
          }
          else
          {
            lineScrollableOverflowRect.ShiftYEdgeBy(-overflowValue);
          }
        }
      };
      adjustOverflowLogicalWidthWithBlockFlowQuirk();

      if (line.HasContentfulInFlowBox() && !line.HasBlockLevelBox())
      {
        inlineContent.SetHasPaintedInlineLevelBoxes();
      }

      auto firstBoxIndex = boxIndex;
      auto lineInkOverflowRect = lineScrollableOverflowRect;
      // Collect overflow from boxes.
      // Note while we compute ink overflow for all type of boxes including atomic inline level boxes (e.g.
      // <iframe> <img>) as part of constructing display boxes (see InlineDisplayContentBuilder)
      // RenderBlockFlow expects visual overflow. Visual overflow propagation is slightly different from ink
      // overflow when it comes to renderers with self painting layers. -and for now we consult atomic
      // renderers for such visual overflow which is not how we are supposed to do in LFC. (visual overflow is
      // computed during their Layout() call which we issue right before running inline layout)
      auto firstTextBoxIndex = Maybe<size_t> {};
      auto lastTextBoxIndex = Maybe<size_t> {};
      for (; boxIndex < boxes.size() && boxes[boxIndex].LineIndex() == lineIndex; ++boxIndex)
      {
        auto &box = boxes[boxIndex];
        if (box.IsRootInlineBox() || box.IsEllipsis() || box.IsLineBreak())
        {
          continue;
        }

        if (box.IsText())
        {
          lineInkOverflowRect.Unite(box.InkOverflow());
          if (box.IsVisible() && !box.GetText().RenderedContent().empty())
          {
            firstTextBoxIndex = firstTextBoxIndex.value_or(boxIndex);
            lastTextBoxIndex = boxIndex;
          }
          continue;
        }

        if (box.IsAtomicInlineBox() || box.IsBlockLevelBox())
        {
          if (box.IsBlockLevelBox())
          {
            inlineContent.SetHasBlockLevelBoxes();
          }
          auto &renderer = Downcast<RenderBox>(*box.LayoutBox().RendererForIntegration());
          if (!renderer.HasSelfPaintingLayer())
          {
            auto childInkOverflow =
              renderer.LogicalVisualOverflowRectForPropagation(renderer.Parent()->GetWritingMode());
            childInkOverflow.Move(LayoutUnit {box.Left()}, LayoutUnit {box.Top()});
            lineInkOverflowRect.Unite(FloatRect {childInkOverflow});
          }
          // FIXME: Form controls clip their content (hasControlClip), once there are form controls.
          auto childScrollableOverflow =
            renderer.LayoutOverflowRectForPropagation(renderer.Parent()->GetWritingMode());
          childScrollableOverflow.Move(LayoutUnit {box.Left()}, LayoutUnit {box.Top()});
          lineScrollableOverflowRect.Unite(FloatRect {childScrollableOverflow});
          continue;
        }

        if (box.IsInlineBox())
        {
          bool hasSelfPaintingLayer =
            Downcast<RenderLayerModelObject>(*box.LayoutBox().RendererForIntegration())
              .HasSelfPaintingLayer();
          if (!hasSelfPaintingLayer)
          {
            lineInkOverflowRect.Unite(box.InkOverflow());
          }
          if (line.HasBlockLevelBox())
          {
            if (hasSelfPaintingLayer || box.LayoutBox().Style().HasOpacity()
                || box.LayoutBox().Style().HasOutline())
            {
              // See if the inline box has properties that affect block-in-inline painting.
              inlineContent.SetHasPaintedInlineLevelBoxes();
            }
          }
          continue;
        }
      }

      if (firstTextBoxIndex && lastTextBoxIndex)
      {
        auto [leadingOverflow, trailingOverflow] =
          GlyphOverflowInInlineDirection(*firstTextBoxIndex, *lastTextBoxIndex, boxes, lineInkOverflowRect,
                                         line.IsLeftToRightInlineDirection());
        lineInkOverflowRect.Inflate(leadingOverflow, 0.f, trailingOverflow, 0.f);
      }

      line.SetScrollableOverflow(lineScrollableOverflowRect);
      line.SetInkOverflow(lineInkOverflowRect);
      line.SetFirstBoxIndex(firstBoxIndex);

      blockScrollableOverflowRect.Unite(lineScrollableOverflowRect);
      blockInkOverflowRect.Unite(lineInkOverflowRect);

      krys_debug_assert(line.BoxCount() == boxIndex - firstBoxIndex);

      if (lineIndex)
      {
        auto &lastInkOverflow = lines[lineIndex - 1].InkOverflow();
        if (lineInkOverflowRect.Y() <= lastInkOverflow.Y()
            || lastInkOverflow.MaxY() >= lineInkOverflowRect.MaxY())
        {
          inlineContent.SetHasMultilinePaintOverlap();
        }
      }
    }

    inlineContent.SetScrollableOverflow(blockScrollableOverflowRect);
    inlineContent.SetInkOverflow(blockInkOverflowRect);
  }

  void InlineContentBuilder::ComputeIsFirstIsLastBoxAndBidiReorderingForInlineContent(
    inline_display::Boxes &boxes) const noexcept
  {
    if (boxes.empty())
    {
      // Line clamp may produce a completely empty IFC.
      return;
    }

    HashMap<const layout::Box *, size_t> lastDisplayBoxForLayoutBoxIndexes;
    lastDisplayBoxForLayoutBoxIndexes.reserve(boxes.size() - 1);

    krys_debug_assert(boxes[0].IsRootInlineBox());
    boxes[0].SetIsFirstForLayoutBox();
    size_t lastRootInlineBoxIndex = 0;

    for (size_t index = 1; index < boxes.size(); ++index)
    {
      auto &displayBox = boxes[index];
      if (displayBox.IsRootInlineBox())
      {
        lastRootInlineBoxIndex = index;
        continue;
      }
      auto &layoutBox = displayBox.LayoutBox();
      if (Is<layout::InlineTextBox>(layoutBox) && displayBox.BidiLevel() != UBIDI_DEFAULT_LTR)
      {
        Downcast<RenderText>(*layoutBox.RendererForIntegration()).SetNeedsVisualReordering();
      }

      auto [entry, isNewEntry] = lastDisplayBoxForLayoutBoxIndexes.insert_or_assign(&layoutBox, index);
      if (isNewEntry)
      {
        displayBox.SetIsFirstForLayoutBox();
      }
    }
    for (auto &[layoutBox, index] : lastDisplayBoxForLayoutBoxIndexes)
    {
      boxes[index].SetIsLastForLayoutBox();
    }
    boxes[lastRootInlineBoxIndex].SetIsLastForLayoutBox();
  }
  FloatRect InlineContentBuilder::HandlePartialDisplayContentUpdate(
    layout::InlineLayoutResult &&layoutResult, InlineContent &inlineContent,
    const layout::InlineDamage *lineDamage) const noexcept
  {
    auto firstDamagedLineIndex = [&]() -> Maybe<size_t>
    {
      auto &displayContentFromPreviousLayout = inlineContent.DisplayContent();
      if (!lineDamage || !lineDamage->LayoutStartPosition()
          || !displayContentFromPreviousLayout.LineList.size())
      {
        return {};
      }
      auto canidateLineIndex = lineDamage->LayoutStartPosition()->LineIndex;
      if (canidateLineIndex >= displayContentFromPreviousLayout.LineList.size())
      {
        krys_debug_assert(false);
        return {};
      }
      if (layoutResult.DisplayContent.BoxList.size()
          && canidateLineIndex > layoutResult.DisplayContent.BoxList[0].LineIndex())
      {
        // We should never generate lines _before_ the damaged line.
        krys_debug_assert(false);
        return {};
      }
      return canidateLineIndex;
    }();

    auto firstDamagedBoxIndex = [&]() -> Maybe<size_t>
    {
      auto &displayContentFromPreviousLayout = inlineContent.DisplayContent();
      return firstDamagedLineIndex
               ? Maybe<size_t> {displayContentFromPreviousLayout.LineList[*firstDamagedLineIndex]
                                  .FirstBoxIndex()}
               : null;
    }();

    auto numberOfDamagedLines = [&]() -> Maybe<size_t>
    {
      if (!firstDamagedLineIndex)
      {
        return {};
      }
      auto &displayContentFromPreviousLayout = inlineContent.DisplayContent();
      krys_debug_assert(layoutResult.ContentRange != layout::InlineLayoutResult::Range::Full);
      auto canidateLineCount = layoutResult.ContentRange == layout::InlineLayoutResult::Range::FullFromDamage
                                 ? displayContentFromPreviousLayout.LineList.size() - *firstDamagedLineIndex
                                 : layoutResult.DisplayContent.LineList.size();

      if (*firstDamagedLineIndex + canidateLineCount > displayContentFromPreviousLayout.LineList.size())
      {
        krys_debug_assert(false);
        return {};
      }
      return canidateLineCount;
    }();

    auto numberOfDamagedBoxes = [&]() -> Maybe<size_t>
    {
      if (!firstDamagedLineIndex || !numberOfDamagedLines || !firstDamagedBoxIndex)
      {
        return {};
      }
      auto &displayContentFromPreviousLayout = inlineContent.DisplayContent();
      krys_debug_assert(*firstDamagedLineIndex + *numberOfDamagedLines
                        <= displayContentFromPreviousLayout.LineList.size());
      size_t boxCount = 0;
      for (size_t i = 0; i < *numberOfDamagedLines; ++i)
      {
        boxCount += displayContentFromPreviousLayout.LineList[*firstDamagedLineIndex + i].BoxCount();
      }
      krys_debug_assert(boxCount);
      return boxCount;
    }();

    if (!firstDamagedLineIndex || !numberOfDamagedLines || !firstDamagedBoxIndex || !numberOfDamagedBoxes)
    {
      krys_debug_assert(false);
      return {};
    }

    auto numberOfNewLines = layoutResult.DisplayContent.LineList.size();
    auto numberOfNewBoxes = layoutResult.DisplayContent.BoxList.size();

    auto damagedRect = FloatRect {};
    auto adjustDamagedRectWithLineRange = [&](size_t firstLineIndex, size_t lineCount)
    {
      auto &lines = inlineContent.DisplayContent().LineList;
      krys_debug_assert(firstLineIndex + lineCount <= lines.size());
      for (size_t i = 0; i < lineCount; ++i)
      {
        damagedRect.Unite(lines[firstLineIndex + i].InkOverflow());
      }
    };

    // Repaint the damaged content boundary.
    adjustDamagedRectWithLineRange(*firstDamagedLineIndex, *numberOfDamagedLines);

    switch (layoutResult.ContentRange)
    {
      case layout::InlineLayoutResult::Range::FullFromDamage:
      {
        auto &displayContent = inlineContent.DisplayContent();
        displayContent.Remove(*firstDamagedLineIndex, *numberOfDamagedLines, *firstDamagedBoxIndex,
                              *numberOfDamagedBoxes);
        displayContent.Append(krys::move(layoutResult.DisplayContent));
        break;
      }
      case layout::InlineLayoutResult::Range::PartialFromDamage:
      {
        auto &displayContent = inlineContent.DisplayContent();
        displayContent.Remove(*firstDamagedLineIndex, *numberOfDamagedLines, *firstDamagedBoxIndex,
                              *numberOfDamagedBoxes);
        displayContent.Insert(krys::move(layoutResult.DisplayContent), *firstDamagedLineIndex,
                              *firstDamagedBoxIndex);

        auto adjustCachedBoxIndexesIfNeeded = [&]
        {
          if (numberOfNewBoxes == *numberOfDamagedBoxes)
          {
            return;
          }
          auto firstCleanLineIndex = *firstDamagedLineIndex + *numberOfDamagedLines;
          auto &lines = displayContent.LineList;
          for (size_t cleanLineIndex = firstCleanLineIndex; cleanLineIndex < lines.size(); ++cleanLineIndex)
          {
            // The box count may have shrunk: the first box index moves by the difference either way.
            auto adjustedFirstBoxIndex =
              lines[cleanLineIndex].FirstBoxIndex() + numberOfNewBoxes - *numberOfDamagedBoxes;
            krys_debug_assert(adjustedFirstBoxIndex > 0);
            lines[cleanLineIndex].SetFirstBoxIndex(adjustedFirstBoxIndex);
          }
        };
        adjustCachedBoxIndexesIfNeeded();
        break;
      }
      case layout::InlineLayoutResult::Range::Full:
      default:                                      krys_debug_assert(false);
    }

    AdjustDisplayLines(inlineContent, *firstDamagedLineIndex);
    // Repaint the new content boundary.
    adjustDamagedRectWithLineRange(*firstDamagedLineIndex, numberOfNewLines);

    return damagedRect;
  }
}
