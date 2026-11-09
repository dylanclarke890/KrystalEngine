/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/DoubleSize.hpp"
#include "Krystal.Booey/Graphics/Geometry/IntPoint.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include <cmath>

namespace krys::boo
{
  /// @brief A point in double precision coordinates.
  class DoublePoint
  {
  private:
    double _x {0};
    double _y {0};

  public:
    constexpr DoublePoint() noexcept = default;
    constexpr DoublePoint(double x, double y) noexcept : _x(x), _y(y)
    {
    }

    constexpr DoublePoint(const IntPoint &point) noexcept : _x(point.X()), _y(point.Y())
    {
    }

    constexpr explicit DoublePoint(const DoubleSize &size) noexcept : _x(size.Width()), _y(size.Height())
    {
    }

    KRYS_NODISCARD constexpr static DoublePoint Zero() noexcept
    {
      return DoublePoint();
    }

    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    {
      return !_x && !_y;
    }

    KRYS_NODISCARD constexpr double X() const noexcept
    {
      return _x;
    }

    KRYS_NODISCARD constexpr double Y() const noexcept
    {
      return _y;
    }

    constexpr void Move(double dx, double dy) noexcept
    {
      _x += dx;
      _y += dy;
    }

    constexpr void Move(const IntSize &a) noexcept
    {
      _x += a.Width();
      _y += a.Height();
    }

    constexpr void Move(const DoubleSize &a) noexcept
    {
      _x += a.Width();
      _y += a.Height();
    }

    constexpr void MoveBy(const DoublePoint &a) noexcept
    {
      _x += a.X();
      _y += a.Y();
    }

    constexpr void Scale(double scale) noexcept
    {
      _x *= scale;
      _y *= scale;
    }

    KRYS_NODISCARD constexpr DoublePoint Scaled(double scale) const noexcept
    {
      return {_x * scale, _y * scale};
    }

    KRYS_NODISCARD friend constexpr bool operator==(const DoublePoint &,
                                                    const DoublePoint &) noexcept = default;
  };

  KRYS_NODISCARD constexpr DoublePoint operator+(const DoublePoint &a, const DoubleSize &b) noexcept
  {
    return DoublePoint(a.X() + b.Width(), a.Y() + b.Height());
  }

  KRYS_NODISCARD constexpr DoublePoint operator+(const DoublePoint &a, const DoublePoint &b) noexcept
  {
    return DoublePoint(a.X() + b.X(), a.Y() + b.Y());
  }

  KRYS_NODISCARD constexpr DoubleSize operator-(const DoublePoint &a, const DoublePoint &b) noexcept
  {
    return DoubleSize(a.X() - b.X(), a.Y() - b.Y());
  }

  KRYS_NODISCARD constexpr DoublePoint operator-(const DoublePoint &a, const DoubleSize &b) noexcept
  {
    return DoublePoint(a.X() - b.Width(), a.Y() - b.Height());
  }

  KRYS_NODISCARD inline IntPoint FlooredIntPoint(const DoublePoint &p) noexcept
  {
    return IntPoint(ClampToInteger(std::floor(p.X())), ClampToInteger(std::floor(p.Y())));
  }

  KRYS_NODISCARD inline IntPoint RoundedIntPoint(const DoublePoint &p) noexcept
  {
    return IntPoint(ClampToInteger(std::round(p.X())), ClampToInteger(std::round(p.Y())));
  }

  KRYS_NODISCARD constexpr DoubleSize ToDoubleSize(const DoublePoint &a) noexcept
  {
    return DoubleSize(a.X(), a.Y());
  }
}
