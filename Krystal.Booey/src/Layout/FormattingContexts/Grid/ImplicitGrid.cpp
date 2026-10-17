#include "Krystal.Booey/Layout/FormattingContexts/Grid/GridLayout.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Grid/UnplacedGridItem.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include <algorithm>

namespace krys::boo::layout
{
  // The implicit grid is created from the explicit grid + items that are placed outside
  // of the explicit grid. Since we know the explicit tracks from style we start the
  // implicit grid as exactly the explicit grid and allow placement to add implicit
  // tracks and grow the grid.

  ImplicitGrid::ImplicitGrid(size_t totalColumnsCount, size_t totalRowsCount) noexcept
      : _gridMatrix(totalRowsCount, List<GridCell>(totalColumnsCount))
  {
  }

  void ImplicitGrid::InsertUnplacedGridItem(const UnplacedGridItem &unplacedGridItem) noexcept
  {
    // https://drafts.csswg.org/css-grid/#common-uses-numeric
    // Grid positions have already been normalized to non-negative matrix indices.
    auto [columnStart, columnEnd] = unplacedGridItem.NormalizedColumnStartEnd();
    auto [rowStart, rowEnd] = unplacedGridItem.NormalizedRowStartEnd();

    // Multi-cell items (spanning multiple columns) are not yet supported.
    if (columnEnd - columnStart > 1)
    {
      // FIXME: Not implemented yet.
      krys_debug_assert(false);
      return;
    }

    // Multi-cell items (spanning multiple rows) are not yet supported.
    if (rowEnd - rowStart > 1)
    {
      // FIXME: Not implemented yet.
      krys_debug_assert(false);
      return;
    }

    for (auto rowIndex = rowStart; rowIndex < rowEnd; ++rowIndex)
    {
      for (auto columnIndex = columnStart; columnIndex < columnEnd; ++columnIndex)
      {
        _gridMatrix[rowIndex][columnIndex].push_back(unplacedGridItem);
      }
    }
  }

  GridAreas ImplicitGrid::GetGridAreas() const noexcept
  {
    GridAreas gridAreas;
    gridAreas.reserve(RowsCount() * ColumnsCount());

    // An item is in the grid areas once, at the first cell it's found in.
    HashSet<const ElementBox *> seenItems;
    for (size_t rowIndex = 0; rowIndex < _gridMatrix.size(); ++rowIndex)
    {
      for (size_t columnIndex = 0; columnIndex < _gridMatrix[rowIndex].size(); ++columnIndex)
      {
        const auto &gridCell = _gridMatrix[rowIndex][columnIndex];
        for (const auto &unplacedGridItem : gridCell)
        {
          if (!seenItems.insert(&unplacedGridItem.LayoutBox()).second)
          {
            continue;
          }
          gridAreas.emplace_back(unplacedGridItem,
                                 GridAreaLines {columnIndex, columnIndex + 1, rowIndex, rowIndex + 1});
        }
      }
    }
    return gridAreas;
  }

  void ImplicitGrid::InsertDefiniteRowItem(const UnplacedGridItem &unplacedGridItem,
                                           GridAutoFlowOptions autoFlowOptions,
                                           HashMap<size_t, size_t> &rowCursors) noexcept
  {
    // Step 2 of CSS Grid auto-placement algorithm:
    // Process items locked to a given row (definite row position, auto column position)
    // See: https://www.w3.org/TR/css-grid-1/#auto-placement-algo

    auto columnSpan = unplacedGridItem.ColumnSpanSize();
    // FIXME: Support multi-column spans
    krys_debug_assert(columnSpan == 1);

    krys_debug_assert(unplacedGridItem.HasDefiniteRowPosition()
                      && !unplacedGridItem.HasDefiniteColumnPosition());
    auto [normalizedRowStart, normalizedRowEnd] = unplacedGridItem.NormalizedRowStartEnd();
    // FIXME: Support multi-row spans
    krys_debug_assert(normalizedRowEnd - normalizedRowStart == 1);

    auto findColumnPosition = [&]() -> Maybe<size_t>
    {
      if (autoFlowOptions.Strategy == PackingStrategy::Dense)
      {
        // Dense packing: always start searching from column 0
        return FindFirstAvailableColumnPosition(normalizedRowStart, normalizedRowEnd, columnSpan, 0);
      }
      // Sparse packing: use per-row cursors to maintain placement order
      // For multi-row items, use the maximum cursor position across all spanned rows
      krys_debug_assert(autoFlowOptions.Strategy == PackingStrategy::Sparse);
      size_t startSearchColumn = 0;
      for (auto row = normalizedRowStart; row < normalizedRowEnd; ++row)
      {
        if (auto it = rowCursors.find(row); it != rowCursors.end())
        {
          startSearchColumn = std::max(startSearchColumn, it->second);
        }
      }
      return FindFirstAvailableColumnPosition(normalizedRowStart, normalizedRowEnd, columnSpan,
                                              startSearchColumn);
    };

    auto growGridToFit = [&](size_t currentColumnsCount)
    {
      // Find the last occupied column in the spanned rows
      size_t lastOccupiedColumn = 0;
      for (auto row = normalizedRowStart; row < normalizedRowEnd; ++row)
      {
        for (auto column = currentColumnsCount; column > 0; --column)
        {
          if (!_gridMatrix[row][column - 1].empty())
          {
            lastOccupiedColumn = std::max(lastOccupiedColumn, column - 1);
            break;
          }
        }
      }

      auto minimumColumnsNeeded = lastOccupiedColumn + 1 + columnSpan;
      for (auto &row : _gridMatrix)
      {
        row.resize(minimumColumnsNeeded);
      }
    };

    auto columnPosition = findColumnPosition();

    if (!columnPosition)
    {
      growGridToFit(ColumnsCount());

      // Retry finding position in the grown grid
      columnPosition = findColumnPosition();
#if KRYS_DEBUG
      krys_assert_msg(columnPosition.has_value(), "Must succeed after growing.");

      // Verify the found position doesn't overlap with existing items
      krys_assert_msg(
        IsCellRangeEmpty(*columnPosition, *columnPosition + columnSpan, normalizedRowStart, normalizedRowEnd),
        "After grid growth, placed item overlaps with occupied cells.");

      // The grid grew past every occupied column, so its last column is empty.
      auto hasEmptyLastColumn = [&]
      {
        return std::ranges::all_of(_gridMatrix, [](const List<GridCell> &row) { return row.back().empty(); });
      };
      krys_assert(hasEmptyLastColumn());
#endif
    }

    InsertItemInArea(unplacedGridItem, *columnPosition, *columnPosition + columnSpan, normalizedRowStart,
                     normalizedRowEnd);

    if (autoFlowOptions.Strategy != PackingStrategy::Dense)
    {
      for (auto row = normalizedRowStart; row < normalizedRowEnd; ++row)
      {
        rowCursors[row] = *columnPosition + columnSpan;
      }
    }
  }

  Maybe<size_t> ImplicitGrid::FindFirstAvailableColumnPosition(size_t rowStart, size_t rowEnd,
                                                               size_t columnSpan,
                                                               size_t startSearchColumn) const noexcept
  {
    auto currentColumnsCount = ColumnsCount();

    // If we can't fit the span starting from the search position, signal that we need to grow the grid
    if (startSearchColumn + columnSpan > currentColumnsCount)
    {
      return null;
    }

    // Search within existing grid bounds
    for (auto columnStart = startSearchColumn; columnStart <= currentColumnsCount - columnSpan; ++columnStart)
    {
      if (IsCellRangeEmpty(columnStart, columnStart + columnSpan, rowStart, rowEnd))
      {
        return columnStart;
      }
    }
    // If we are unable to find a valid position, signal that we need to grow the grid.
    return null;
  }

  bool ImplicitGrid::IsCellRangeEmpty(size_t columnStart, size_t columnEnd, size_t rowStart,
                                      size_t rowEnd) const noexcept
  {
    for (auto row = rowStart; row < rowEnd; ++row)
    {
      for (auto column = columnStart; column < columnEnd; ++column)
      {
        if (!_gridMatrix[row][column].empty())
        {
          return false;
        }
      }
    }
    return true;
  }

  void ImplicitGrid::InsertItemInArea(const UnplacedGridItem &unplacedGridItem, size_t columnStart,
                                      size_t columnEnd, size_t rowStart, size_t rowEnd) noexcept
  {
    for (auto row = rowStart; row < rowEnd; ++row)
    {
      for (auto column = columnStart; column < columnEnd; ++column)
      {
        _gridMatrix[row][column].push_back(unplacedGridItem);
      }
    }
  }
}
