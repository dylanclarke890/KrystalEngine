#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingUtils.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineBox.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  class InlineFormattingContext;
  class InlineLayoutState;

  /// @brief Computes the height of a line box and vertically aligns its inline level boxes.
  class LineBoxVerticalAligner
  {
    struct LineBoxAlignmentContent
    {
      struct TopAndBottomAlignedMaximumHeight
      {
        Maybe<InlineLayoutUnit> Top {};
        Maybe<InlineLayoutUnit> Bottom {};
      };

      InlineLayoutUnit NonLineBoxRelativeAlignedMaximumHeight {0};
      TopAndBottomAlignedMaximumHeight TopAndBottomAlignedMaximumHeightValue {};
      bool HasTextEmphasis {false};

      KRYS_NODISCARD InlineLayoutUnit Height() const noexcept
      {
        return std::max(NonLineBoxRelativeAlignedMaximumHeight,
                        std::max(TopAndBottomAlignedMaximumHeightValue.Top.value_or(0.f),
                                 TopAndBottomAlignedMaximumHeightValue.Bottom.value_or(0.f)));
      }
    };

    enum class IsInlineLevelBoxAlignment : bool
    {
      No,
      Yes
    };

  private:
    const InlineFormattingContext &_inlineFormattingContext;

  public:
    explicit LineBoxVerticalAligner(const InlineFormattingContext &inlineFormattingContext) noexcept;

    KRYS_NODISCARD InlineLayoutUnit ComputeLogicalHeightAndAlign(LineBox &lineBox) const noexcept;

  private:
    KRYS_NODISCARD InlineLayoutUnit SimplifiedVerticalAlignment(LineBox &lineBox) const noexcept;

    KRYS_NODISCARD LineBoxAlignmentContent ComputeLineBoxLogicalHeight(LineBox &lineBox) const noexcept;
    void ComputeRootInlineBoxVerticalPosition(
      LineBox &lineBox, const LineBoxAlignmentContent &lineBoxAlignmentContent) const noexcept;
    void AlignInlineLevelBoxes(LineBox &lineBox, InlineLayoutUnit lineBoxLogicalHeight) const noexcept;
    KRYS_NODISCARD InlineLayoutUnit
      AdjustForAnnotationIfNeeded(LineBox &lineBox, InlineLayoutUnit lineBoxHeight) const noexcept;
    KRYS_NODISCARD Maybe<InlineLevelBox::AscentAndDescent>
      LayoutBoundsForInlineBoxSubtree(const LineBox::InlineLevelBoxList &nonRootInlineLevelBoxes,
                                      size_t inlineBoxIndex) const noexcept;
    KRYS_NODISCARD InlineLayoutUnit LogicalTopOffsetFromParentBaseline(
      const InlineLevelBox &inlineLevelBox, const InlineLevelBox &parentInlineBox,
      IsInlineLevelBoxAlignment isInlineLevelBoxAlignment = IsInlineLevelBoxAlignment::No) const noexcept;

    KRYS_NODISCARD const InlineFormattingUtils &FormattingUtils() const noexcept;
    KRYS_NODISCARD const InlineFormattingContext &GetFormattingContext() const noexcept
    {
      return _inlineFormattingContext;
    }
    KRYS_NODISCARD const ElementBox &RootBox() const noexcept;
    KRYS_NODISCARD const InlineLayoutState &GetLayoutState() const noexcept;
  };
}
