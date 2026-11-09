/*
 * Copyright (C) 2012 Adobe Systems Incorporated. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <algorithm>

namespace krys::boo
{
  /// @brief A horizontal interval `[x1, x2]`, or undefined.
  template <typename T>
  class ShapeInterval
  {
  private:
    // The initial values don't matter (unless you're looking at them in the debugger) so long as
    // IsUndefined() is true.
    T _x1 {-1};
    T _x2 {-2};

  public:
    constexpr ShapeInterval() noexcept = default;

    constexpr ShapeInterval(T x1, T x2) noexcept : _x1(x1), _x2(x2)
    {
      krys_debug_assert(x2 >= x1);
    }

    KRYS_NODISCARD constexpr bool IsUndefined() const noexcept
    {
      return _x2 < _x1;
    }
    KRYS_NODISCARD constexpr T X1() const noexcept
    {
      return IsUndefined() ? 0 : _x1;
    }
    KRYS_NODISCARD constexpr T X2() const noexcept
    {
      return IsUndefined() ? 0 : _x2;
    }
    KRYS_NODISCARD constexpr T Width() const noexcept
    {
      return IsUndefined() ? 0 : _x2 - _x1;
    }
    KRYS_NODISCARD constexpr bool IsEmpty() const noexcept
    {
      return IsUndefined() ? true : _x1 == _x2;
    }

    constexpr void Set(T x1, T x2) noexcept
    {
      krys_debug_assert(x2 >= x1);
      _x1 = x1;
      _x2 = x2;
    }

    KRYS_NODISCARD constexpr bool Overlaps(const ShapeInterval &interval) const noexcept
    {
      if (IsUndefined() || interval.IsUndefined())
      {
        return false;
      }
      return X2() >= interval.X1() && X1() <= interval.X2();
    }

    KRYS_NODISCARD constexpr bool Contains(const ShapeInterval &interval) const noexcept
    {
      if (IsUndefined() || interval.IsUndefined())
      {
        return false;
      }
      return X1() <= interval.X1() && X2() >= interval.X2();
    }

    constexpr void Unite(const ShapeInterval &interval) noexcept
    {
      if (interval.IsUndefined())
      {
        return;
      }
      if (IsUndefined())
      {
        Set(interval.X1(), interval.X2());
      }
      else
      {
        Set(std::min<T>(X1(), interval.X1()), std::max<T>(X2(), interval.X2()));
      }
    }

    KRYS_NODISCARD constexpr bool operator==(const ShapeInterval &) const noexcept = default;
  };

  using IntShapeInterval = ShapeInterval<int32>;
  using FloatShapeInterval = ShapeInterval<float>;

  using IntShapeIntervals = List<IntShapeInterval>;
  using FloatShapeIntervals = List<FloatShapeInterval>;
}
