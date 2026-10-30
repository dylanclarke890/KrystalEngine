#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/LayoutPoint.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/Layout/MarginTypes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  using InlineLayoutUnit = float;
  using InlineLayoutPoint = FloatPoint;
  using InlineLayoutSize = FloatSize;
  using InlineLayoutRect = FloatRect;

  // The conversions below would otherwise hide these.
  using boo::ToLayoutPoint;
  using boo::ToLayoutSize;

  struct Position
  {
    constexpr operator LayoutUnit() const noexcept
    {
      return Value;
    }

    friend constexpr auto operator<=>(Position, Position) noexcept = default;

    LayoutUnit Value;
  };

  struct Point
  {
    // FIXME: Use Position<Horizontal>, Position<Vertical> to avoid top/left vs. x/y confusion.
    LayoutUnit X; // left
    LayoutUnit Y; // top

    constexpr Point() noexcept = default;
    constexpr Point(LayoutUnit x, LayoutUnit y) noexcept : X(x), Y(y)
    {
    }
    constexpr Point(LayoutPoint point) noexcept : X(point.X()), Y(point.Y())
    {
    }

    KRYS_NODISCARD constexpr static Point Max() noexcept
    {
      return {LayoutUnit::Max(), LayoutUnit::Max()};
    }

    constexpr void Move(LayoutSize offset) noexcept
    {
      X += offset.Width();
      Y += offset.Height();
    }

    constexpr void MoveBy(LayoutPoint offset) noexcept
    {
      X += offset.X();
      Y += offset.Y();
    }

    constexpr operator LayoutPoint() const noexcept
    {
      return {X, Y};
    }
  };

  // FIXME: Wrap these into structs.
  using PointInContextRoot = Point;
  using PositionInContextRoot = Position;

  struct ContentWidthAndMargin
  {
    LayoutUnit ContentWidth;
    UsedHorizontalMargin UsedMargin;
  };

  struct ContentHeightAndMargin
  {
    LayoutUnit ContentHeight;
    UsedVerticalMargin::NonCollapsedValues NonCollapsedMargin;
  };

  struct HorizontalGeometry
  {
    LayoutUnit Left;
    LayoutUnit Right;
    layout::ContentWidthAndMargin ContentWidthAndMargin;
  };

  struct VerticalGeometry
  {
    LayoutUnit Top;
    LayoutUnit Bottom;
    layout::ContentHeightAndMargin ContentHeightAndMargin;
  };

  struct OverriddenHorizontalValues
  {
    Maybe<LayoutUnit> Width;
    Maybe<UsedHorizontalMargin> Margin;
  };

  struct OverriddenVerticalValues
  {
    // Consider collapsing it.
    Maybe<LayoutUnit> Height;
  };

  KRYS_NODISCARD inline LayoutUnit ToLayoutUnit(InlineLayoutUnit value) noexcept
  {
    return LayoutUnit(value);
  }

  KRYS_NODISCARD inline LayoutUnit CeiledLayoutUnit(InlineLayoutUnit value) noexcept
  {
    return LayoutUnit::FromFloatCeil(value);
  }

  KRYS_NODISCARD constexpr LayoutPoint ToLayoutPoint(const InlineLayoutPoint &point) noexcept
  {
    return LayoutPoint(point);
  }

  KRYS_NODISCARD constexpr LayoutSize ToLayoutSize(const InlineLayoutSize &size) noexcept
  {
    return LayoutSize(size);
  }

  KRYS_NODISCARD inline LayoutRect ToLayoutRect(const InlineLayoutRect &rect) noexcept
  {
    return LayoutRect(rect);
  }

  KRYS_NODISCARD constexpr InlineLayoutUnit MaxInlineLayoutUnit() noexcept
  {
    return Limits<float>::max();
  }

  struct CellSpan
  {
    size_t Column {1};
    size_t Row {1};
  };
}
