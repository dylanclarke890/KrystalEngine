#include "Krystal.Booey/Rendering/RenderGrid.hpp"

namespace krys::boo
{
  RenderGrid::RenderGrid(dom::Element &element, RenderStyle &&style) noexcept
      : RenderBlock(Type::Grid, element, krys::move(style), {})
  {
    krys_debug_assert(IsRenderGrid());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderGrid::~RenderGrid() noexcept = default;

  utf8_stringview RenderGrid::RenderName() const noexcept
  {
    if (IsFloating())
    {
      return u8"RenderGrid (floating)";
    }
    if (IsOutOfFlowPositioned())
    {
      return u8"RenderGrid (positioned)";
    }
    if (IsAnonymous())
    {
      return u8"RenderGrid (generated)";
    }
    if (IsRelativelyPositioned())
    {
      return u8"RenderGrid (relative positioned)";
    }
    return u8"RenderGrid";
  }

  const List<LayoutUnit> &RenderGrid::Positions(style::GridTrackSizingDirection) const noexcept
  {
    krys_unreachable();
  }

  bool RenderGrid::IsSubgrid(style::GridTrackSizingDirection direction) const noexcept
  {
    // If the grid container is forced to establish an independent formatting
    // context (like contain layout, or position:absolute), then the used value
    // of grid-template-rows/columns is 'none' and the container is not a subgrid.
    // https://drafts.csswg.org/css-grid-2/#subgrid-listing
    if (EstablishesIndependentFormattingContextIgnoringDisplayType(Style()))
    {
      return false;
    }
    const auto &tracks = direction == style::GridTrackSizingDirection::Columns ? Style().GridTemplateColumns()
                                                                               : Style().GridTemplateRows();
    if (!tracks.Subgrid)
    {
      return false;
    }
    auto *renderGrid = DynamicDowncast<RenderGrid>(Parent());
    if (!renderGrid)
    {
      return false;
    }
    return !renderGrid->IsMasonry(direction);
  }

  bool RenderGrid::IsSubgridInParentDirection(style::GridTrackSizingDirection parentDirection) const noexcept
  {
    auto *renderGrid = DynamicDowncast<RenderGrid>(Parent());
    if (!renderGrid)
    {
      return false;
    }
    // The flow-aware direction: an orthogonal item's columns are its parent's rows.
    auto isOrthogonal = IsHorizontalWritingMode() != renderGrid->IsHorizontalWritingMode();
    auto direction = !isOrthogonal ? parentDirection
                     : parentDirection == style::GridTrackSizingDirection::Columns
                       ? style::GridTrackSizingDirection::Rows
                       : style::GridTrackSizingDirection::Columns;
    return IsSubgrid(direction);
  }

  uint32 RenderGrid::NumTracks(style::GridTrackSizingDirection) const noexcept
  {
    krys_unreachable();
  }

  uint32 RenderGrid::AutoRepeatCountForDirection(style::GridTrackSizingDirection) const noexcept
  {
    krys_unreachable();
  }

  List<LayoutUnit> RenderGrid::TrackSizesForComputedStyle(style::GridTrackSizingDirection) const noexcept
  {
    krys_unreachable();
  }

  int32 RenderGrid::ExplicitGridStartForDirection(style::GridTrackSizingDirection) const noexcept
  {
    krys_unreachable();
  }

  bool RenderGrid::IsMasonry() const noexcept
  {
    return IsMasonry(style::GridTrackSizingDirection::Rows)
           || IsMasonry(style::GridTrackSizingDirection::Columns);
  }

  // Masonry Spec Section 2
  // "If masonry is specified for both grid-template-columns and grid-template-rows, then the used value for
  // grid-template-columns is none, and thus the inline axis will be the grid axis."
  bool RenderGrid::IsMasonry(style::GridTrackSizingDirection direction) const noexcept
  {
    // isSubgrid will return false if the masonry axis matches. Need to check style if we are a subgrid
    const auto &tracks = direction == style::GridTrackSizingDirection::Columns ? Style().GridTemplateColumns()
                                                                               : Style().GridTemplateRows();
    if (auto *parentGrid = DynamicDowncast<RenderGrid>(Parent()); parentGrid && tracks.Subgrid)
    {
      return parentGrid->IsMasonry(direction);
    }
    if (Style().Display() != DisplayType::GridLanes && Style().Display() != DisplayType::InlineGridLanes)
    {
      return false;
    }
    return (direction == style::GridTrackSizingDirection::Columns) == Style().GetGridAutoFlow().IsColumn();
  }

  bool RenderGrid::IsExtrinsicallySized() const noexcept
  {
    // FIXME: Whether the grid's size doesn't depend on its content (auto width in block layout, fixed height,
    // no content-sized tracks, and items placed within the explicit tracks), once there's grid layout.
    return false;
  }
}
