#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineQuirks.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingContext.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineBox.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::layout
{
  InlineQuirks::InlineQuirks(const InlineFormattingContext &inlineFormattingContext) noexcept
      : _inlineFormattingContext(inlineFormattingContext)
  {
  }

  bool InlineQuirks::TrailingNonBreakingSpaceNeedsAdjustment(bool, bool) const noexcept
  {
    // Only `nbsp-mode: space` makes trailing no-break spaces trimmable, and that mode isn't supported.
    return false;
  }

  InlineLayoutUnit InlineQuirks::InitialLineHeight() const noexcept
  {
    krys_debug_assert(!GetFormattingContext().GetLayoutState().InStandardsMode());
    return 0.f;
  }

  bool InlineQuirks::LineBreakBoxAffectsParentInlineBox(const LineBox &lineBox) noexcept
  {
    // In quirks mode linebreak boxes (<br>) stop affecting the line box when (assume <br> is nested e.g.
    // <span style="font-size: 100px"><br></span>)
    // 1. the root inline box has content <div>content<br>/div>
    // 2. there's at least one atomic inline level box on the line e.g <div><img><br></div> or
    // <div><span><img></span><br></div>
    // 3. there's at least one inline box with content e.g. <div><span>content</span><br></div>
    if (lineBox.RootInlineBox().HasContent())
    {
      return false;
    }
    if (lineBox.HasAtomicInlineBox())
    {
      return false;
    }
    // At this point we either have only the <br> on the line or inline boxes with or without content.
    auto &inlineLevelBoxes = lineBox.NonRootInlineLevelBoxes();
    krys_debug_assert(!inlineLevelBoxes.empty());
    if (inlineLevelBoxes.size() == 1)
    {
      return true;
    }
    for (auto &inlineLevelBox : lineBox.NonRootInlineLevelBoxes())
    {
      // Filter out empty inline boxes e.g. <div><span></span><span></span><br></div>
      if (inlineLevelBox.IsInlineBox() && inlineLevelBox.HasContent())
      {
        return false;
      }
    }
    return true;
  }

  bool InlineQuirks::InlineBoxAffectsLineBox(const InlineLevelBox &inlineLevelBox) const noexcept
  {
    krys_debug_assert(!GetFormattingContext().GetLayoutState().InStandardsMode());
    krys_debug_assert(inlineLevelBox.IsInlineBox());
    // Inline boxes (e.g. root inline box or <span>) affects line boxes either through the strut or actual
    // content.
    if (inlineLevelBox.HasContent())
    {
      return true;
    }
    if (inlineLevelBox.IsRootInlineBox())
    {
      // This root inline box has no direct text content and we are in non-standards mode.
      // Now according to legacy line layout, we need to apply the following list-item specific quirk:
      // We do not create markers for list items when the list-style-type is none, while other browsers do.
      // The side effect of having no marker is that in quirks mode we have to specifically check for
      // list-item and make sure it is treated as if it had content and stretched the line. see
      // LegacyInlineFlowBox c'tor.
      return inlineLevelBox.LayoutBox().Style().IsOriginalDisplayListItemType();
    }
    // Non-root inline boxes (e.g. <span>).
    auto &boxGeometry = GetFormattingContext().GeometryForBox(inlineLevelBox.LayoutBox());
    if (boxGeometry.HorizontalBorderAndPadding())
    {
      // Horizontal border and padding make the inline box stretch the line (e.g. <span style="padding:
      // 10px;"></span>).
      return true;
    }
    return false;
  }

  Maybe<LayoutUnit> InlineQuirks::InitialLetterAlignmentOffset(const Box &floatBox,
                                                               const RenderStyle &lineBoxStyle) const noexcept
  {
    krys_debug_assert(floatBox.IsFloatingPositioned());
    if (!floatBox.Style().LineBoxContain().Contains(style::WebkitLineBoxContainValue::InitialLetter))
    {
      return {};
    }
    auto &primaryFontMetrics = lineBoxStyle.GetFontCascade().MetricsOfPrimaryFont();
    auto lineHeight = [&]() -> InlineLayoutUnit
    {
      if (lineBoxStyle.GetLineHeight().IsNormal())
      {
        return InlineFormattingUtils::Ascent(primaryFontMetrics, FontBaseline::Alphabetic, floatBox)
               + InlineFormattingUtils::Descent(primaryFontMetrics, FontBaseline::Alphabetic, floatBox);
      }
      return lineBoxStyle.ComputedLineHeight();
    };
    auto &floatBoxGeometry = GetFormattingContext().GeometryForBox(floatBox);
    auto fontHeight = InlineFormattingUtils::SnapToInt(primaryFontMetrics.Ascent(), floatBox)
                      + InlineFormattingUtils::SnapToInt(primaryFontMetrics.Descent(), floatBox);
    return LayoutUnit {
      InlineFormattingUtils::Ascent(primaryFontMetrics, FontBaseline::Alphabetic, floatBox)
      + (lineHeight() - fontHeight) / 2
      - InlineFormattingUtils::SnapToInt(primaryFontMetrics.CapHeight().value_or(0.f), floatBox)
      - floatBoxGeometry.MarginBorderAndPaddingBefore().ToFloat()};
  }

  bool InlineQuirks::ShouldCollapseLineBoxHeight(const Line::RunList &lineContent,
                                                 size_t numberOfOutsideListMarkers) const noexcept
  {
    // This quirk is needed as we put the marker in seemingly random places in the list item's subtree.
    // (instead of having a well defined place and layout behavior).
    if (!lineContent.size() || numberOfOutsideListMarkers != 1)
    {
      return false;
    }

    if (!lineContent[0].IsListMarkerOutside())
    {
      krys_debug_assert(lineContent[0].IsListMarkerInside());
      return false;
    }

    size_t emptyInlineBoxCount = 0;
    for (auto &run : lineContent)
    {
      if (run.IsListMarkerOutside())
      {
        continue;
      }
      if (Line::Run::IsContentfulOrHasDecoration(run, GetFormattingContext()))
      {
        return false;
      }
      if (run.IsInlineBoxStart())
      {
        ++emptyInlineBoxCount;
      }
    }

    if (lineContent[0].IsListMarkerOutside() && emptyInlineBoxCount
        && emptyInlineBoxCount == lineContent.size() - 1)
    {
      // This is to handle non-contentful lines introduced by block boxes. They are supposed to be collapsed
      // so that the block content can be placed next to the list marker. Regular inline content would never
      // produced a line with inline box only runs. Also inline content like <li><span><br> is not supposed to
      // produce a collapsed line box. The underlying issue is the assumption that we shouldn’t collapse when
      // rootBox is a list item (see below).
      return true;
    }

    auto &rootBox = GetFormattingContext().Root();
    if (rootBox.IsAnonymous() || rootBox.IsListItem())
    {
      return false;
    }

    return true;
  }
}
