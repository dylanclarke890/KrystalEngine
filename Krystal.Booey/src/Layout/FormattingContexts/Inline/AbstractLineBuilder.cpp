#include "Krystal.Booey/Layout/FormattingContexts/Inline/AbstractLineBuilder.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingContext.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::layout
{
  AbstractLineBuilder::AbstractLineBuilder(InlineFormattingContext &inlineFormattingContext,
                                           const ElementBox &rootBox,
                                           HorizontalConstraints rootHorizontalConstraints,
                                           const InlineItemList &inlineItemList) noexcept
      : _line(inlineFormattingContext), _inlineItemList(inlineItemList.data(), inlineItemList.size()),
        _inlineFormattingContext(inlineFormattingContext), _rootBox(rootBox),
        _rootHorizontalConstraints(rootHorizontalConstraints)
  {
  }

  void AbstractLineBuilder::Reset() noexcept
  {
    _wrapOpportunityList.clear();
    _partialLeadingTextItem = {};
    _previousLine = {};
    _isFirstFormattedLineCandidate = false;
  }

  Maybe<InlineLayoutUnit> AbstractLineBuilder::OverflowWidthAsLeadingForNextLine(
    const InlineContentBreaker::ContinuousContent::RunList &candidateRuns,
    const InlineContentBreaker::Result &lineBreakingResult) const noexcept
  {
    using Action = InlineContentBreaker::Result::Action;
    auto trailingTextRunIndexAsLeadingCandidate = [&]() -> Maybe<size_t>
    {
      krys_debug_assert(lineBreakingResult.ResultAction == Action::Wrap
                        || lineBreakingResult.ResultAction == Action::Break);

      auto trailingRunIndex = [&]() -> Maybe<size_t>
      {
        if (candidateRuns.size() == 1)
        {
          return 0;
        }
        if (lineBreakingResult.ResultAction == Action::Break && lineBreakingResult.PartialTrailing)
        {
          return lineBreakingResult.PartialTrailing->TrailingRunIndex;
        }
        return {};
      };
      auto candidateIndex = trailingRunIndex();
      if (!candidateIndex || !candidateRuns[*candidateIndex].Item.IsText())
      {
        return {};
      }
      auto &inlineTextItem = Downcast<InlineTextItem>(candidateRuns[*candidateIndex].Item);
      // For whitespace content we can only re-use measured width when no position dependent character(s) is
      // present.
      return !inlineTextItem.IsWhitespace() || inlineTextItem.Width().has_value() ? candidateIndex : null;
    };

    auto index = trailingTextRunIndexAsLeadingCandidate();
    if (!index)
    {
      return {};
    }

    auto &overflowingRun = candidateRuns[*index];
    auto *inlineTextItem = DynamicDowncast<InlineTextItem>(overflowingRun.Item);
    if (!inlineTextItem)
    {
      krys_debug_assert(false);
      return {};
    }
    if (IsFirstFormattedLineCandidate())
    {
      auto &usedStyle = overflowingRun.Style;
      auto &style = overflowingRun.Item.Style();
      if (&usedStyle != &style && !usedStyle.FontCascadeEqual(style))
      {
        // We may have the incorrect text width when styles differ. Just re-measure the text content when we
        // place it on the next line.
        return {};
      }
    }
    auto logicalWidthForNextLineAsLeading = overflowingRun.ContentWidth();
    if (lineBreakingResult.ResultAction == Action::Wrap)
    {
      return logicalWidthForNextLineAsLeading;
    }
    if (lineBreakingResult.ResultAction == Action::Break && lineBreakingResult.PartialTrailing->Partial)
    {
      return logicalWidthForNextLineAsLeading - lineBreakingResult.PartialTrailing->Partial->LogicalWidth;
    }
    return {};
  }

  void AbstractLineBuilder::SetIntrinsicWidthMode(IntrinsicWidthMode intrinsicWidthMode) noexcept
  {
    _intrinsicWidthMode = intrinsicWidthMode;
    _inlineContentBreaker.SetIsMinimumInIntrinsicWidthMode(_intrinsicWidthMode
                                                           == IntrinsicWidthMode::Minimum);
  }

  const RenderStyle &AbstractLineBuilder::RootStyle() const noexcept
  {
    return IsFirstFormattedLineCandidate() ? Root().FirstLineStyle() : Root().Style();
  }

  const InlineLayoutState &AbstractLineBuilder::GetLayoutState() const noexcept
  {
    return GetFormattingContext().GetLayoutState();
  }

  InlineLayoutState &AbstractLineBuilder::GetLayoutState() noexcept
  {
    return GetFormattingContext().GetLayoutState();
  }
}
