#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/Inline/Display/InlineDisplayContent.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingConstraints.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineTypes.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineRect.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/LineLayoutResult.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::layout
{
  class Box;
  class InlineFormattingContext;
  class InlineLayoutState;
  class LineBox;

  /// @brief Builds the display line for a line box, and applies line ending truncation (ellipsis) to display
  /// lines.
  class InlineDisplayLineBuilder
  {
    struct EnclosingLineGeometry
    {
      inline_display::Line::EnclosingTopAndBottom EnclosingTopAndBottom;
      InlineRect ContentOverflowRect;
    };

  private:
    InlineFormattingContext &_inlineFormattingContext;
    const ConstraintsForInlineContent &_constraints;

  public:
    InlineDisplayLineBuilder(InlineFormattingContext &inlineFormattingContext,
                             const ConstraintsForInlineContent &constraints) noexcept;

    KRYS_NODISCARD inline_display::Line Build(const LineLayoutResult &lineLayoutResult,
                                              const LineBox &lineBox,
                                              bool lineIsFullyTruncatedInBlockDirection) const noexcept;

    static void ApplyEllipsisIfNeeded(LineEndingTruncationPolicy truncationPolicy,
                                      inline_display::Line &displayLine, inline_display::Boxes &displayBoxes,
                                      bool isLegacyLineClamp) noexcept;
    static void
      AddLegacyLineClampTrailingLinkBoxIfApplicable(const InlineFormattingContext &inlineFormattingContext,
                                                    const InlineLayoutState &inlineLayoutState,
                                                    inline_display::Content &displayContent) noexcept;
    KRYS_NODISCARD static bool
      HasTrailingLineWithBlockContent(const inline_display::Lines &displayLines) noexcept;

  private:
    KRYS_NODISCARD EnclosingLineGeometry
      CollectEnclosingLineGeometry(const LineLayoutResult &lineLayoutResult, const LineBox &lineBox,
                                   const InlineRect &lineBoxRect) const noexcept;

    KRYS_NODISCARD const ConstraintsForInlineContent &GetConstraints() const noexcept
    {
      return _constraints;
    }
    KRYS_NODISCARD const InlineFormattingContext &GetFormattingContext() const noexcept
    {
      return _inlineFormattingContext;
    }
    KRYS_NODISCARD InlineFormattingContext &GetFormattingContext() noexcept
    {
      return _inlineFormattingContext;
    }
    KRYS_NODISCARD const Box &Root() const noexcept;
  };
}
