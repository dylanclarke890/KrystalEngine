#include "Krystal.Booey/Rendering/Grid.hpp"
#include "Krystal.Booey/Rendering/GridLayoutFunctions.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridPosition.hpp"

namespace krys::boo
{
  using style::GridTrackSizingDirection;

  Grid::Grid(RenderGrid &grid) noexcept : _orderIterator(grid)
  {
  }

  uint32 Grid::NumTracks(GridTrackSizingDirection direction) const noexcept
  {
    if (direction == GridTrackSizingDirection::Rows)
    {
      return static_cast<uint32>(_grid.size());
    }
    return _grid.empty() ? 0 : static_cast<uint32>(_grid[0].size());
  }

  void Grid::EnsureGridSize(uint32 maximumRowSize, uint32 maximumColumnSize) noexcept
  {
    krys_debug_assert(static_cast<int>(maximumRowSize) < style::GridPosition::Max() * 2);
    krys_debug_assert(static_cast<int>(maximumColumnSize) < style::GridPosition::Max() * 2);
    size_t oldColumnSize = NumTracks(GridTrackSizingDirection::Columns);
    size_t oldRowSize = NumTracks(GridTrackSizingDirection::Rows);
    if (maximumRowSize > oldRowSize)
    {
      _grid.resize(maximumRowSize);
    }

    // Just grow the first row for now so that we know the requested size,
    // and we'll lazily allocate the others when they get used.
    if (maximumColumnSize > oldColumnSize && maximumRowSize)
    {
      _grid[0].resize(maximumColumnSize);
    }
  }

  void Grid::EnsureStorageForRow(uint32 row) noexcept
  {
    if (_grid[row].size() < _grid[0].size())
    {
      _grid[row].resize(_grid[0].size());
    }
  }

  GridArea Grid::Insert(RenderBox &gridItem, const GridArea &area) noexcept
  {
    GridArea clampedArea = area;
    if (_maxRows)
    {
      clampedArea.Rows.Clamp(static_cast<int>(_maxRows));
    }
    if (_maxColumns)
    {
      clampedArea.Columns.Clamp(static_cast<int>(_maxColumns));
    }

    krys_debug_assert(clampedArea.Rows.IsTranslatedDefinite() && clampedArea.Columns.IsTranslatedDefinite());
    EnsureGridSize(clampedArea.Rows.EndLine(), clampedArea.Columns.EndLine());

    for (auto row : clampedArea.Rows)
    {
      EnsureStorageForRow(row);
      for (auto column : clampedArea.Columns)
      {
        _grid[row][column].push_back(&gridItem);
      }
    }

    SetGridItemArea(gridItem, clampedArea);
    return clampedArea;
  }

  const GridCell &Grid::Cell(uint32 row, uint32 column) const noexcept
  {
    // Storage for rows other than the first is lazily allocated. We can
    // just return a fake entry if the request is outside the allocated area,
    // since it must be empty.
    static const GridCell emptyCell;
    if (row && _grid[row].size() <= column)
    {
      return emptyCell;
    }

    return _grid[row][column];
  }

  void Grid::SetExplicitGridStart(uint32 rowStart, uint32 columnStart) noexcept
  {
    _explicitRowStart = rowStart;
    _explicitColumnStart = columnStart;
  }

  uint32 Grid::ExplicitGridStart(GridTrackSizingDirection direction) const noexcept
  {
    return direction == GridTrackSizingDirection::Rows ? _explicitRowStart : _explicitColumnStart;
  }

  void Grid::SetClampingForSubgrid(uint32 maxRows, uint32 maxColumns) noexcept
  {
    _maxRows = maxRows;
    _maxColumns = maxColumns;
  }

  void Grid::ClampAreaToSubgridIfNeeded(GridArea &area) noexcept
  {
    if (!area.Rows.IsIndefinite())
    {
      if (_maxRows)
      {
        area.Rows.Clamp(static_cast<int>(_maxRows));
      }
    }
    if (!area.Columns.IsIndefinite())
    {
      if (_maxColumns)
      {
        area.Columns.Clamp(static_cast<int>(_maxColumns));
      }
    }
  }

  GridArea Grid::GridItemArea(const RenderBox &item) const noexcept
  {
    auto entry = _gridItemArea.find(&item);
    krys_debug_assert(entry != _gridItemArea.end());
    return entry != _gridItemArea.end() ? entry->second : GridArea {};
  }

  void Grid::SetGridItemArea(const RenderBox &item, GridArea area) noexcept
  {
    _gridItemArea.insert_or_assign(&item, area);
  }

  void Grid::SetAutoRepeatTracks(uint32 autoRepeatRows, uint32 autoRepeatColumns) noexcept
  {
    krys_debug_assert(static_cast<uint32>(style::GridPosition::Max())
                      >= NumTracks(GridTrackSizingDirection::Rows) + autoRepeatRows);
    krys_debug_assert(static_cast<uint32>(style::GridPosition::Max())
                      >= NumTracks(GridTrackSizingDirection::Columns) + autoRepeatColumns);
    _autoRepeatRows = autoRepeatRows;
    _autoRepeatColumns = autoRepeatColumns;
  }

  uint32 Grid::AutoRepeatTracks(GridTrackSizingDirection direction) const noexcept
  {
    return direction == GridTrackSizingDirection::Rows ? _autoRepeatRows : _autoRepeatColumns;
  }

  void Grid::SetAutoRepeatEmptyColumns(UniquePtr<OrderedTrackIndexSet> autoRepeatEmptyColumns) noexcept
  {
    krys_debug_assert(!autoRepeatEmptyColumns || (autoRepeatEmptyColumns->size() <= _autoRepeatColumns));
    _autoRepeatEmptyColumns = krys::move(autoRepeatEmptyColumns);
  }

  void Grid::SetAutoRepeatEmptyRows(UniquePtr<OrderedTrackIndexSet> autoRepeatEmptyRows) noexcept
  {
    krys_debug_assert(!autoRepeatEmptyRows || (autoRepeatEmptyRows->size() <= _autoRepeatRows));
    _autoRepeatEmptyRows = krys::move(autoRepeatEmptyRows);
  }

  bool Grid::HasAutoRepeatEmptyTracks(GridTrackSizingDirection direction) const noexcept
  {
    return direction == GridTrackSizingDirection::Columns ? !!_autoRepeatEmptyColumns
                                                          : !!_autoRepeatEmptyRows;
  }

  bool Grid::IsEmptyAutoRepeatTrack(GridTrackSizingDirection direction, uint32 line) const noexcept
  {
    krys_debug_assert(HasAutoRepeatEmptyTracks(direction));
    return AutoRepeatEmptyTracks(direction)->contains(line);
  }

  OrderedTrackIndexSet *Grid::AutoRepeatEmptyTracks(GridTrackSizingDirection direction) const noexcept
  {
    krys_debug_assert(HasAutoRepeatEmptyTracks(direction));
    return direction == GridTrackSizingDirection::Columns ? _autoRepeatEmptyColumns.get()
                                                          : _autoRepeatEmptyRows.get();
  }

  GridSpan Grid::GridItemSpan(const RenderBox &gridItem, GridTrackSizingDirection direction) const noexcept
  {
    return GridItemArea(gridItem).Span(direction);
  }

  GridSpan Grid::GridItemSpanIgnoringCollapsedTracks(const RenderBox &gridItem,
                                                     GridTrackSizingDirection direction) const noexcept
  {
    auto span = GridItemSpan(gridItem, direction);
    if (!span.StartLine() || !HasAutoRepeatEmptyTracks(direction))
    {
      return span;
    }
    uint32 currentLine = span.StartLine() - 1;

    while (currentLine && IsEmptyAutoRepeatTrack(direction, currentLine))
    {
      currentLine--;
    }
    if (currentLine)
    {
      return GridSpan::TranslatedDefiniteGridSpan(currentLine + 1, span.IntegerSpan());
    }

    // Still need to check if the first track is empty
    return IsEmptyAutoRepeatTrack(direction, currentLine)
             ? GridSpan::TranslatedDefiniteGridSpan(currentLine, span.IntegerSpan())
             : GridSpan::TranslatedDefiniteGridSpan(currentLine + 1, span.IntegerSpan());
  }

  void Grid::SetupGridForMasonryLayout() noexcept
  {
    // FIXME(248472): See if we can resize grid instead of clearing it here:
    // https://bugs.webkit.org/show_bug.cgi?id=248472
    _grid.clear();
    _gridItemArea.clear();
  }

  void Grid::SetNeedsItemsPlacement(bool needsItemsPlacement) noexcept
  {
    _needsItemsPlacement = needsItemsPlacement;

    if (!needsItemsPlacement)
    {
      _grid.shrink_to_fit();
      return;
    }

    _grid.clear();
    _gridItemArea.clear();
    _explicitRowStart = 0;
    _explicitColumnStart = 0;
    _autoRepeatEmptyColumns = nullptr;
    _autoRepeatEmptyRows = nullptr;
    _autoRepeatColumns = 0;
    _autoRepeatRows = 0;
    _maxColumns = 0;
    _maxRows = 0;
  }

  GridIterator::GridIterator(const Grid &grid, GridTrackSizingDirection direction, uint32 fixedTrackIndex,
                             uint32 varyingTrackIndex) noexcept
      : _grid(grid), _direction(direction),
        _rowIndex((direction == GridTrackSizingDirection::Columns) ? varyingTrackIndex : fixedTrackIndex),
        _columnIndex((direction == GridTrackSizingDirection::Columns) ? fixedTrackIndex : varyingTrackIndex),
        _gridItemIndex(0)
  {
    if (_grid.MaxRows())
    {
      krys_debug_assert(_grid.NumTracks(GridTrackSizingDirection::Rows));
      krys_debug_assert(_rowIndex < _grid.NumTracks(GridTrackSizingDirection::Rows));
    }
    if (_grid.MaxColumns())
    {
      krys_debug_assert(_grid.NumTracks(GridTrackSizingDirection::Columns));
      krys_debug_assert(_columnIndex < _grid.NumTracks(GridTrackSizingDirection::Columns));
    }
  }

  RenderBox *GridIterator::NextGridItem() noexcept
  {
    uint32 &varyingTrackIndex = (_direction == GridTrackSizingDirection::Columns) ? _rowIndex : _columnIndex;
    const uint32 endOfVaryingTrackIndex = (_direction == GridTrackSizingDirection::Columns)
                                            ? _grid.NumTracks(GridTrackSizingDirection::Rows)
                                            : _grid.NumTracks(GridTrackSizingDirection::Columns);
    for (; varyingTrackIndex < endOfVaryingTrackIndex; ++varyingTrackIndex)
    {
      const auto &gridItems = _grid.Cell(_rowIndex, _columnIndex);
      if (_gridItemIndex < gridItems.size())
      {
        return gridItems[_gridItemIndex++];
      }

      _gridItemIndex = 0;
    }
    return nullptr;
  }

  bool GridIterator::IsEmptyAreaEnough(uint32 rowSpan, uint32 columnSpan) const noexcept
  {
    // Ignore cells outside current grid as we will grow it later if needed.
    uint32 maxRows = std::min<uint32>(_rowIndex + rowSpan, _grid.NumTracks(GridTrackSizingDirection::Rows));
    uint32 maxColumns =
      std::min<uint32>(_columnIndex + columnSpan, _grid.NumTracks(GridTrackSizingDirection::Columns));

    // This adds a O(N^2) behavior that shouldn't be a big deal as we expect spanning areas to be small.
    for (uint32 row = _rowIndex; row < maxRows; ++row)
    {
      for (uint32 column = _columnIndex; column < maxColumns; ++column)
      {
        auto &gridItems = _grid.Cell(row, column);
        if (!gridItems.empty())
        {
          return false;
        }
      }
    }

    return true;
  }

  Maybe<GridArea> GridIterator::NextEmptyGridArea(uint32 fixedTrackSpan, uint32 varyingTrackSpan) noexcept
  {
    krys_debug_assert(fixedTrackSpan >= 1);
    krys_debug_assert(varyingTrackSpan >= 1);

    if (!_grid.HasGridItems())
    {
      return {};
    }

    uint32 rowSpan = (_direction == GridTrackSizingDirection::Columns) ? varyingTrackSpan : fixedTrackSpan;
    uint32 columnSpan = (_direction == GridTrackSizingDirection::Columns) ? fixedTrackSpan : varyingTrackSpan;

    uint32 &varyingTrackIndex = (_direction == GridTrackSizingDirection::Columns) ? _rowIndex : _columnIndex;
    const uint32 endOfVaryingTrackIndex = (_direction == GridTrackSizingDirection::Columns)
                                            ? _grid.NumTracks(GridTrackSizingDirection::Rows)
                                            : _grid.NumTracks(GridTrackSizingDirection::Columns);
    for (; varyingTrackIndex < endOfVaryingTrackIndex; ++varyingTrackIndex)
    {
      if (IsEmptyAreaEnough(rowSpan, columnSpan))
      {
        auto result =
          GridArea {GridSpan::TranslatedDefiniteGridSpan(_rowIndex, _rowIndex + rowSpan),
                    GridSpan::TranslatedDefiniteGridSpan(_columnIndex, _columnIndex + columnSpan)};
        // Advance the iterator to avoid an infinite loop where we would return the same grid area over and
        // over.
        ++varyingTrackIndex;
        return result;
      }
    }
    return {};
  }

  GridIterator GridIterator::CreateForSubgrid(const RenderGrid &subgrid, const GridIterator &outer,
                                              GridSpan subgridSpanInOuter) noexcept
  {
    krys_debug_assert(subgrid.IsSubgridInParentDirection(outer.Direction()));
    auto *parent = Downcast<RenderGrid>(subgrid.Parent());

    // Translate the current row/column indices into the coordinate
    // space of the subgrid.
    uint32 fixedIndex =
      (outer.Direction() == GridTrackSizingDirection::Columns) ? outer._columnIndex : outer._rowIndex;
    fixedIndex -= subgridSpanInOuter.StartLine();

    auto innerDirection =
      GridLayoutFunctions::FlowAwareDirectionForGridItem(*parent, subgrid, outer.Direction());
    krys_debug_assert(subgrid.IsSubgrid(innerDirection));

    if (GridLayoutFunctions::IsSubgridReversedDirection(*parent, outer.Direction(), subgrid))
    {
      uint32 fixedMax = subgrid.CurrentGrid().NumTracks(innerDirection);
      fixedIndex = fixedMax - fixedIndex - 1;
    }

    return GridIterator(subgrid.CurrentGrid(), innerDirection, fixedIndex);
  }
}
