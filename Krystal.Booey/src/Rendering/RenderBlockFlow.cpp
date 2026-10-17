#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"

namespace krys::boo
{
  RenderBlockFlow::RenderBlockFlow(Type type, dom::Element &element, RenderStyle &&style,
                                   EnumSet<BlockFlowFlag> flags) noexcept
      : RenderBlock(type, element, krys::move(style), {}, flags)
  {
    krys_debug_assert(IsRenderBlockFlow());
    SetChildrenInline(true);
  }

  RenderBlockFlow::RenderBlockFlow(Type type, dom::Document &document, RenderStyle &&style,
                                   EnumSet<BlockFlowFlag> flags) noexcept
      : RenderBlock(type, document, krys::move(style), {}, flags)
  {
    krys_debug_assert(IsRenderBlockFlow());
    SetChildrenInline(true);
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderBlockFlow::~RenderBlockFlow() noexcept = default;

  void RenderBlockFlow::ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                                      LayoutUnit &maxLogicalWidth) const noexcept
  {
    if (ShouldApplySizeOrInlineSizeContainment())
    {
      if (auto width = ExplicitIntrinsicInnerLogicalWidth())
      {
        minLogicalWidth = *width;
        maxLogicalWidth = *width;
      }
    }
    else if (ChildrenInline())
    {
      // FIXME: The preferred widths of inline content (computeInlinePreferredLogicalWidths), once there's
      // inline layout.
    }
    else
    {
      ComputeBlockPreferredLogicalWidths(minLogicalWidth, maxLogicalWidth);
    }

    maxLogicalWidth = std::max(minLogicalWidth, maxLogicalWidth);

    // FIXME: Multi-column widths (adjustIntrinsicLogicalWidthsForColumns), horizontal marquees, and a table
    // cell's fixed width, once there are columns, marquees and table layout.

    auto scrollbarWidth = LayoutUnit(IntrinsicScrollbarLogicalWidthIncludingGutter());
    maxLogicalWidth += scrollbarWidth;
    minLogicalWidth += scrollbarWidth;
  }
}
