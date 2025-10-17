#pragma once

#include "Krystal.UI.Layout/Enums/Dimension.hpp"
#include "Krystal.UI.Layout/Enums/Direction.hpp"
#include "Krystal.UI.Layout/Enums/Edge.hpp"
#include "Krystal.UI.Layout/Enums/FlexDirection.hpp"
#include "Krystal.UI.Layout/Enums/PhysicalEdge.hpp"

namespace Krys::UI::Layout
{
  inline bool IsRow(const FlexDirection flexDirection) noexcept
  {
    return flexDirection == FlexDirection::Row || flexDirection == FlexDirection::RowReverse;
  }

  inline bool IsColumn(const FlexDirection flexDirection) noexcept
  {
    return flexDirection == FlexDirection::Column || flexDirection == FlexDirection::ColumnReverse;
  }

  inline FlexDirection ResolveDirection(const FlexDirection flexDirection, const Direction direction) noexcept
  {
    if (direction == Direction::RTL)
    {
      if (flexDirection == FlexDirection::Row)
      {
        return FlexDirection::RowReverse;
      }
      else if (flexDirection == FlexDirection::RowReverse)
      {
        return FlexDirection::Row;
      }
    }

    return flexDirection;
  }

  inline FlexDirection ResolveCrossDirection(const FlexDirection flexDirection,
                                             const Direction direction) noexcept
  {
    return IsColumn(flexDirection) ? ResolveDirection(FlexDirection::Row, direction) : FlexDirection::Column;
  }

  inline PhysicalEdge FlexStartEdge(FlexDirection flexDirection) noexcept
  {
    switch (flexDirection)
    {
      case FlexDirection::Column:        return PhysicalEdge::Top;
      case FlexDirection::ColumnReverse: return PhysicalEdge::Bottom;
      case FlexDirection::Row:           return PhysicalEdge::Left;
      case FlexDirection::RowReverse:    return PhysicalEdge::Right;
    }

    return PhysicalEdge::Right; // should never reach here
  }

  inline PhysicalEdge FlexEndEdge(FlexDirection flexDirection) noexcept
  {
    switch (flexDirection)
    {
      case FlexDirection::Column:        return PhysicalEdge::Bottom;
      case FlexDirection::ColumnReverse: return PhysicalEdge::Top;
      case FlexDirection::Row:           return PhysicalEdge::Right;
      case FlexDirection::RowReverse:    return PhysicalEdge::Left;
    }

    return PhysicalEdge::Right; // should never reach here
  }

  inline PhysicalEdge InlineStartEdge(FlexDirection flexDirection, Direction direction) noexcept
  {
    if (IsRow(flexDirection))
    {
      return direction == Direction::RTL ? PhysicalEdge::Right : PhysicalEdge::Left;
    }

    return PhysicalEdge::Top;
  }

  inline PhysicalEdge InlineEndEdge(FlexDirection flexDirection, Direction direction) noexcept
  {
    if (IsRow(flexDirection))
    {
      return direction == Direction::RTL ? PhysicalEdge::Left : PhysicalEdge::Right;
    }

    return PhysicalEdge::Bottom;
  }

  inline Dimension ToDimension(FlexDirection flexDirection) noexcept
  {
    switch (flexDirection)
    {
      case FlexDirection::Column:        return Dimension::Height;
      case FlexDirection::ColumnReverse: return Dimension::Height;
      case FlexDirection::Row:           return Dimension::Width;
      case FlexDirection::RowReverse:    return Dimension::Width;
    }

    return Dimension::Width; // should never reach here
  }
}