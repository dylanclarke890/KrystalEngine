#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/Inline/Display/InlineDisplayContent.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLine.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineTypes.hpp"
#include "Krystal.Booey/Layout/LayoutUnits.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"

namespace krys::boo::layout
{
  class Box;
  class InlineFormattingContext;

  /// @brief Distributes the free space of a line between its runs: `text-align: justify` and `ruby-align`,
  /// and moves display boxes by the resulting alignment offsets.
  class InlineContentAligner
  {
  public:
    KRYS_NODISCARD static InlineLayoutUnit
      ApplyTextAlignJustify(Line::RunList &runs, InlineLayoutUnit spaceToDistribute,
                            size_t hangingTrailingWhitespaceLength) noexcept;

    KRYS_NODISCARD static InlineLayoutUnit ApplyRubyAlign(RubyAlign rubyAlign, Line::RunList &runs,
                                                          IndexRange range,
                                                          InlineLayoutUnit spaceToDistribute) noexcept;

    enum class AdjustContentOnlyInsideRubyBase : bool
    {
      No,
      Yes
    };
    static void
      ApplyRubyBaseAlignmentOffset(inline_display::Boxes &displayBoxes,
                                   const HashMap<const Box *, InlineLayoutUnit> &alignmentOffsetList,
                                   AdjustContentOnlyInsideRubyBase adjustContentOnlyInsideRubyBase,
                                   InlineFormattingContext &inlineFormattingContext) noexcept;
    static void ApplyRubyAnnotationAlignmentOffset(inline_display::Boxes &displayBoxes,
                                                   InlineLayoutUnit alignmentOffset,
                                                   InlineFormattingContext &inlineFormattingContext) noexcept;

  private:
    KRYS_NODISCARD static InlineLayoutUnit ApplyExpansionOnRange(Line::RunList &runs, IndexRange range,
                                                                 const ExpansionInfo &expansion,
                                                                 InlineLayoutUnit spaceToDistribute) noexcept;
  };
}
