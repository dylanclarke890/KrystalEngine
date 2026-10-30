#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineBox.hpp"
#include "Krystal.Booey/Graphics/FontMetrics.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingUtils.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::layout
{
  LineBox::LineBox(const Box &rootLayoutBox, InlineLayoutUnit contentLogicalLeft,
                   InlineLayoutUnit contentLogicalWidth, size_t lineIndex, bool isFirstFormattedLine,
                   size_t nonSpanningInlineLevelBoxCount) noexcept
      : _lineIndex(lineIndex), _isFirstFormattedLine(isFirstFormattedLine),
        _rootInlineBox(InlineLevelBox::CreateRootInlineBox(
          rootLayoutBox, isFirstFormattedLine ? rootLayoutBox.FirstLineStyle() : rootLayoutBox.Style(),
          contentLogicalLeft, contentLogicalWidth))
  {
    _nonRootInlineLevelBoxList.reserve(nonSpanningInlineLevelBoxCount);
    _nonRootInlineLevelBoxMap.reserve(nonSpanningInlineLevelBoxCount);
    _rootInlineBox.SetTextEmphasis(
      InlineFormattingUtils::TextEmphasisForInlineBox(rootLayoutBox, Downcast<ElementBox>(rootLayoutBox)));
  }

  void LineBox::AddInlineLevelBox(InlineLevelBox &&inlineLevelBox) noexcept
  {
    _boxTypes.Add(inlineLevelBox.GetType());
    _nonRootInlineLevelBoxMap.insert_or_assign(&inlineLevelBox.LayoutBox(),
                                               _nonRootInlineLevelBoxList.size());
    _nonRootInlineLevelBoxList.push_back(krys::move(inlineLevelBox));
  }

  InlineRect LineBox::LogicalRectForTextRun(const Line::Run &run) const noexcept
  {
    krys_debug_assert(run.IsText() || run.IsSoftLineBreak());
    auto *ancestorInlineBox = &ParentInlineBox(run);
    krys_debug_assert(ancestorInlineBox->IsInlineBox());
    auto runlogicalTop =
      ancestorInlineBox->LogicalTop() - ancestorInlineBox->InlineBoxContentOffsetForTextBoxTrim();
    auto &metricsOfPrimaryFont = ancestorInlineBox->PrimarymetricsOfPrimaryFont();
    auto logicalHeight =
      InlineFormattingUtils::SnapToInt(metricsOfPrimaryFont.Ascent(), *ancestorInlineBox)
      + InlineFormattingUtils::SnapToInt(metricsOfPrimaryFont.Descent(), *ancestorInlineBox);

    while (ancestorInlineBox != &_rootInlineBox && !ancestorInlineBox->HasLineBoxRelativeAlignment())
    {
      ancestorInlineBox = &ParentInlineBox(*ancestorInlineBox);
      krys_debug_assert(ancestorInlineBox->IsInlineBox());
      runlogicalTop += ancestorInlineBox->LogicalTop();
    }
    return {runlogicalTop, _rootInlineBox.LogicalLeft() + run.LogicalLeft(), run.LogicalWidth(),
            logicalHeight};
  }

  InlineRect LineBox::LogicalRectForLineBreakBox(const Box &layoutBox) const noexcept
  {
    krys_debug_assert(layoutBox.IsLineBreakBox());
    return LogicalRectForInlineLevelBox(layoutBox);
  }

  InlineLayoutUnit LineBox::InlineLevelBoxAbsoluteTop(const InlineLevelBox &inlineLevelBox) const noexcept
  {
    // Inline level boxes are relative to their parent unless the vertical alignment makes them relative to
    // the line box (e.g. top, bottom).
    auto top = inlineLevelBox.LogicalTop();
    if (inlineLevelBox.IsRootInlineBox() || inlineLevelBox.HasLineBoxRelativeAlignment())
    {
      return top;
    }

    // Fast path for inline level boxes on the root inline box (e.g <div><img></div>).
    if (&inlineLevelBox.LayoutBox().Parent() == &_rootInlineBox.LayoutBox())
    {
      return top + _rootInlineBox.LogicalTop();
    }

    // Nested inline content e.g <div><span><img></span></div>
    auto *ancestorInlineBox = &inlineLevelBox;
    while (ancestorInlineBox != &_rootInlineBox && !ancestorInlineBox->HasLineBoxRelativeAlignment())
    {
      ancestorInlineBox = &ParentInlineBox(*ancestorInlineBox);
      krys_debug_assert(ancestorInlineBox->IsInlineBox());
      top += ancestorInlineBox->LogicalTop();
    }
    return top;
  }

  InlineRect LineBox::LogicalRectForInlineLevelBox(const Box &layoutBox) const noexcept
  {
    // FIXME: Blocks in inline. Refactor and remove the block test.
    krys_debug_assert(layoutBox.IsInlineLevelBox() || layoutBox.IsLineBreakBox()
                      || (layoutBox.IsBlockLevelBox() && layoutBox.IsInFlow()));
    auto *inlineBox = InlineLevelBoxFor(layoutBox);
    if (!inlineBox)
    {
      krys_debug_assert(false);
      return {};
    }
    auto &inlineBoxLogicalRect = inlineBox->LogicalRect();
    return InlineRect {InlineLevelBoxAbsoluteTop(*inlineBox), inlineBoxLogicalRect.Left(),
                       inlineBoxLogicalRect.Width(), inlineBoxLogicalRect.Height()};
  }

  InlineRect LineBox::LogicalBorderBoxForAtomicInlineBox(const Box &layoutBox,
                                                         const BoxGeometry &boxGeometry) const noexcept
  {
    // FIXME: Blocks in inline. Refactor and remove the block test.
    krys_debug_assert(layoutBox.IsAtomicInlineBox() || (layoutBox.IsBlockLevelBox() && layoutBox.IsInFlow()));
    auto logicalRect = LogicalRectForInlineLevelBox(layoutBox);
    // Inline level boxes use their margin box for vertical alignment. Let's covert them to border boxes.
    logicalRect.MoveVertically(boxGeometry.MarginBefore().ToFloat());
    auto verticalMargin = (boxGeometry.MarginBefore() + boxGeometry.MarginAfter()).ToFloat();
    logicalRect.ExpandVertically(-verticalMargin);

    return logicalRect;
  }

  InlineRect LineBox::LogicalBorderBoxForInlineBox(const Box &layoutBox,
                                                   const BoxGeometry &boxGeometry) const noexcept
  {
    auto logicalRect = LogicalRectForInlineLevelBox(layoutBox);
    // This logical rect is as tall as the "text" content is. Let's adjust with vertical border and padding.
    logicalRect.ExpandVertically(boxGeometry.VerticalBorderAndPadding().ToFloat());
    logicalRect.MoveVertically(-boxGeometry.BorderAndPaddingBefore().ToFloat());
    return logicalRect;
  }

  InlineRect LineBox::LogicalContentBoxForInlineBox(const Box &layoutBox) const noexcept
  {
    return LogicalRectForInlineLevelBox(layoutBox);
  }
}
