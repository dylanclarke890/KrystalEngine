/*
 * Copyright (C) 2018 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Geometry/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatSize.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutPoint.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutSize.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Booey/Layout/MarginTypes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
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
