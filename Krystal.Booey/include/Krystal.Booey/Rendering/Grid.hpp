#pragma once

#include "Krystal.Booey/Rendering/OrderIterator.hpp"
#include "Krystal.Booey/Rendering/Style/GridArea.hpp"
#include "Krystal.Booey/Rendering/Style/GridSpan.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTrackSizingDirection.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class RenderBox;
  class RenderGrid;

  using GridCell = List<RenderBox *>;
  using GridAsMatrix = List<List<GridCell>>;
  using OrderedTrackIndexSet = ListHashSet<size_t>;

  /// @brief A grid container's grid: the cells its items occupy, the explicit grid's offset in the implicit
  /// grid, and the auto repeat tracks.
  class Grid final
  {
    KRYS_NON_COPY_MOVABLE(Grid);

  private:
    OrderIterator _orderIterator;

    uint32 _explicitColumnStart {0};
    uint32 _explicitRowStart {0};

    uint32 _autoRepeatColumns {0};
    uint32 _autoRepeatRows {0};

    uint32 _maxColumns {0};
    uint32 _maxRows {0};

    bool _needsItemsPlacement {true};

    GridAsMatrix _grid;

    // Keyed by address: an item that leaves the grid marks it for placement, which clears the areas and
    // cells.
    HashMap<const RenderBox *, GridArea> _gridItemArea;

    UniquePtr<OrderedTrackIndexSet> _autoRepeatEmptyColumns;
    UniquePtr<OrderedTrackIndexSet> _autoRepeatEmptyRows;

  public:
    explicit Grid(RenderGrid &grid) noexcept;

    KRYS_NODISCARD uint32 NumTracks(style::GridTrackSizingDirection direction) const noexcept;

    void EnsureGridSize(uint32 maximumRowSize, uint32 maximumColumnSize) noexcept;
    GridArea Insert(RenderBox &gridItem, const GridArea &area) noexcept;

    // Note that each in flow child of a grid container becomes a grid item. This means that
    // this method will return false for a grid container with only out of flow children.
    KRYS_NODISCARD bool HasGridItems() const noexcept
    {
      return !_gridItemArea.empty();
    }

    KRYS_NODISCARD GridArea GridItemArea(const RenderBox &item) const noexcept;
    void SetGridItemArea(const RenderBox &item, GridArea area) noexcept;

    KRYS_NODISCARD GridSpan GridItemSpan(const RenderBox &gridItem,
                                         style::GridTrackSizingDirection direction) const noexcept;
    KRYS_NODISCARD GridSpan GridItemSpanIgnoringCollapsedTracks(
      const RenderBox &gridItem, style::GridTrackSizingDirection direction) const noexcept;

    KRYS_NODISCARD const GridCell &Cell(uint32 row, uint32 column) const noexcept;

    KRYS_NODISCARD uint32 ExplicitGridStart(style::GridTrackSizingDirection direction) const noexcept;
    void SetExplicitGridStart(uint32 rowStart, uint32 columnStart) noexcept;

    KRYS_NODISCARD uint32 AutoRepeatTracks(style::GridTrackSizingDirection direction) const noexcept;
    void SetAutoRepeatTracks(uint32 autoRepeatRows, uint32 autoRepeatColumns) noexcept;

    void SetClampingForSubgrid(uint32 maxRows, uint32 maxColumns) noexcept;

    void ClampAreaToSubgridIfNeeded(GridArea &area) noexcept;

    void SetAutoRepeatEmptyColumns(UniquePtr<OrderedTrackIndexSet> autoRepeatEmptyColumns) noexcept;
    void SetAutoRepeatEmptyRows(UniquePtr<OrderedTrackIndexSet> autoRepeatEmptyRows) noexcept;

    KRYS_NODISCARD bool HasAutoRepeatEmptyTracks(style::GridTrackSizingDirection direction) const noexcept;
    KRYS_NODISCARD bool IsEmptyAutoRepeatTrack(style::GridTrackSizingDirection direction,
                                               uint32 line) const noexcept;

    KRYS_NODISCARD OrderedTrackIndexSet *
      AutoRepeatEmptyTracks(style::GridTrackSizingDirection direction) const noexcept;

    KRYS_NODISCARD OrderIterator &GetOrderIterator() noexcept
    {
      return _orderIterator;
    }

    void SetNeedsItemsPlacement(bool needsItemsPlacement) noexcept;
    KRYS_NODISCARD bool NeedsItemsPlacement() const noexcept
    {
      return _needsItemsPlacement;
    }

    void SetupGridForMasonryLayout() noexcept;
    KRYS_NODISCARD uint32 MaxRows() const noexcept
    {
      return _maxRows;
    }
    KRYS_NODISCARD uint32 MaxColumns() const noexcept
    {
      return _maxColumns;
    }

  private:
    void EnsureStorageForRow(uint32 row) noexcept;
  };

  /// @brief Walks the items of one row or column of a grid, or finds empty areas in it.
  class GridIterator
  {
    KRYS_NON_COPYABLE(GridIterator);

  private:
    const Grid &_grid;
    style::GridTrackSizingDirection _direction;
    uint32 _rowIndex;
    uint32 _columnIndex;
    uint32 _gridItemIndex;

  public:
    // |direction| is the direction that is fixed to |fixedTrackIndex| so e.g
    // GridIterator(m_grid, ForColumns, 1) will walk over the rows of the 2nd column.
    GridIterator(const Grid &grid, style::GridTrackSizingDirection direction, uint32 fixedTrackIndex,
                 uint32 varyingTrackIndex = 0) noexcept;
    GridIterator(GridIterator &&) noexcept = default;

    KRYS_NODISCARD static GridIterator CreateForSubgrid(const RenderGrid &subgrid, const GridIterator &outer,
                                                        GridSpan subgridSpanInOuter) noexcept;

    KRYS_NODISCARD RenderBox *NextGridItem() noexcept;
    KRYS_NODISCARD bool IsEmptyAreaEnough(uint32 rowSpan, uint32 columnSpan) const noexcept;
    KRYS_NODISCARD Maybe<GridArea> NextEmptyGridArea(uint32 fixedTrackSpan, uint32 varyingTrackSpan) noexcept;

    KRYS_NODISCARD style::GridTrackSizingDirection Direction() const noexcept
    {
      return _direction;
    }
  };
}
