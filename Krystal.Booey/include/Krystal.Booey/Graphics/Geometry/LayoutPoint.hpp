/*
 * Copyright (c) 2012-2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Geometry/LayoutGeometryForward.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutSize.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Geometry/Point.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief The fractional parts of @p point's coordinates.
  KRYS_NODISCARD constexpr LayoutPoint Fraction(const LayoutPoint &point) noexcept
  {
    return {point.X().Fraction(), point.Y().Fraction()};
  }

  KRYS_NODISCARD constexpr LayoutPoint ToLayoutPoint(const LayoutSize &size) noexcept
  {
    return LayoutPoint(size.Width(), size.Height());
  }

  KRYS_NODISCARD constexpr LayoutSize ToLayoutSize(const LayoutPoint &point) noexcept
  {
    return LayoutSize(point.X(), point.Y());
  }

  KRYS_NODISCARD constexpr PointI FlooredPointI(const LayoutPoint &point) noexcept
  {
    return PointI(point.X().Floor(), point.Y().Floor());
  }

  KRYS_NODISCARD constexpr PointI RoundedPointI(const LayoutPoint &point) noexcept
  {
    return PointI(point.X().Round(), point.Y().Round());
  }

  KRYS_NODISCARD constexpr PointI RoundedPointI(const LayoutSize &size) noexcept
  {
    return RoundedPointI(ToLayoutPoint(size));
  }

  KRYS_NODISCARD constexpr PointI CeiledPointI(const LayoutPoint &point) noexcept
  {
    return PointI(point.X().Ceil(), point.Y().Ceil());
  }

  KRYS_NODISCARD inline LayoutPoint FlooredLayoutPoint(const PointF &p) noexcept
  {
    return LayoutPoint(LayoutUnit::FromFloatFloor(p.X()), LayoutUnit::FromFloatFloor(p.Y()));
  }

  KRYS_NODISCARD inline LayoutPoint CeiledLayoutPoint(const PointF &p) noexcept
  {
    return LayoutPoint(LayoutUnit::FromFloatCeil(p.X()), LayoutUnit::FromFloatCeil(p.Y()));
  }

  KRYS_NODISCARD constexpr SizeI SnappedSizeI(const LayoutSize &size, const LayoutPoint &location) noexcept
  {
    auto snap = [](LayoutUnit a, LayoutUnit b)
    {
      LayoutUnit fraction = b.Fraction();
      return RoundToInt(fraction + a) - RoundToInt(fraction);
    };
    return SizeI(snap(size.Width(), location.X()), snap(size.Height(), location.Y()));
  }

  KRYS_NODISCARD inline PointF RoundPointToDevicePixels(const LayoutPoint &point, float pixelSnappingFactor,
                                                        bool directionalRoundingToRight = true,
                                                        bool directionalRoundingToBottom = true) noexcept
  {
    return PointF(RoundToDevicePixel(point.X(), pixelSnappingFactor, !directionalRoundingToRight),
                  RoundToDevicePixel(point.Y(), pixelSnappingFactor, !directionalRoundingToBottom));
  }

  KRYS_NODISCARD inline PointF FloorPointToDevicePixels(const LayoutPoint &point,
                                                        float pixelSnappingFactor) noexcept
  {
    return PointF(FloorToDevicePixel(point.X(), pixelSnappingFactor),
                  FloorToDevicePixel(point.Y(), pixelSnappingFactor));
  }

  KRYS_NODISCARD inline PointF CeilPointToDevicePixels(const LayoutPoint &point,
                                                       float pixelSnappingFactor) noexcept
  {
    return PointF(CeilToDevicePixel(point.X(), pixelSnappingFactor),
                  CeilToDevicePixel(point.Y(), pixelSnappingFactor));
  }

  KRYS_NODISCARD inline SizeF SnapSizeToDevicePixel(const LayoutSize &size, const LayoutPoint &location,
                                                    float pixelSnappingFactor) noexcept
  {
    auto snap = [&](LayoutUnit a, LayoutUnit b)
    {
      LayoutUnit fraction = b.Fraction();
      return RoundToDevicePixel(fraction + a, pixelSnappingFactor)
             - RoundToDevicePixel(fraction, pixelSnappingFactor);
    };
    return SizeF(snap(size.Width(), location.X()), snap(size.Height(), location.Y()));
  }

  struct LayoutPointLimits
  {
    LayoutPoint Min;
    LayoutPoint Max;

    KRYS_NODISCARD constexpr LayoutPoint Clamp(const LayoutPoint &point) const noexcept
    {
      return point.ConstrainedBetween(Min, Max);
    }

    KRYS_NODISCARD constexpr bool Fits(const LayoutPoint &point) const noexcept
    {
      return point == Clamp(point);
    }

    KRYS_NODISCARD constexpr LayoutSize Distance(const LayoutPoint &point) const noexcept
    {
      return point - Clamp(point);
    }
  };

  TextStream &operator<<(TextStream &ts, const LayoutPoint &value) noexcept;
}
