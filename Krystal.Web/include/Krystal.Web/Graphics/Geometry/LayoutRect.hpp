/*
 * Copyright (c) 2012 Google Inc. All rights reserved.
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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/Span.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Web/Graphics/Geometry/BoxExtents.hpp"
#include "Krystal.Web/Graphics/Geometry/LayoutGeometryForward.hpp"
#include "Krystal.Web/Graphics/Geometry/LayoutPoint.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::web
{
  /// @brief Whether @p rect's maximum corner is within `LayoutUnit`'s range, so not saturated.
  KRYS_NODISCARD bool IsMaxXMaxYRepresentable(const LayoutRect &rect) noexcept;

  /// @brief Unites @p other into @p rect, unless the result wouldn't be representable.
  /// @return Whether it did.
  KRYS_NODISCARD bool CheckedUnite(LayoutRect &rect, const LayoutRect &other) noexcept;

  /// @brief Stretches @p rect across the infinite rect horizontally.
  void ExpandToInfiniteX(LayoutRect &rect) noexcept;

  /// @brief Stretches @p rect across the infinite rect vertically.
  void ExpandToInfiniteY(LayoutRect &rect) noexcept;

  KRYS_NODISCARD LayoutRect UnionRect(Span<const LayoutRect> rects) noexcept;

  // Integral snapping functions.
  KRYS_NODISCARD constexpr RectI SnappedRectI(const LayoutRect &rect) noexcept
  {
    return RectI(RoundedPointI(rect.Location()), SnappedSizeI(rect.Size(), rect.Location()));
  }

  KRYS_NODISCARD constexpr RectI SnappedRectI(LayoutUnit left, LayoutUnit top, LayoutUnit width,
                                              LayoutUnit height) noexcept
  {
    return RectI(PointI(left.Round(), top.Round()),
                 SnappedSizeI(LayoutSize(width, height), LayoutPoint(left, top)));
  }

  KRYS_NODISCARD constexpr RectI SnappedRectI(LayoutPoint location, LayoutSize size) noexcept
  {
    return RectI(RoundedPointI(location), SnappedSizeI(size, location));
  }

  KRYS_NODISCARD RectI EnclosingRectI(const LayoutRect &rect) noexcept;

  KRYS_NODISCARD LayoutRect EnclosingLayoutRect(const RectF &rect) noexcept;

  // Device pixel snapping functions.
  KRYS_NODISCARD inline RectF SnapRectToDevicePixels(const LayoutRect &rect,
                                                     float pixelSnappingFactor) noexcept
  {
    return RectF(PointF(RoundToDevicePixel(rect.X(), pixelSnappingFactor),
                        RoundToDevicePixel(rect.Y(), pixelSnappingFactor)),
                 SnapSizeToDevicePixel(rect.Size(), rect.Location(), pixelSnappingFactor));
  }

  KRYS_NODISCARD inline RectF SnapRectToDevicePixels(LayoutUnit x, LayoutUnit y, LayoutUnit width,
                                                     LayoutUnit height, float pixelSnappingFactor) noexcept
  {
    return SnapRectToDevicePixels(LayoutRect(x, y, width, height), pixelSnappingFactor);
  }

  // FIXME: This needs to take vertical centering into account too.
  KRYS_NODISCARD inline RectF SnapRectToDevicePixelsWithWritingDirection(const LayoutRect &rect,
                                                                         float deviceScaleFactor,
                                                                         bool ltr) noexcept
  {
    if (!ltr)
    {
      PointF snappedTopRight = RoundPointToDevicePixels(rect.MaxXMinYCorner(), deviceScaleFactor, ltr);
      SizeF snappedSize = SnapSizeToDevicePixel(rect.Size(), rect.MaxXMinYCorner(), deviceScaleFactor);
      return RectF(snappedTopRight.X() - snappedSize.Width(), snappedTopRight.Y(), snappedSize.Width(),
                   snappedSize.Height());
    }
    return SnapRectToDevicePixels(rect, deviceScaleFactor);
  }

  KRYS_NODISCARD RectF EncloseRectToDevicePixels(const LayoutRect &rect, float pixelSnappingFactor) noexcept;

  TextStream &operator<<(TextStream &ts, const LayoutRect &value) noexcept;
}
