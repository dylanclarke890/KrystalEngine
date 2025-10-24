#pragma once

#include "Krystal.UI.Styles/Enums/Dimension.hpp"
#include "Krystal.UI.Styles/Enums/Direction.hpp"
#include "Krystal.UI.Styles/Enums/Edge.hpp"
#include "Krystal.UI.Styles/Enums/FlexDirection.hpp"
#include "Krystal.UI.Styles/Enums/PhysicalEdge.hpp"

namespace Krys::UI::Styles
{
  NO_DISCARD inline FlexDirection ResolveDirection(const FlexDirection flexDirection,
                                                   const Direction direction) noexcept
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

  NO_DISCARD inline FlexDirection ResolveCrossDirection(const FlexDirection flexDirection,
                                                        const Direction direction) noexcept
  {
    return IsColumn(flexDirection) ? ResolveDirection(FlexDirection::Row, direction) : FlexDirection::Column;
  }

  NO_DISCARD inline PhysicalEdge FlexStartEdge(FlexDirection flexDirection) noexcept
  {
    switch (flexDirection)
    {
      case FlexDirection::Column:        return PhysicalEdge::Top;
      case FlexDirection::ColumnReverse: return PhysicalEdge::Bottom;
      case FlexDirection::Row:           return PhysicalEdge::Left;
      case FlexDirection::RowReverse:    return PhysicalEdge::Right;
    }

    std::unreachable();
  }

  NO_DISCARD inline PhysicalEdge FlexEndEdge(FlexDirection flexDirection) noexcept
  {
    switch (flexDirection)
    {
      case FlexDirection::Column:        return PhysicalEdge::Bottom;
      case FlexDirection::ColumnReverse: return PhysicalEdge::Top;
      case FlexDirection::Row:           return PhysicalEdge::Right;
      case FlexDirection::RowReverse:    return PhysicalEdge::Left;
    }

    std::unreachable();
  }

  NO_DISCARD inline PhysicalEdge InlineStartEdge(FlexDirection flexDirection, Direction direction) noexcept
  {
    if (IsRow(flexDirection))
    {
      return direction == Direction::RTL ? PhysicalEdge::Right : PhysicalEdge::Left;
    }

    return PhysicalEdge::Top;
  }

  NO_DISCARD inline PhysicalEdge InlineEndEdge(FlexDirection flexDirection, Direction direction) noexcept
  {
    if (IsRow(flexDirection))
    {
      return direction == Direction::RTL ? PhysicalEdge::Left : PhysicalEdge::Right;
    }

    return PhysicalEdge::Bottom;
  }

  NO_DISCARD inline Dimension ToDimension(FlexDirection flexDirection) noexcept
  {
    switch (flexDirection)
    {
      case FlexDirection::Column:        return Dimension::Height;
      case FlexDirection::ColumnReverse: return Dimension::Height;
      case FlexDirection::Row:           return Dimension::Width;
      case FlexDirection::RowReverse:    return Dimension::Width;
    }

    std::unreachable();
  }
}