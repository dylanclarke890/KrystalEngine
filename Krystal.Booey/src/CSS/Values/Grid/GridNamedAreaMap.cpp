#include "Krystal.Booey/CSS/Values/Grid/GridNamedAreaMap.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/HashSet.hpp"

namespace krys::boo::css
{
  bool AddRow(GridNamedAreaMap &gridAreaMap, const GridNamedAreaMapRow &row) noexcept
  {
    if (!gridAreaMap.RowCount)
    {
      gridAreaMap.ColumnCount = row.size();
      if (!gridAreaMap.ColumnCount)
      {
        return false;
      }
    }
    else if (gridAreaMap.ColumnCount != row.size())
    {
      // The declaration is invalid if all the rows don't have the number of columns.
      return false;
    }

    for (size_t currentColumn = 0; currentColumn < gridAreaMap.ColumnCount; ++currentColumn)
    {
      const auto &gridAreaName = row[currentColumn];

      // Unnamed areas are always valid (we consider them to be 1x1).
      if (gridAreaName == u8".")
      {
        continue;
      }

      auto lookAheadColumn = currentColumn + 1;
      while (lookAheadColumn < gridAreaMap.ColumnCount && row[lookAheadColumn] == gridAreaName)
      {
        lookAheadColumn++;
      }

      auto [iterator, isNewEntry] = gridAreaMap.Map.try_emplace(
        gridAreaName,
        GridSpan::TranslatedDefiniteGridSpan(static_cast<unsigned>(gridAreaMap.RowCount),
                                             static_cast<unsigned>(gridAreaMap.RowCount + 1)),
        GridSpan::TranslatedDefiniteGridSpan(static_cast<unsigned>(currentColumn),
                                             static_cast<unsigned>(lookAheadColumn)));

      if (!isNewEntry)
      {
        auto &gridArea = iterator->second;

        // The following checks test that the grid area is a single filled-in rectangle.
        // 1. The new row is adjacent to the previously parsed row.
        if (gridAreaMap.RowCount != gridArea.Rows.EndLine())
        {
          return false;
        }

        // 2. The new area starts at the same position as the previously parsed area.
        if (currentColumn != gridArea.Columns.StartLine())
        {
          return false;
        }

        // 3. The new area ends at the same position as the previously parsed area.
        if (lookAheadColumn != gridArea.Columns.EndLine())
        {
          return false;
        }

        gridArea.Rows =
          GridSpan::TranslatedDefiniteGridSpan(gridArea.Rows.StartLine(), gridArea.Rows.EndLine() + 1);
      }

      currentColumn = lookAheadColumn - 1;
    }

    ++gridAreaMap.RowCount;
    return true;
  }

#pragma region Serialisation

  namespace
  {
    void SerialiseGridNamedAreaMapPosition(StringBuilder &builder, const GridNamedAreaMap::MapType &map,
                                           size_t row, size_t column) noexcept
    {
      HashSet<CSSOMString> candidates;
      for (const auto &[name, area] : map)
      {
        if (row >= area.Rows.StartLine() && row < area.Rows.EndLine())
        {
          candidates.insert(name);
        }
      }

      for (const auto &[name, area] : map)
      {
        if (column >= area.Columns.StartLine() && column < area.Columns.EndLine()
            && candidates.contains(name))
        {
          builder.Append(name);
          return;
        }
      }

      builder.Append(u8'.');
    }
  }

  void Serialiser<GridNamedAreaMap>::operator()(StringBuilder &builder, const SerialisationContext &,
                                                const GridNamedAreaMap &value) noexcept
  {
    for (size_t row = 0; row < value.RowCount; ++row)
    {
      builder.Append(u8'"');
      for (size_t column = 0; column < value.ColumnCount; ++column)
      {
        SerialiseGridNamedAreaMapPosition(builder, value.Map, row, column);
        if (column != value.ColumnCount - 1)
        {
          builder.Append(u8' ');
        }
      }
      builder.Append(u8'"');
      if (row != value.RowCount - 1)
      {
        builder.Append(u8' ');
      }
    }
  }

#pragma endregion
}
