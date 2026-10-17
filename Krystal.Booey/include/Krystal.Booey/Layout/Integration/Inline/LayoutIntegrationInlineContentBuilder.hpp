#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingContext.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class RenderBlockFlow;
}

namespace krys::boo::layout_integration
{
  class InlineContent;

  /// @brief Moves a layout result's display content into a block container's inline content and computes the
  /// lines' overflow and the boxes' first and last flags.
  class InlineContentBuilder
  {
  private:
    const RenderBlockFlow &_blockFlow;

  public:
    explicit InlineContentBuilder(const RenderBlockFlow &blockFlow) noexcept;

    /// @return The rect the content covered before and after, to repaint.
    FloatRect Build(layout::InlineLayoutResult &&layoutResult, InlineContent &inlineContent) const noexcept;
    void UpdateLineOverflow(InlineContent &inlineContent) const noexcept;

  private:
    void AdjustDisplayLines(InlineContent &inlineContent, size_t startIndex) const noexcept;
    void
      ComputeIsFirstIsLastBoxAndBidiReorderingForInlineContent(inline_display::Boxes &boxes) const noexcept;
  };
}
