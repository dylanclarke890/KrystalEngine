#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTrackSizingDirection.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo
{
  namespace style
  {
    class GridTrackBreadth;
    struct GridTrackFitContentLength;
  }

  class GridSpan;
  class RenderElement;
  class RenderGrid;

  /// @brief The margins a grid item gets from the subgrids between it and the grid whose tracks it's sized
  /// in: their margins, borders and padding at the edges, and half the difference in gaps between them.
  struct ExtraMarginsFromSubgrids
  {
    KRYS_NODISCARD LayoutUnit ExtraTrackStartMargin() const noexcept
    {
      return ExtraMargins.first;
    }
    KRYS_NODISCARD LayoutUnit ExtraTrackEndMargin() const noexcept
    {
      return ExtraMargins.second;
    }
    KRYS_NODISCARD LayoutUnit ExtraTotalMargin() const noexcept
    {
      return ExtraMargins.first + ExtraMargins.second;
    }

    ExtraMarginsFromSubgrids &operator+=(const ExtraMarginsFromSubgrids &rhs) noexcept
    {
      ExtraMargins.first += rhs.ExtraTrackStartMargin();
      ExtraMargins.second += rhs.ExtraTrackEndMargin();
      return *this;
    }

    void AddTrackStartMargin(LayoutUnit extraMargin) noexcept
    {
      ExtraMargins.first += extraMargin;
    }
    void AddTrackEndMargin(LayoutUnit extraMargin) noexcept
    {
      ExtraMargins.second += extraMargin;
    }

    Pair<LayoutUnit, LayoutUnit> ExtraMargins;
  };

  /// @brief Helpers shared by grid layout, track sizing and baseline alignment.
  namespace GridLayoutFunctions
  {
    KRYS_NODISCARD LayoutUnit ComputeMarginLogicalSizeForGridItem(const RenderGrid &grid,
                                                                  style::GridTrackSizingDirection direction,
                                                                  const RenderBox &gridItem) noexcept;
    KRYS_NODISCARD LayoutUnit MarginLogicalSizeForGridItem(const RenderGrid &grid,
                                                           style::GridTrackSizingDirection direction,
                                                           const RenderBox &gridItem) noexcept;
    void SetOverridingContentSizeForGridItem(const RenderGrid &renderGrid, RenderBox &gridItem,
                                             LayoutUnit logicalSize,
                                             style::GridTrackSizingDirection direction) noexcept;
    void ClearOverridingContentSizeForGridItem(const RenderGrid &renderGrid, RenderBox &gridItem,
                                               style::GridTrackSizingDirection direction) noexcept;
    KRYS_NODISCARD bool IsOrthogonalGridItem(const RenderGrid &grid, const RenderBox &gridItem) noexcept;
    KRYS_NODISCARD bool
      IsGridItemInlineSizeDependentOnBlockConstraints(const RenderBox &gridItem, const RenderGrid &parentGrid,
                                                      ItemPosition gridItemAlignSelf) noexcept;
    KRYS_NODISCARD bool IsOrthogonalParent(const RenderGrid &grid, const RenderElement &parent) noexcept;
    KRYS_NODISCARD bool IsAspectRatioBlockSizeDependentGridItem(const RenderBox &gridItem) noexcept;
    KRYS_NODISCARD style::GridTrackSizingDirection
      FlowAwareDirectionForGridItem(const RenderGrid &grid, const RenderBox &gridItem,
                                    style::GridTrackSizingDirection direction) noexcept;
    KRYS_NODISCARD style::GridTrackSizingDirection
      FlowAwareDirectionForParent(const RenderGrid &grid, const RenderElement &parent,
                                  style::GridTrackSizingDirection direction) noexcept;
    KRYS_NODISCARD Maybe<RenderBox::GridAreaSize>
      OverridingContainingBlockContentSizeForGridItem(const RenderBox &gridItem,
                                                      style::GridTrackSizingDirection direction) noexcept;
    KRYS_NODISCARD bool
      HasRelativeOrIntrinsicSizeForGridItem(const RenderBox &gridItem,
                                            style::GridTrackSizingDirection direction) noexcept;

    KRYS_NODISCARD bool IsFlippedDirection(const RenderGrid &grid,
                                           style::GridTrackSizingDirection direction) noexcept;
    KRYS_NODISCARD bool IsSubgridReversedDirection(const RenderGrid &grid,
                                                   style::GridTrackSizingDirection outerDirection,
                                                   const RenderGrid &subgrid) noexcept;
    KRYS_NODISCARD ExtraMarginsFromSubgrids ExtraMarginForSubgridAncestors(
      style::GridTrackSizingDirection direction, const RenderBox &gridItem) noexcept;

    KRYS_NODISCARD uint32 AlignmentContextForBaselineAlignment(const GridSpan &span,
                                                               const ItemPosition &alignment) noexcept;

    KRYS_NODISCARD bool HasAutoMarginsInColumnAxis(const RenderBox &gridItem,
                                                   WritingMode parentWritingMode) noexcept;
    KRYS_NODISCARD bool HasAutoMarginsInRowAxis(const RenderBox &gridItem,
                                                WritingMode parentWritingMode) noexcept;

    KRYS_NODISCARD bool HasAutoSizeInColumnAxis(const RenderBox &gridItem,
                                                WritingMode parentWritingMode) noexcept;
    KRYS_NODISCARD bool HasAutoSizeInRowAxis(const RenderBox &gridItem,
                                             WritingMode parentWritingMode) noexcept;

    KRYS_NODISCARD bool AllowedToStretchGridItemAlongColumnAxis(const RenderBox &gridItem,
                                                                ItemPosition alignSelf,
                                                                WritingMode writingMode) noexcept;
    KRYS_NODISCARD bool AllowedToStretchGridItemAlongRowAxis(const RenderBox &gridItem,
                                                             ItemPosition justifySelf,
                                                             WritingMode writingMode) noexcept;

    KRYS_NODISCARD LayoutUnit AvailableAlignmentSpaceForGridItemBeforeStretching(
      const RenderGrid &grid, LayoutUnit gridAreaBreadthForGridItem, const RenderBox &gridItem,
      style::GridTrackSizingDirection direction) noexcept;

    void UpdateAutoMarginsIfNeeded(RenderBox &gridItem, WritingMode writingMode) noexcept;
    void UpdateAutoMarginsInRowAxisIfNeeded(RenderBox &gridItem, WritingMode writingMode) noexcept;
    void UpdateAutoMarginsInColumnAxisIfNeeded(RenderBox &gridItem, WritingMode writingMode) noexcept;

    KRYS_NODISCARD bool IsRelativeGridTrackBreadthAsAuto(const style::GridTrackFitContentLength &length,
                                                         Maybe<LayoutUnit> availableSpace) noexcept;
    KRYS_NODISCARD bool IsRelativeGridTrackBreadthAsAuto(const style::GridTrackBreadth &length,
                                                         Maybe<LayoutUnit> availableSpace) noexcept;
  }
}
