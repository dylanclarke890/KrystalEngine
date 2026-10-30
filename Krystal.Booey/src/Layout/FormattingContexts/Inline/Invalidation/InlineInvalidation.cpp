#include "Krystal.Booey/Layout/FormattingContexts/Inline/Invalidation/InlineInvalidation.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineSoftLineBreakItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineTextItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextBreakingPositionContext.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::layout
{
  struct InvalidatedLine
  {
    size_t Index {0};
    InlineItemPosition LeadingInlineItemPosition {};
    size_t DamagedLineIndex {0};
  };

  namespace
  {
    struct DamagedContent
    {
      const Box &LayoutBox;
      // Only text type of boxes may have offset. No offset also simply points to the end of the layout box.
      Maybe<size_t> Offset {};
      enum class Type : uint8
      {
        Insertion,
        Removal
      };
      Type ContentType {Type::Insertion};
    };

    KRYS_NODISCARD Maybe<size_t> DamagedLineIndex(const DamagedContent &damagedContent,
                                                  const inline_display::Boxes &displayBoxes) noexcept
    {
      krys_debug_assert(!displayBoxes.empty());

      if (displayBoxes.size() == 1)
      {
        // Let's return the first line on empty content (when only root inline display box is present).
        return displayBoxes.back().LineIndex();
      }

      auto lastDisplayBoxIndexForLayoutBox = [&]() -> Maybe<size_t>
      {
        for (auto index = displayBoxes.size(); index--;)
        {
          auto &displayBox = displayBoxes[index];
          if (&damagedContent.LayoutBox == &displayBox.LayoutBox())
          {
            return index;
          }
        }
        // FIXME: When the damaged layout box does not generate even one display box (all collapsed), we
        // should look at previous/next content to damage lines.
        return {};
      };
      auto lastDisplayBoxIndex = lastDisplayBoxIndexForLayoutBox();
      if (!lastDisplayBoxIndex)
      {
        // FIXME: Completely collapsed content should not trigger full invalidation.
        return {};
      }

      auto candidateLineIndex = [&](size_t damagedDisplayBoxIndex) -> Maybe<size_t>
      {
        if (!damagedDisplayBoxIndex || damagedDisplayBoxIndex >= displayBoxes.size())
        {
          krys_debug_assert(false);
          return {};
        }
        auto &damagedDisplayBox = displayBoxes[damagedDisplayBoxIndex];
        krys_debug_assert(&damagedDisplayBox.LayoutBox() == &damagedContent.LayoutBox);
        // In case of content deletion, we may need to damage the "previous" line.
        if (damagedContent.ContentType == DamagedContent::Type::Insertion || !damagedDisplayBox.LineIndex())
        {
          return damagedDisplayBox.LineIndex();
        }
        if (!displayBoxes[damagedDisplayBoxIndex - 1].IsRootInlineBox())
        {
          // There's more content in front, it's safe to stay on the current line.
          krys_debug_assert(displayBoxes[damagedDisplayBoxIndex - 1].LineIndex()
                            == damagedDisplayBox.LineIndex());
          return damagedDisplayBox.LineIndex();
        }
        auto damagedContentIsFirstContentOnLine =
          !damagedDisplayBox.IsText()
          || (damagedContent.Offset
              && (!*damagedContent.Offset || damagedDisplayBox.GetText().Start() == *damagedContent.Offset));
        return damagedContentIsFirstContentOnLine ? damagedDisplayBox.LineIndex() - 1
                                                  : damagedDisplayBox.LineIndex();
      };

      auto leadingIndexForDisplayBox = *lastDisplayBoxIndex;
      if (damagedContent.LayoutBox.IsLineBreakBox() || damagedContent.LayoutBox.IsReplacedBox())
      {
        return candidateLineIndex(leadingIndexForDisplayBox);
      }

      if (Is<InlineTextBox>(damagedContent.LayoutBox))
      {
        if (!damagedContent.Offset)
        {
          return candidateLineIndex(leadingIndexForDisplayBox);
        }

        if (*damagedContent.Offset > displayBoxes[leadingIndexForDisplayBox].GetText().End())
        {
          // Protect against text updater providing bogus damage offset (offset is _after_ the last display
          // box here). Let's just fallback to full invalidation.
          return {};
        }

        for (auto index = leadingIndexForDisplayBox; index > 0; --index)
        {
          auto &displayBox = displayBoxes[index];
          if (displayBox.IsRootInlineBox() || (displayBox.IsInlineBox() && !displayBox.IsFirstForLayoutBox()))
          {
            // Multi line damage includes line leading root inline display boxes (and maybe line spanning
            // inline boxes).
            continue;
          }
          if (&displayBoxes[index].LayoutBox() != &damagedContent.LayoutBox)
          {
            // We should always be able to find the damaged offset within this layout box.
            break;
          }
          krys_debug_assert(displayBox.IsTextOrSoftLineBreak());
          // Find out which display box has the damaged offset position.
          auto startOffset = displayBox.GetText().Start();
          if (startOffset <= *damagedContent.Offset)
          {
            // FIXME: Add support for leading inline box deletion too.
            return candidateLineIndex(index);
          }
          leadingIndexForDisplayBox = index;
        }
        // In some cases leading content does not produce a display box (collapsed whitespace), let's just use
        // the first display box.
        krys_debug_assert(leadingIndexForDisplayBox);
        return candidateLineIndex(leadingIndexForDisplayBox);
      }
      krys_debug_assert(false);
      return {};
    }

    KRYS_NODISCARD const inline_display::Box *
      LeadingContentDisplayForLineIndex(size_t lineIndex, const inline_display::Boxes &displayBoxes) noexcept
    {
      auto rootInlineBoxIndexOnLine = [&]() -> size_t
      {
        for (auto index = displayBoxes.size(); index--;)
        {
          auto &displayBox = displayBoxes[index];
          if (displayBox.LineIndex() == lineIndex && displayBox.IsRootInlineBox())
          {
            return index;
          }
        }
        krys_debug_assert(false);
        return 0;
      };

      for (auto firstContentDisplayBoxIndex = rootInlineBoxIndexOnLine() + 1;
           firstContentDisplayBoxIndex < displayBoxes.size(); ++firstContentDisplayBoxIndex)
      {
        auto &displayBox = displayBoxes[firstContentDisplayBoxIndex];
        if (displayBox.IsRootInlineBox())
        {
          return nullptr;
        }
        if (!displayBox.IsNonRootInlineBox() || displayBox.IsFirstForLayoutBox())
        {
          return &displayBox;
        }
      }
      return nullptr;
    }

    KRYS_NODISCARD Maybe<InlineItemPosition>
      InlineItemPositionForDisplayBox(const inline_display::Box &displayBox,
                                      const InlineItemList &inlineItemList) noexcept
    {
      krys_debug_assert(!inlineItemList.empty());

      auto lastInlineItemIndexForDisplayBox = [&]() -> Maybe<size_t>
      {
        for (size_t index = inlineItemList.size(); index--;)
        {
          if (&inlineItemList[index].LayoutBox() == &displayBox.LayoutBox()
              && !inlineItemList[index].IsInlineBoxEnd())
          {
            return index;
          }
        }
        return {};
      }();

      if (!lastInlineItemIndexForDisplayBox)
      {
        return {};
      }

      // This is our last InlineItem associated with this layout box. Let's find out
      // which previous InlineItem is at the beginning of this display box.
      auto inlineItemIndex = *lastInlineItemIndexForDisplayBox;
      if (!displayBox.IsTextOrSoftLineBreak())
      {
        return InlineItemPosition {inlineItemIndex, 0};
      }
      // 1. A display box could consist of one or more InlineItems.
      // 2. A display box could start at any offset inside an InlineItem.
      auto startOffset = displayBox.GetText().Start();
      while (true)
      {
        auto inlineTextItemRange = [&]() -> std::pair<size_t, size_t>
        {
          if (auto *inlineTextItem = DynamicDowncast<InlineTextItem>(inlineItemList[inlineItemIndex]))
          {
            return {inlineTextItem->Start(), inlineTextItem->End()};
          }
          if (auto *inlineSoftBreakItem =
                DynamicDowncast<InlineSoftLineBreakItem>(inlineItemList[inlineItemIndex]))
          {
            auto startPosition = inlineSoftBreakItem->Position();
            return {startPosition, startPosition + 1};
          }
          krys_debug_assert(false);
          return {};
        };
        auto [textRangeBegin, textRangeEnd] = inlineTextItemRange();
        if (textRangeBegin <= startOffset && textRangeEnd > startOffset)
        {
          return InlineItemPosition {inlineItemIndex, startOffset - textRangeBegin};
        }
        if (!inlineItemIndex--)
        {
          break;
        }
      }
      return {};
    }

    KRYS_NODISCARD Maybe<InlineItemPosition>
      InlineItemPositionForDamagedContentPosition(const DamagedContent &damagedContent,
                                                  InlineItemPosition candidatePosition,
                                                  const InlineItemList &inlineItemList) noexcept
    {
      krys_debug_assert(candidatePosition.Index < inlineItemList.size());
      if (!candidatePosition)
      {
        return candidatePosition;
      }
      if (!Is<InlineTextBox>(damagedContent.LayoutBox))
      {
        krys_debug_assert(!damagedContent.Offset);
        return candidatePosition;
      }
      auto &candidateInlineItem = inlineItemList[candidatePosition.Index];
      if (&candidateInlineItem.LayoutBox() != &damagedContent.LayoutBox
          || (!Is<InlineTextItem>(candidateInlineItem) && !Is<InlineSoftLineBreakItem>(candidateInlineItem)))
      {
        return candidatePosition;
      }
      if (!damagedContent.Offset)
      {
        // When damage points to "after" the layout box, whatever InlineItem we found is surely before the
        // damage.
        return candidatePosition;
      }

      auto startPosition = [&](const InlineItem &inlineItem) -> size_t
      {
        auto *inlineTextItem = DynamicDowncast<InlineTextItem>(inlineItem);
        return inlineTextItem ? inlineTextItem->Start()
                              : Downcast<InlineSoftLineBreakItem>(inlineItem).Position();
      };

      auto contentOffset = startPosition(candidateInlineItem) + candidatePosition.Offset;
      if (contentOffset < *damagedContent.Offset)
      {
        // The damaged content is after the start of this inline item.
        return candidatePosition;
      }
      // When the inline item's entire content is being removed, we need to find the previous inline item that
      // belongs to this damaged layout box.
      if (contentOffset == *damagedContent.Offset
          && damagedContent.ContentType != DamagedContent::Type::Removal)
      {
        return candidatePosition;
      }

      // The damage offset is before the first display box we managed to find for this layout box.
      // Let's adjust the candidate position by moving it over to the damaged offset.
      for (auto index = candidatePosition.Index; index--;)
      {
        auto &previousInlineItem = inlineItemList[index];
        if (&candidateInlineItem.LayoutBox() != &previousInlineItem.LayoutBox())
        {
          // We should stay on the same layout box.
          krys_debug_assert(false);
          return {};
        }
        if (startPosition(previousInlineItem) <= *damagedContent.Offset)
        {
          return InlineItemPosition {index, {}};
        }
      }
      return {};
    }

    KRYS_NODISCARD bool IsValidInlineItemPositionForLine(const InlineItemPosition &inlineItemPosition,
                                                         size_t lineIndex) noexcept
    {
      // It's clearly not correct when starting position is 0 while the damaged line is not the first one.
      if (!inlineItemPosition && lineIndex)
      {
        return false;
      }
      // FIXME: Consider checking for the case when we are on the first line with a non-zero
      // inlineItemPosition.
      return true;
    }

    KRYS_NODISCARD Maybe<InlineItemPosition>
      LeadingInlineItemPositionOnLastLine(const InlineItemList &inlineItemList,
                                          const inline_display::Boxes &displayBoxes) noexcept
    {
      krys_debug_assert(!displayBoxes.empty());
      auto lastLineIndex = displayBoxes.back().LineIndex();
      auto *leadingContentDisplayBoxOnInvalidatedLine =
        LeadingContentDisplayForLineIndex(lastLineIndex, displayBoxes);
      if (!leadingContentDisplayBoxOnInvalidatedLine)
      {
        return {};
      }

      auto inlineItemPositionForLeadingDisplayBox =
        InlineItemPositionForDisplayBox(*leadingContentDisplayBoxOnInvalidatedLine, inlineItemList);
      if (!inlineItemPositionForLeadingDisplayBox)
      {
        return {};
      }

      if (!IsValidInlineItemPositionForLine(*inlineItemPositionForLeadingDisplayBox, lastLineIndex))
      {
        krys_debug_assert(false);
        return {};
      }

      return *inlineItemPositionForLeadingDisplayBox;
    }

    KRYS_NODISCARD Maybe<InvalidatedLine>
      InvalidatedLineByDamagedBox(const DamagedContent &damagedContent, const InlineItemList &inlineItemList,
                                  const inline_display::Boxes &displayBoxes) noexcept
    {
      krys_debug_assert(!displayBoxes.empty());
      // 1. Find the root inline box based on the damaged layout box (this is our damaged line)
      // 2. Find the first content display box on this damaged line
      // 3. Find the associated InlineItem with partial text offset if applicable
      auto damagedLine = DamagedLineIndex(damagedContent, displayBoxes);
      if (!damagedLine)
      {
        return {};
      }

      // We can't be sure whether a newly inserted content introduces a new breaking opportunity, so let's
      // just initiate layout from the previous line.
      auto lineToInvalidate = damagedContent.ContentType == DamagedContent::Type::Removal ? *damagedLine
                              : !*damagedLine                                             ? 0
                                                                                          : *damagedLine - 1;
      auto *leadingContentDisplayBoxOnInvalidatedLine =
        LeadingContentDisplayForLineIndex(lineToInvalidate, displayBoxes);
      if (!leadingContentDisplayBoxOnInvalidatedLine)
      {
        // This is a completely empty line (e.g. <div><span> </span></div> where the whitespace is collapsed).
        return {};
      }
      if (auto inlineItemPositionForLeadingDisplayBox =
            InlineItemPositionForDisplayBox(*leadingContentDisplayBoxOnInvalidatedLine, inlineItemList))
      {
        if (!IsValidInlineItemPositionForLine(*inlineItemPositionForLeadingDisplayBox, lineToInvalidate))
        {
          krys_debug_assert(false);
          return {};
        }
        // In some rare cases, the leading display box's inline item position is after the actual damage (e.g.
        // when collapsed leading content is removed).
        if (auto adjustedPosition = InlineItemPositionForDamagedContentPosition(
              damagedContent, *inlineItemPositionForLeadingDisplayBox, inlineItemList))
        {
          return InvalidatedLine {lineToInvalidate, *adjustedPosition, *damagedLine};
        }
      }
      return {};
    }

    KRYS_NODISCARD InlineDamage::TrailingDisplayBoxList
      TrailingDisplayBoxesAfterDamagedLine(size_t damagedLineIndex,
                                           const inline_display::Content &displayContent) noexcept
    {
      auto trailingDisplayBoxes = InlineDamage::TrailingDisplayBoxList {};
      auto &lines = displayContent.LineList;
      auto &boxes = displayContent.BoxList;
      for (size_t lineIndex = damagedLineIndex + 1; lineIndex < lines.size(); ++lineIndex)
      {
        auto lastDisplayBoxIndexForLine = lines[lineIndex].FirstBoxIndex() + lines[lineIndex].BoxCount() - 1;
        if (lastDisplayBoxIndexForLine >= boxes.size())
        {
          krys_debug_assert(false);
          return {};
        }
        trailingDisplayBoxes.push_back(boxes[lastDisplayBoxIndexForLine]);
      }
      return trailingDisplayBoxes;
    }

    KRYS_NODISCARD bool IsSupportedContent(const Box &layoutBox) noexcept
    {
      return Is<InlineTextBox>(layoutBox) || layoutBox.IsLineBreakBox() || layoutBox.IsReplacedBox()
             || layoutBox.IsInlineBox();
    }
  }

  InlineInvalidation::InlineInvalidation(InlineDamage &inlineDamage, const InlineItemList &inlineItemList,
                                         const inline_display::Content &displayContent) noexcept
      : _inlineDamage(inlineDamage), _inlineItemList(inlineItemList), _displayContent(displayContent)
  {
  }

  bool InlineInvalidation::RootStyleWillChange(const ElementBox &formattingContextRoot,
                                               const RenderStyle &newStyle) noexcept
  {
    krys_debug_assert(formattingContextRoot.EstablishesInlineFormattingContext());

    if (_inlineDamage.IsInlineItemListDirty())
    {
      return true;
    }

    auto inlineItemListNeedsUpdate = [&]
    {
      auto &oldStyle = formattingContextRoot.Style();

      if (TextBreakingPositionContext {oldStyle} != TextBreakingPositionContext {newStyle})
      {
        return true;
      }

      if (!oldStyle.FontCascadeEqual(newStyle))
      {
        return true;
      }

      auto *newFirstLineStyle = newStyle.GetCachedPseudoStyle({PseudoElementType::FirstLine});
      auto *oldFirstLineStyle = oldStyle.GetCachedPseudoStyle({PseudoElementType::FirstLine});
      if (newFirstLineStyle && oldFirstLineStyle && !oldFirstLineStyle->FontCascadeEqual(*newFirstLineStyle))
      {
        return true;
      }

      if ((newFirstLineStyle && !newFirstLineStyle->FontCascadeEqual(oldStyle))
          || (oldFirstLineStyle && !oldFirstLineStyle->FontCascadeEqual(newStyle)))
      {
        return true;
      }

      if (oldStyle.GetWritingMode().BidiDirection() != newStyle.GetWritingMode().BidiDirection()
          || oldStyle.GetUnicodeBidi() != newStyle.GetUnicodeBidi()
          || oldStyle.GetTabSize() != newStyle.GetTabSize())
      {
        return true;
      }

      return false;
    };

    if (inlineItemListNeedsUpdate())
    {
      _inlineDamage.SetInlineItemListDirty();
    }

    return true;
  }

  bool InlineInvalidation::StyleWillChange(const Box &layoutBox, const RenderStyle &newStyle,
                                           style::Difference diff) noexcept
  {
    if (diff == style::DifferenceResult::Layout)
    {
      _inlineDamage.ResetLayoutPosition();
      _inlineDamage.SetDamageReason(InlineDamage::Reason::StyleChange);
    }

    if (_inlineDamage.IsInlineItemListDirty())
    {
      return true;
    }

    if (layoutBox.IsInlineTextBox())
    {
      // Either the root or parent inline box takes care of this style change.
      return true;
    }

    auto inlineItemListNeedsUpdate = [&]
    {
      auto &oldStyle = layoutBox.Style();

      auto hasInlineItemTypeChanged = oldStyle.HasOutOfFlowPosition() != newStyle.HasOutOfFlowPosition()
                                      || oldStyle.IsFloating() != newStyle.IsFloating()
                                      || oldStyle.Display() != newStyle.Display();
      if (hasInlineItemTypeChanged)
      {
        return true;
      }

      if (!layoutBox.IsInlineBox())
      {
        return false;
      }

      auto contentMayNeedNewBreakingPositionsAndMeasuring =
        TextBreakingPositionContext {oldStyle} != TextBreakingPositionContext {newStyle}
        || !oldStyle.FontCascadeEqual(newStyle);
      if (contentMayNeedNewBreakingPositionsAndMeasuring)
      {
        return true;
      }

      auto bidiContextChanged =
        oldStyle.GetUnicodeBidi() != newStyle.GetUnicodeBidi()
        || oldStyle.GetWritingMode().BidiDirection() != newStyle.GetWritingMode().BidiDirection();
      if (bidiContextChanged)
      {
        return true;
      }

      return false;
    };

    if (inlineItemListNeedsUpdate())
    {
      _inlineDamage.SetInlineItemListDirty();
    }

    return true;
  }

  bool InlineInvalidation::InlineLevelBoxContentWillChange(const Box &) noexcept
  {
    // FIXME: Add support for partial layout when inline box content change may trigger size change.
    _inlineDamage.ResetLayoutPosition();
    return true;
  }

  bool InlineInvalidation::UpdateInlineDamage(const InvalidatedLine &invalidatedLine,
                                              InlineDamage::Reason reason,
                                              ShouldApplyRangeLayout shouldApplyRangeLayout,
                                              LayoutUnit pageTopAdjustment) noexcept
  {
    auto isValidDamage = [&]
    {
      // Check for consistency.
      if (!invalidatedLine.LeadingInlineItemPosition)
      {
        // We have to start at the first line if damage points to the leading inline item.
        return !invalidatedLine.Index;
      }
      return true;
    };
    if (!isValidDamage())
    {
      krys_debug_assert(false);
      _inlineDamage.ResetLayoutPosition();
      return false;
    }

    auto partialContentTop =
      LayoutUnit {
        invalidatedLine.Index ? DisplayLines()[invalidatedLine.Index - 1].LineBoxLogicalRect().MaxY() : 0.f}
      + pageTopAdjustment;

    auto layoutStartPosition = InlineDamage::LayoutPosition {
      invalidatedLine.Index, invalidatedLine.LeadingInlineItemPosition, partialContentTop};

    _inlineDamage.SetDamageReason(reason);
    _inlineDamage.SetLayoutStartPosition(layoutStartPosition);

    if (shouldApplyRangeLayout == ShouldApplyRangeLayout::Yes)
    {
      _inlineDamage.SetTrailingDisplayBoxes(
        TrailingDisplayBoxesAfterDamagedLine(invalidatedLine.Index, _displayContent));
    }
    return true;
  }

  bool InlineInvalidation::SetFullLayoutIfNeeded(const Box &layoutBox) noexcept
  {
    if (!IsSupportedContent(layoutBox))
    {
      krys_debug_assert(false);
      _inlineDamage.ResetLayoutPosition();
      return true;
    }

    if (DisplayBoxes().empty())
    {
      krys_debug_assert(false);
      _inlineDamage.ResetLayoutPosition();
      return true;
    }

    if (_inlineItemList.empty())
    {
      // We must be under memory pressure.
      _inlineDamage.ResetLayoutPosition();
      return true;
    }

    if (_inlineDamage.Reasons().Contains(InlineDamage::Reason::StyleChange))
    {
      _inlineDamage.ResetLayoutPosition();
      return true;
    }

    return false;
  }

  bool InlineInvalidation::TextInserted(const InlineTextBox &newOrDamagedInlineTextBox,
                                        Maybe<size_t> offset) noexcept
  {
    _inlineDamage.SetInlineItemListDirty();

    if (SetFullLayoutIfNeeded(newOrDamagedInlineTextBox))
    {
      return false;
    }

    auto &displayBoxes = DisplayBoxes();
    auto invalidatedLine = Maybe<InvalidatedLine> {};
    auto damageReason = offset                                          ? InlineDamage::Reason::ContentChange
                        : newOrDamagedInlineTextBox.NextInFlowSibling() ? InlineDamage::Reason::Insert
                                                                        : InlineDamage::Reason::Append;
    switch (damageReason)
    {
      case InlineDamage::Reason::ContentChange:
        // Existing text box got modified. Dirty all the way up to the damaged position's line.
        invalidatedLine =
          InvalidatedLineByDamagedBox({newOrDamagedInlineTextBox, *offset}, _inlineItemList, displayBoxes);
        break;
      case InlineDamage::Reason::Append:
        // New text box got appended. Let's dirty the last line.
        if (auto leadingInlineItemPosition =
              LeadingInlineItemPositionOnLastLine(_inlineItemList, displayBoxes))
        {
          auto lastLineIndex = displayBoxes.back().LineIndex();
          invalidatedLine = InvalidatedLine {lastLineIndex, *leadingInlineItemPosition, lastLineIndex};
        }
        break;
      case InlineDamage::Reason::Insert:
      {
        invalidatedLine = InvalidatedLine {};
        // New text box got inserted. Let's damage existing content starting from the previous sibling.
        if (auto *previousSibling = newOrDamagedInlineTextBox.PreviousInFlowSibling())
        {
          invalidatedLine = InvalidatedLineByDamagedBox({*previousSibling}, _inlineItemList, displayBoxes);
        }
        break;
      }
      default: krys_debug_assert(false); break;
    }

    if (invalidatedLine)
    {
      return UpdateInlineDamage(*invalidatedLine, damageReason,
                                offset ? ShouldApplyRangeLayout::No : ShouldApplyRangeLayout::Yes);
    }

    _inlineDamage.ResetLayoutPosition();
    return false;
  }

  bool InlineInvalidation::TextWillBeRemoved(const InlineTextBox &damagedInlineTextBox,
                                             Maybe<size_t> offset) noexcept
  {
    _inlineDamage.SetInlineItemListDirty();

    if (SetFullLayoutIfNeeded(damagedInlineTextBox))
    {
      return false;
    }

    if (auto invalidatedLine = InvalidatedLineByDamagedBox(
          {damagedInlineTextBox, offset.value_or(0), DamagedContent::Type::Removal}, _inlineItemList,
          DisplayBoxes()))
    {
      return UpdateInlineDamage(*invalidatedLine, InlineDamage::Reason::Remove, ShouldApplyRangeLayout::No);
    }

    _inlineDamage.ResetLayoutPosition();
    return false;
  }

  bool InlineInvalidation::InlineLevelBoxInserted(const Box &layoutBox) noexcept
  {
    _inlineDamage.SetInlineItemListDirty();

    if (SetFullLayoutIfNeeded(layoutBox))
    {
      return false;
    }

    auto &displayBoxes = DisplayBoxes();
    auto invalidatedLine = Maybe<InvalidatedLine> {};
    if (!layoutBox.NextInFlowSibling())
    {
      // New box got appended. Let's dirty the last line.
      if (_inlineDamage.Reasons() == EnumSet<InlineDamage::Reason> {InlineDamage::Reason::Append})
      {
        // Series of append operations always produces the same damage position.
        return _inlineDamage.LayoutStartPosition().has_value();
      }
      krys_debug_assert(_inlineDamage.Reasons().IsEmpty());
      if (auto leadingInlineItemPosition = LeadingInlineItemPositionOnLastLine(_inlineItemList, displayBoxes))
      {
        auto lastLineIndex = displayBoxes.back().LineIndex();
        invalidatedLine = InvalidatedLine {lastLineIndex, *leadingInlineItemPosition, lastLineIndex};
      }
    }
    else
    {
      invalidatedLine = InvalidatedLine {};
      // New box got inserted. Let's damage existing content starting from the previous sibling.
      if (auto *previousSibling = layoutBox.PreviousInFlowSibling())
      {
        invalidatedLine = InvalidatedLineByDamagedBox({*previousSibling}, _inlineItemList, displayBoxes);
      }
    }
    if (invalidatedLine)
    {
      return UpdateInlineDamage(*invalidatedLine,
                                !layoutBox.NextInFlowSibling() ? InlineDamage::Reason::Append
                                                               : InlineDamage::Reason::Insert,
                                ShouldApplyRangeLayout::Yes);
    }

    _inlineDamage.ResetLayoutPosition();
    return false;
  }

  bool InlineInvalidation::InlineLevelBoxWillBeRemoved(const Box &layoutBox) noexcept
  {
    _inlineDamage.SetInlineItemListDirty();

    if (SetFullLayoutIfNeeded(layoutBox))
    {
      return false;
    }

    if (auto invalidatedLine = InvalidatedLineByDamagedBox({layoutBox, {}, DamagedContent::Type::Removal},
                                                           _inlineItemList, DisplayBoxes()))
    {
      return UpdateInlineDamage(*invalidatedLine, InlineDamage::Reason::Remove, ShouldApplyRangeLayout::Yes);
    }

    _inlineDamage.ResetLayoutPosition();
    return false;
  }

  bool InlineInvalidation::RestartForPagination(size_t lineIndex, LayoutUnit pageTopAdjustment) noexcept
  {
    auto *leadingContentDisplayBoxOnDamagedLine =
      LeadingContentDisplayForLineIndex(lineIndex, DisplayBoxes());
    if (!leadingContentDisplayBoxOnDamagedLine)
    {
      return false;
    }
    auto inlineItemPositionForLeadingDisplayBox =
      InlineItemPositionForDisplayBox(*leadingContentDisplayBoxOnDamagedLine, _inlineItemList);
    if (!inlineItemPositionForLeadingDisplayBox || !*inlineItemPositionForLeadingDisplayBox)
    {
      return false;
    }

    return UpdateInlineDamage({lineIndex, *inlineItemPositionForLeadingDisplayBox},
                              InlineDamage::Reason::Pagination, ShouldApplyRangeLayout::Yes,
                              pageTopAdjustment);
  }

  void InlineInvalidation::ResetInlineDamage(InlineDamage &inlineDamage) noexcept
  {
    inlineDamage.SetInlineItemListDirty();
    inlineDamage.ResetLayoutPosition();
  }
}
