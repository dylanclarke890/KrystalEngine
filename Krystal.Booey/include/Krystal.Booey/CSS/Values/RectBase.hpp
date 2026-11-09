/**
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004-2023 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief Four values for the top, right, bottom and left sides; shared by `Quad` and `Rect`.
  class RectBase
  {
  private:
    const Ref<const Value> _top;
    const Ref<const Value> _right;
    const Ref<const Value> _bottom;
    const Ref<const Value> _left;

  protected:
    explicit RectBase(Ref<Value> value) noexcept
        : _top(value), _right(value), _bottom(value), _left(krys::move(value))
    {
    }

    RectBase(Ref<Value> top, Ref<Value> right, Ref<Value> bottom, Ref<Value> left) noexcept
        : _top(krys::move(top)), _right(krys::move(right)), _bottom(krys::move(bottom)),
          _left(krys::move(left))
    {
    }

    ~RectBase() noexcept = default;

  public:
    KRYS_NODISCARD const Value &Top() const noexcept
    {
      return *_top;
    }

    KRYS_NODISCARD const Value &Right() const noexcept
    {
      return *_right;
    }

    KRYS_NODISCARD const Value &Bottom() const noexcept
    {
      return *_bottom;
    }

    KRYS_NODISCARD const Value &Left() const noexcept
    {
      return *_left;
    }

    KRYS_NODISCARD bool Equals(const RectBase &other) const noexcept
    {
      return CompareValue(_top, other._top) && CompareValue(_right, other._right)
             && CompareValue(_left, other._left) && CompareValue(_bottom, other._bottom);
    }
  };
}
