#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineContentAligner.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingContext.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::layout
{
  namespace
  {
    inline void ShiftDisplayBox(inline_display::Box &displayBox, InlineLayoutUnit offset,
                                InlineFormattingContext &inlineFormattingContext) noexcept
    {
      if (!offset)
      {
        return;
      }
      auto writingMode = inlineFormattingContext.Root().Style().GetWritingMode();
      if (writingMode.IsLineOverLeft())
      {
        displayBox.MoveVertically(-offset);
      }
      else if (writingMode.IsHorizontal())
      {
        displayBox.MoveHorizontally(offset);
      }
      else
      {
        displayBox.MoveVertically(offset);
      }
      if (!displayBox.IsTextOrSoftLineBreak() && !displayBox.IsRootInlineBox())
      {
        inlineFormattingContext.GeometryForBox(displayBox.LayoutBox()).MoveHorizontally(LayoutUnit {offset});
      }
    }

    inline void ExpandInlineBox(InlineLayoutUnit expansion, inline_display::Box &displayBox,
                                InlineFormattingContext &inlineFormattingContext) noexcept
    {
      if (!displayBox.IsInlineBox())
      {
        krys_debug_assert(false);
        return;
      }
      if (!expansion)
      {
        return;
      }
      auto writingMode = inlineFormattingContext.Root().GetWritingMode();
      if (writingMode.IsHorizontal())
      {
        displayBox.ExpandHorizontally(expansion);
      }
      else
      {
        displayBox.ExpandVertically(expansion);
      }
      auto &boxGeometry = inlineFormattingContext.GeometryForBox(displayBox.LayoutBox());
      if (writingMode.IsLineOverLeft())
      {
        displayBox.SetTop(displayBox.Top() - expansion);
        boxGeometry.SetLeft(BoxGeometry::BorderBoxLeft(boxGeometry) - LayoutUnit {expansion});
      }
      boxGeometry.SetContentBoxWidth(boxGeometry.ContentBoxWidth() + LayoutUnit {expansion});
    }

    KRYS_NODISCARD inline InlineLayoutUnit
      AlignmentOffset(const Box &layoutBox,
                      const HashMap<const Box *, InlineLayoutUnit> &alignmentOffsetList) noexcept
    {
      auto alignmentOffsetEntry = alignmentOffsetList.find(&layoutBox);
      return alignmentOffsetEntry != alignmentOffsetList.end() ? alignmentOffsetEntry->second : 0.f;
    }

    struct InlineBoxIndexAndExpansion
    {
      size_t Index {0};
      InlineLayoutUnit Expansion {0.f};
    };
    KRYS_NODISCARD InlineBoxIndexAndExpansion
      ExpandInlineBoxWithDescendants(size_t inlineBoxIndex, inline_display::Boxes &displayBoxes,
                                     const HashMap<const Box *, InlineLayoutUnit> &alignmentOffsetList,
                                     InlineFormattingContext &inlineFormattingContext) noexcept
    {
      if (inlineBoxIndex >= displayBoxes.size() || !displayBoxes[inlineBoxIndex].IsInlineBox())
      {
        krys_debug_assert(false);
        return {inlineBoxIndex, {}};
      }
      auto &inlineBox = displayBoxes[inlineBoxIndex].LayoutBox();
      auto descendantExpansion = InlineLayoutUnit {0.f};
      size_t index = inlineBoxIndex + 1;
      while (index < displayBoxes.size()
             && static_cast<const Box *>(&displayBoxes[index].LayoutBox().Parent()) == &inlineBox)
      {
        if (displayBoxes[index].IsInlineBox())
        {
          auto indexAndExpansion =
            ExpandInlineBoxWithDescendants(index, displayBoxes, alignmentOffsetList, inlineFormattingContext);
          index = indexAndExpansion.Index;
          descendantExpansion += indexAndExpansion.Expansion;
          continue;
        }
        ++index;
      }
      auto totalExpansion = 2 * AlignmentOffset(inlineBox, alignmentOffsetList) + descendantExpansion;
      if (inlineBoxIndex)
      {
        // Root inline box has the correct (inflated) logical width.
        ExpandInlineBox(totalExpansion, displayBoxes[inlineBoxIndex], inlineFormattingContext);
      }
      return {index, totalExpansion};
    }

    struct BaseIndexAndOffset
    {
      size_t Index {0};
      InlineLayoutUnit Offset {0.f};
    };
    KRYS_NODISCARD BaseIndexAndOffset ShiftRubyBaseContentByAlignmentOffset(
      BaseIndexAndOffset baseIndexAndOffset, inline_display::Boxes &displayBoxes,
      const HashMap<const Box *, InlineLayoutUnit> &alignmentOffsetList,
      InlineContentAligner::AdjustContentOnlyInsideRubyBase adjustContentOnlyInsideRubyBase,
      InlineFormattingContext &inlineFormattingContext) noexcept
    {
      auto baseIndex = baseIndexAndOffset.Index;
      if (baseIndex >= displayBoxes.size() || !displayBoxes[baseIndex].LayoutBox().IsRubyBase())
      {
        krys_debug_assert(false);
        return {baseIndexAndOffset.Index, {}};
      }

      // Shift base content within the base (no resize) as part of the alignment process.
      auto &rootBox = inlineFormattingContext.Root();
      auto &rubyBaseBox = displayBoxes[baseIndex].LayoutBox();
      auto baseOffset = baseIndexAndOffset.Offset;
      auto baseContentOffset = AlignmentOffset(rubyBaseBox, alignmentOffsetList);
      size_t baseContentIndex = baseIndex + 1;

      while (baseContentIndex < displayBoxes.size())
      {
        auto &displayBox = displayBoxes[baseContentIndex];
        auto &layoutBox = displayBox.LayoutBox();
        auto isInsideCurrentRubyBase = [&]
        {
          // Ruby content tends to produce flat structures.
          for (const Box *ancestor = &layoutBox.Parent(); ancestor; ancestor = &ancestor->Parent())
          {
            if (ancestor == &rubyBaseBox)
            {
              return true;
            }
            if (ancestor->IsRubyBase() || ancestor->IsRuby() || ancestor == &rootBox)
            {
              return false;
            }
          }
          return false;
        };
        if (!isInsideCurrentRubyBase())
        {
          break;
        }
        if (!layoutBox.IsRubyAnnotationBox())
        {
          ShiftDisplayBox(displayBox, baseOffset + baseContentOffset, inlineFormattingContext);
        }
        if (layoutBox.IsRubyBase())
        {
          auto baseEndIndexAndAlignment = ShiftRubyBaseContentByAlignmentOffset(
            {baseContentIndex, baseOffset + baseContentOffset}, displayBoxes, alignmentOffsetList,
            adjustContentOnlyInsideRubyBase, inlineFormattingContext);
          baseContentIndex = baseEndIndexAndAlignment.Index;
          if (adjustContentOnlyInsideRubyBase == InlineContentAligner::AdjustContentOnlyInsideRubyBase::No)
          {
            baseOffset += baseEndIndexAndAlignment.Offset;
          }
          continue;
        }
        ++baseContentIndex;
      }
      auto accumulatedOffset = 2 * baseContentOffset;
      if (adjustContentOnlyInsideRubyBase == InlineContentAligner::AdjustContentOnlyInsideRubyBase::No)
      {
        accumulatedOffset += baseOffset;
      }
      return {baseContentIndex, accumulatedOffset};
    }

    enum class IgnoreRubyRange : bool
    {
      No,
      Yes
    };
    void ComputedExpansions(const Line::RunList &runs, IndexRange runRange,
                            size_t hangingTrailingWhitespaceLength, ExpansionInfo &expansionInfo,
                            IgnoreRubyRange ignoreRuby) noexcept
    {
      // Collect and distribute the expansion opportunities.
      expansionInfo.OpportunityCount = 0;
      auto rangeSize = runRange.End() - runRange.Begin();
      if (rangeSize > runs.size())
      {
        krys_debug_assert(false);
        return;
      }
      expansionInfo.OpportunityList.resize(rangeSize);
      expansionInfo.BehaviorList.resize(rangeSize);
      auto lastExpansionIndexWithContent = Maybe<size_t> {};

      // Line start behaves as if we had an expansion here (i.e. first runs should not start with allowing
      // left expansion).
      auto runIsAfterExpansion = true;
      auto lastTextRunIndexForTrimming = [&]() -> Maybe<size_t>
      {
        if (!hangingTrailingWhitespaceLength)
        {
          return {};
        }
        for (auto index = runs.size(); index--;)
        {
          if (runs[index].IsText())
          {
            return index;
          }
        }
        return {};
      }();
      for (size_t index = 0; index < rangeSize; ++index)
      {
        auto runIndex = [&]
        {
          return runRange.Begin() + index;
        };
        auto skipRubyContentIfApplicable = [&]
        {
          auto &rubyBox = runs[runIndex()].LayoutBox();
          if (ignoreRuby == IgnoreRubyRange::No || !rubyBox.IsRuby())
          {
            return;
          }
          runIsAfterExpansion = false;
          for (; index < rangeSize; ++index)
          {
            expansionInfo.BehaviorList[index] = ExpansionBehavior::DefaultBehavior();
            expansionInfo.OpportunityList[index] = 0;
            auto &run = runs[runIndex()];
            if (run.IsInlineBoxEnd() && &run.LayoutBox() == &rubyBox)
            {
              ++index;
              return;
            }
          }
        };
        skipRubyContentIfApplicable();
        if (index >= rangeSize)
        {
          break;
        }
        auto &run = runs[runIndex()];

        auto expansionBehavior = ExpansionBehavior::DefaultBehavior();
        size_t expansionOpportunitiesInRun = 0;

        if (run.IsText())
        {
          if (run.HasTextCombine())
          {
            expansionBehavior = ExpansionBehavior::ForbidAll();
          }
          else
          {
            expansionBehavior.Left =
              runIsAfterExpansion ? ExpansionBehavior::Behavior::Forbid : ExpansionBehavior::Behavior::Allow;
            expansionBehavior.Right = ExpansionBehavior::Behavior::Allow;
            auto &textContent = *run.TextContent();
            auto length = textContent.Length;
            if (lastTextRunIndexForTrimming && runIndex() == *lastTextRunIndexForTrimming)
            {
              // Trailing hanging whitespace sequence is ignored when computing the expansion opportunities.
              length -= hangingTrailingWhitespaceLength;
            }
            auto content = utf8_stringview {Downcast<InlineTextBox>(run.LayoutBox()).Content()}.substr(
              textContent.Start, length);
            std::tie(expansionOpportunitiesInRun, runIsAfterExpansion) =
              FontCascade::ExpansionOpportunityCount(content, run.InlineDirection(), expansionBehavior);
          }
        }
        else if (run.IsAtomicInlineBox())
        {
          runIsAfterExpansion = false;
        }

        expansionInfo.BehaviorList[index] = expansionBehavior;
        expansionInfo.OpportunityList[index] = expansionOpportunitiesInRun;
        expansionInfo.OpportunityCount += expansionOpportunitiesInRun;

        if (run.IsText() || run.IsAtomicInlineBox())
        {
          lastExpansionIndexWithContent = index;
        }
      }
      // Forbid right expansion in the last run to prevent trailing expansion at the end of the line.
      if (lastExpansionIndexWithContent && expansionInfo.OpportunityList[*lastExpansionIndexWithContent])
      {
        expansionInfo.BehaviorList[*lastExpansionIndexWithContent].Right =
          ExpansionBehavior::Behavior::Forbid;
        if (runIsAfterExpansion)
        {
          // When the last run has an after expansion (e.g. CJK ideograph) we need to remove this trailing
          // expansion opportunity. Note that this is not about trailing collapsible whitespace as at this
          // point we trimmed them all.
          krys_debug_assert(expansionInfo.OpportunityCount
                            && expansionInfo.OpportunityList[*lastExpansionIndexWithContent]);
          --expansionInfo.OpportunityCount;
          --expansionInfo.OpportunityList[*lastExpansionIndexWithContent];
        }
      }
    }
  }

  InlineLayoutUnit InlineContentAligner::ApplyExpansionOnRange(Line::RunList &runs, IndexRange range,
                                                               const ExpansionInfo &expansion,
                                                               InlineLayoutUnit spaceToDistribute) noexcept
  {
    krys_debug_assert(spaceToDistribute > 0);
    krys_debug_assert(expansion.OpportunityCount);
    // Distribute the extra space.
    auto expansionToDistribute =
      spaceToDistribute / static_cast<InlineLayoutUnit>(expansion.OpportunityCount);
    auto accumulatedExpansion = InlineLayoutUnit {};
    auto rangeSize = range.Distance();
    if (range.End() > runs.size())
    {
      krys_debug_assert(false);
      return {};
    }
    for (size_t index = 0; index < rangeSize; ++index)
    {
      auto &run = runs[range.Begin() + index];
      // Move runs by the accumulated expansion first
      run.MoveHorizontally(accumulatedExpansion);
      // and expand.
      auto computedExpansion =
        expansionToDistribute * static_cast<InlineLayoutUnit>(expansion.OpportunityList[index]);
      run.SetExpansion({expansion.BehaviorList[index], computedExpansion});
      run.ShrinkHorizontally(-computedExpansion);
      accumulatedExpansion += computedExpansion;
    }
    // Content grows as runs expand.
    return accumulatedExpansion;
  }

  InlineLayoutUnit
    InlineContentAligner::ApplyTextAlignJustify(Line::RunList &runs, InlineLayoutUnit spaceToDistribute,
                                                size_t hangingTrailingWhitespaceLength) noexcept
  {
    if (runs.empty())
    {
      krys_debug_assert(false);
      return {};
    }

    if (spaceToDistribute <= 0)
    {
      return {};
    }

    auto expansion = ExpansionInfo {};
    auto fullRange = IndexRange {0, runs.size()};
    ComputedExpansions(runs, fullRange, hangingTrailingWhitespaceLength, expansion, IgnoreRubyRange::Yes);
    // Anything to distribute?
    if (!expansion.OpportunityCount)
    {
      return {};
    }
    return ApplyExpansionOnRange(runs, fullRange, expansion, spaceToDistribute);
  }

  InlineLayoutUnit InlineContentAligner::ApplyRubyAlign(RubyAlign rubyAlign, Line::RunList &runs,
                                                        IndexRange range,
                                                        InlineLayoutUnit spaceToDistribute) noexcept
  {
    if (runs.empty())
    {
      krys_debug_assert(false);
      return {};
    }

    if (spaceToDistribute <= 0)
    {
      return {};
    }

    auto rangeHasInlineContent = [&]
    {
      if (!range.Distance())
      {
        return false;
      }
      for (auto index = range.Begin(); index < range.End(); ++index)
      {
        auto &run = runs[index];
        if (!run.IsInlineBox() && !run.IsOpaque())
        {
          return true;
        }
      }
      return false;
    };
    if (!rangeHasInlineContent())
    {
      return {};
    }

    switch (rubyAlign)
    {
      case RubyAlign::Start:  return {};
      case RubyAlign::Center: return spaceToDistribute / 2;
      case RubyAlign::SpaceBetween:
      {
        // The ruby content expands as defined for normal text justification (as defined by text-justify),
        // except that if there are no justification opportunities the content is centered.
        auto expansion = ExpansionInfo {};
        ComputedExpansions(runs, range, {}, expansion, IgnoreRubyRange::No);
        // Anything to distribute?
        if (!expansion.OpportunityCount)
        {
          return spaceToDistribute / 2;
        }
        (void)ApplyExpansionOnRange(runs, range, expansion, spaceToDistribute);
        return {};
      }
      case RubyAlign::SpaceAround:
      {
        auto expansion = ExpansionInfo {};
        ComputedExpansions(runs, range, {}, expansion, IgnoreRubyRange::No);
        // Anything to distribute?
        if (!expansion.OpportunityCount)
        {
          return spaceToDistribute / 2;
        }
        // As for space-between except that there exists an extra justification opportunities whose space is
        // distributed half before and half after the ruby content.
        auto extraExpansionOpportunitySpace =
          spaceToDistribute / static_cast<InlineLayoutUnit>(expansion.OpportunityCount + 1);
        (void)ApplyExpansionOnRange(runs, range, expansion,
                                    spaceToDistribute - extraExpansionOpportunitySpace);
        return extraExpansionOpportunitySpace / 2;
      }
    }
    return {};
  }

  void InlineContentAligner::ApplyRubyBaseAlignmentOffset(
    inline_display::Boxes &displayBoxes, const HashMap<const Box *, InlineLayoutUnit> &alignmentOffsetList,
    AdjustContentOnlyInsideRubyBase adjustContentOnlyInsideRubyBase,
    InlineFormattingContext &inlineFormattingContext) noexcept
  {
    krys_debug_assert(!alignmentOffsetList.empty());

    auto contentOffset = InlineLayoutUnit {0.f};
    for (size_t index = 0; index < displayBoxes.size();)
    {
      auto &displayBox = displayBoxes[index];

      if (adjustContentOnlyInsideRubyBase == AdjustContentOnlyInsideRubyBase::No)
      {
        ShiftDisplayBox(displayBox, contentOffset, inlineFormattingContext);
      }

      if (displayBox.LayoutBox().IsRubyBase())
      {
        auto baseEndIndexAndAlignment =
          ShiftRubyBaseContentByAlignmentOffset({index, contentOffset}, displayBoxes, alignmentOffsetList,
                                                adjustContentOnlyInsideRubyBase, inlineFormattingContext);
        index = baseEndIndexAndAlignment.Index;
        if (adjustContentOnlyInsideRubyBase == AdjustContentOnlyInsideRubyBase::No)
        {
          contentOffset = baseEndIndexAndAlignment.Offset;
        }
        continue;
      }
      ++index;
    }

    if (adjustContentOnlyInsideRubyBase == AdjustContentOnlyInsideRubyBase::No)
    {
      (void)ExpandInlineBoxWithDescendants(0, displayBoxes, alignmentOffsetList, inlineFormattingContext);
    }
  }

  void InlineContentAligner::ApplyRubyAnnotationAlignmentOffset(
    inline_display::Boxes &displayBoxes, InlineLayoutUnit alignmentOffset,
    InlineFormattingContext &inlineFormattingContext) noexcept
  {
    for (size_t index = 0; index < displayBoxes.size(); ++index)
    {
      ShiftDisplayBox(displayBoxes[index], alignmentOffset, inlineFormattingContext);
    }
  }
}
