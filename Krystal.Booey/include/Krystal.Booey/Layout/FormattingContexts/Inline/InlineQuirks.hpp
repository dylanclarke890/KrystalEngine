#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLine.hpp"
#include "Krystal.Booey/Layout/LayoutUnits.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  class InlineFormattingContext;
  class InlineLevelBox;
  class LineBox;

  /// @brief The quirks mode behaviours of inline layout.
  class InlineQuirks
  {
  private:
    const InlineFormattingContext &_inlineFormattingContext;

  public:
    explicit InlineQuirks(const InlineFormattingContext &inlineFormattingContext) noexcept;

    KRYS_NODISCARD bool TrailingNonBreakingSpaceNeedsAdjustment(bool isInIntrinsicWidthMode,
                                                                bool lineHasOverflow) const noexcept;
    KRYS_NODISCARD InlineLayoutUnit InitialLineHeight() const noexcept;
    KRYS_NODISCARD bool InlineBoxAffectsLineBox(const InlineLevelBox &inlineLevelBox) const noexcept;
    KRYS_NODISCARD static bool LineBreakBoxAffectsParentInlineBox(const LineBox &lineBox) noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit>
      InitialLetterAlignmentOffset(const Box &floatBox, const RenderStyle &lineBoxStyle) const noexcept;
    KRYS_NODISCARD bool ShouldCollapseLineBoxHeight(const Line::RunList &lineContent,
                                                    size_t numberOfOutsideListMarkers) const noexcept;

  private:
    KRYS_NODISCARD const InlineFormattingContext &GetFormattingContext() const noexcept
    {
      return _inlineFormattingContext;
    }
  };
}
