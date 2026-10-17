#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/Inline/Display/InlineDisplayContent.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Invalidation/InlineDamage.hpp"
#include "Krystal.Booey/Style/Difference.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::layout
{
  class Box;
  class ElementBox;
  class InlineTextBox;
  struct InvalidatedLine;

  /// @brief Records changes to an inline formatting context's content and style in its damage: whether the
  /// inline item list needs rebuilding, and where a partial layout can start (or that it needs a full one).
  class InlineInvalidation
  {
  private:
    enum class ShouldApplyRangeLayout : bool
    {
      No,
      Yes
    };

    InlineDamage &_inlineDamage;

    const InlineItemList &_inlineItemList;
    const inline_display::Content &_displayContent;

  public:
    InlineInvalidation(InlineDamage &inlineDamage, const InlineItemList &inlineItemList,
                       const inline_display::Content &displayContent) noexcept;

    bool RootStyleWillChange(const ElementBox &formattingContextRoot, const RenderStyle &newStyle) noexcept;
    bool StyleWillChange(const Box &layoutBox, const RenderStyle &newStyle, style::Difference diff) noexcept;

    bool TextInserted(const InlineTextBox &newOrDamagedInlineTextBox, Maybe<size_t> offset = {}) noexcept;
    bool TextWillBeRemoved(const InlineTextBox &damagedInlineTextBox, Maybe<size_t> offset = {}) noexcept;

    bool InlineLevelBoxInserted(const Box &layoutBox) noexcept;
    bool InlineLevelBoxWillBeRemoved(const Box &layoutBox) noexcept;
    bool InlineLevelBoxContentWillChange(const Box &layoutBox) noexcept;

    bool RestartForPagination(size_t lineIndex, LayoutUnit pageTopAdjustment) noexcept;

    KRYS_NODISCARD static bool MayOnlyNeedPartialLayout(const InlineDamage *inlineDamage) noexcept
    {
      return inlineDamage && inlineDamage->LayoutStartPosition();
    }
    static void ResetInlineDamage(InlineDamage &inlineDamage) noexcept;

  private:
    bool UpdateInlineDamage(const InvalidatedLine &invalidatedLine, InlineDamage::Reason reason,
                            ShouldApplyRangeLayout shouldApplyRangeLayout = ShouldApplyRangeLayout::No,
                            LayoutUnit restartPaginationAdjustment = 0_lu) noexcept;
    bool SetFullLayoutIfNeeded(const Box &layoutBox) noexcept;
    KRYS_NODISCARD const inline_display::Boxes &DisplayBoxes() const noexcept
    {
      return _displayContent.BoxList;
    }
    KRYS_NODISCARD const inline_display::Lines &DisplayLines() const noexcept
    {
      return _displayContent.LineList;
    }
  };
}
