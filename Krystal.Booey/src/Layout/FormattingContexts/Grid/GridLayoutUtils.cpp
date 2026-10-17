#include "Krystal.Booey/Layout/FormattingContexts/Grid/GridLayout.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Grid/PlacedGridItem.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Core/Assert.hpp"
#include <algorithm>
#include <numeric>

namespace krys::boo::layout::GridLayoutUtils
{
  LayoutUnit ComputeGapValue(const style::GapGutter &gap) noexcept
  {
    if (gap.IsNormal())
    {
      return {};
    }

    // Only handle fixed length gaps for now
    if (auto fixedGap = gap.TryFixed())
    {
      return style::Evaluate<LayoutUnit>(*fixedGap, style::ZoomNeeded {});
    }

    krys_debug_assert(false);
    return {};
  }

  LayoutUnit UsedInlineSizeForGridItem(const PlacedGridItem &placedGridItem, LayoutUnit borderAndPadding,
                                       const TrackSizes &usedColumnSizes, LayoutUnit columnsGap) noexcept
  {
    auto columnsSize = [&]
    {
      auto columnsStartPosition =
        ComputeGridLinePosition(placedGridItem.ColumnStartLine(), usedColumnSizes, columnsGap);
      auto columnsEndPosition =
        ComputeGridLineEndPosition(placedGridItem.ColumnEndLine(), usedColumnSizes, columnsGap);
      krys_debug_assert(columnsEndPosition >= columnsStartPosition);
      return columnsEndPosition - columnsStartPosition;
    };

    const auto &inlineAxisSizes = placedGridItem.InlineAxisSizes();
    krys_debug_assert((inlineAxisSizes.MinimumSize.IsFixed() || inlineAxisSizes.MinimumSize.IsAuto())
                      && (inlineAxisSizes.MaximumSize.IsFixed() || inlineAxisSizes.MaximumSize.IsNone()));

    const auto &preferredSize = inlineAxisSizes.PreferredSize;
    if (auto fixedInlineSize = preferredSize.TryFixed())
    {
      return style::Evaluate<LayoutUnit>(*fixedInlineSize, placedGridItem.UsedZoom()) + borderAndPadding;
    }

    if (preferredSize.IsAuto())
    {
      // Grid item calculations for automatic sizes in a given dimensions vary by their
      // self-alignment values:
      auto alignmentPosition = placedGridItem.InlineAxisAlignment().Position();

      // normal:
      // If the grid item has no preferred aspect ratio, and no natural size in the relevant
      // axis (if it is a replaced element), the grid item is sized as for align-self: stretch.
      //
      // https://www.w3.org/TR/css-align-3/#propdef-align-self
      //
      // When the box’s computed width/height (as appropriate to the axis) is auto and neither of
      // its margins (in the appropriate axis) are auto, sets the box’s used size to the length
      // necessary to make its outer size as close to filling the alignment container as possible
      // while still respecting the constraints imposed by min-height/min-width/max-height/max-width.
      const auto &marginStart = inlineAxisSizes.MarginStart;
      const auto &marginEnd = inlineAxisSizes.MarginEnd;
      auto isStretched =
        alignmentPosition == ItemPosition::Normal || alignmentPosition == ItemPosition::Stretch;
      if (isStretched && !placedGridItem.HasPreferredAspectRatio() && !placedGridItem.IsReplacedElement()
          && !marginStart.IsAuto() && !marginEnd.IsAuto())
      {
        const auto &usedZoom = placedGridItem.UsedZoom();

        // https://drafts.csswg.org/css-grid-1/#min-size-auto
        // The automatic minimum size is zero, as the item only spans fixed tracks.
        auto minimumSize = LayoutUnit {};
        if (auto fixedMinimumSize = inlineAxisSizes.MinimumSize.TryFixed())
        {
          minimumSize = style::Evaluate<LayoutUnit>(*fixedMinimumSize, usedZoom);
        }
        auto maximumSize = [&]
        {
          const auto &computedMaximumSize = inlineAxisSizes.MaximumSize;
          if (computedMaximumSize.IsNone())
          {
            return LayoutUnit::Max();
          }
          return style::Evaluate<LayoutUnit>(*computedMaximumSize.TryFixed(), usedZoom);
        };

        auto stretchedWidth = columnsSize() - style::Evaluate<LayoutUnit>(*marginStart.TryFixed(), usedZoom)
                              - style::Evaluate<LayoutUnit>(*marginEnd.TryFixed(), usedZoom)
                              - borderAndPadding;
        return std::max(minimumSize, std::min(maximumSize(), stretchedWidth));
      }

      // FIXME: Not implemented yet.
      krys_debug_assert(false);
      return {};
    }

    // FIXME: Not implemented yet.
    krys_debug_assert(false);
    return {};
  }

  LayoutUnit UsedBlockSizeForGridItem(const PlacedGridItem &placedGridItem) noexcept
  {
    const auto &blockAxisSizes = placedGridItem.BlockAxisSizes();
    if (auto fixedBlockSize = blockAxisSizes.PreferredSize.TryFixed())
    {
      return style::Evaluate<LayoutUnit>(*fixedBlockSize, placedGridItem.UsedZoom());
    }

    // FIXME: Not implemented yet.
    krys_debug_assert(false);
    return {};
  }

  LayoutUnit ComputeGridLinePosition(size_t gridLineIndex, const TrackSizes &trackSizes,
                                     LayoutUnit gap) noexcept
  {
    krys_debug_assert(gridLineIndex <= trackSizes.size());
    auto sumOfTrackSizes = std::reduce(trackSizes.begin(), trackSizes.begin() + gridLineIndex, LayoutUnit {});

    // Every track before grid line i is followed by a gap, so the track after it starts after i gaps.
    return sumOfTrackSizes + gap * static_cast<int>(gridLineIndex);
  }

  LayoutUnit ComputeGridLineEndPosition(size_t gridLineIndex, const TrackSizes &trackSizes,
                                        LayoutUnit gap) noexcept
  {
    krys_debug_assert(gridLineIndex <= trackSizes.size());
    auto sumOfTrackSizes = std::reduce(trackSizes.begin(), trackSizes.begin() + gridLineIndex, LayoutUnit {});

    // For grid line i, there are i-1 gaps before it (between the i tracks)
    auto numberOfGaps = gridLineIndex > 0 ? gridLineIndex - 1 : 0;
    return sumOfTrackSizes + gap * static_cast<int>(numberOfGaps);
  }
}
