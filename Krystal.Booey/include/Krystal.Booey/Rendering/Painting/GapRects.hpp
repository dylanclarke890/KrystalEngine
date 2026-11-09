/*
    Copyright (C) 2005, 2006 Apple Inc. All rights reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.


    Some useful definitions needed for laying out elements
*/

#pragma once

#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  /// @brief The selection gaps a block fills: left of the selected content, between it, and right of it.
  struct GapRects
  {
  private:
    LayoutRect _left;
    LayoutRect _center;
    LayoutRect _right;

  public:
    KRYS_NODISCARD const LayoutRect &Left() const noexcept
    {
      return _left;
    }
    KRYS_NODISCARD const LayoutRect &Center() const noexcept
    {
      return _center;
    }
    KRYS_NODISCARD const LayoutRect &Right() const noexcept
    {
      return _right;
    }

    void UniteLeft(const LayoutRect &r) noexcept
    {
      _left.Unite(r);
    }
    void UniteCenter(const LayoutRect &r) noexcept
    {
      _center.Unite(r);
    }
    void UniteRight(const LayoutRect &r) noexcept
    {
      _right.Unite(r);
    }
    void Unite(const GapRects &o) noexcept
    {
      UniteLeft(o.Left());
      UniteCenter(o.Center());
      UniteRight(o.Right());
    }

    operator LayoutRect() const noexcept
    {
      LayoutRect result = _left;
      result.Unite(_center);
      result.Unite(_right);
      return result;
    }

    KRYS_NODISCARD friend bool operator==(const GapRects &, const GapRects &) noexcept = default;
  };
}
