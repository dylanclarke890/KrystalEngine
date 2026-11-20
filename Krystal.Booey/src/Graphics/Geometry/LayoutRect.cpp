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

#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Core/Strings/TextStream.hpp"

namespace krys::boo
{
  bool IsMaxXMaxYRepresentable(const LayoutRect &rect) noexcept
  {
    RectF floatRect = rect;
    float maxX = floatRect.MaxX();
    float maxY = floatRect.MaxY();
    return maxX > LayoutUnit::NearlyMin() && maxX < LayoutUnit::NearlyMax() && maxY > LayoutUnit::NearlyMin()
           && maxY < LayoutUnit::NearlyMax();
  }

  bool CheckedUnite(LayoutRect &rect, const LayoutRect &other) noexcept
  {
    if (other.IsEmpty())
    {
      return true;
    }
    if (rect.IsEmpty())
    {
      rect = other;
      return true;
    }
    if (!IsMaxXMaxYRepresentable(rect) || !IsMaxXMaxYRepresentable(other))
    {
      return false;
    }

    PointF topLeft = PointF(std::min<float>(rect.X(), other.X()), std::min<float>(rect.Y(), other.Y()));
    PointF bottomRight =
      PointF(std::max<float>(rect.MaxX(), other.MaxX()), std::max<float>(rect.MaxY(), other.MaxY()));
    SizeF size = bottomRight - topLeft;

    if (size.Width() >= LayoutUnit::NearlyMax() || size.Height() >= LayoutUnit::NearlyMax())
    {
      return false;
    }

    rect = LayoutRect(LayoutPoint(topLeft), LayoutSize(size));
    return true;
  }

  void ExpandToInfiniteX(LayoutRect &rect) noexcept
  {
    LayoutRect infiniteRect = LayoutRect::InfiniteRect();
    rect.SetX(infiniteRect.X());
    rect.SetWidth(infiniteRect.Width());
  }

  void ExpandToInfiniteY(LayoutRect &rect) noexcept
  {
    LayoutRect infiniteRect = LayoutRect::InfiniteRect();
    rect.SetY(infiniteRect.Y());
    rect.SetHeight(infiniteRect.Height());
  }

  LayoutRect UnionRect(Span<const LayoutRect> rects) noexcept
  {
    if (rects.empty())
    {
      return {};
    }

    LayoutRect result = rects[0];
    for (size_t i = 1; i < rects.size(); ++i)
    {
      result.Unite(rects[i]);
    }

    return result;
  }

  RectI EnclosingRectI(const LayoutRect &rect) noexcept
  {
    // Empty rects with fractional x, y values turn into non-empty rects when converting to enclosing.
    // We need to ensure that empty rects stay empty after the conversion, because the selection code expects
    // them to be empty.
    PointI location = FlooredPointI(rect.MinXMinYCorner());
    PointI maxPoint = PointI(rect.Width() ? rect.MaxX().Ceil() : location.X(),
                             rect.Height() ? rect.MaxY().Ceil() : location.Y());
    return RectI(location, maxPoint - location);
  }

  LayoutRect EnclosingLayoutRect(const RectF &rect) noexcept
  {
    LayoutPoint location = FlooredLayoutPoint(rect.MinXMinYCorner());
    LayoutPoint maxPoint = CeiledLayoutPoint(rect.MaxXMaxYCorner());
    return LayoutRect(location, maxPoint - location);
  }

  RectF EncloseRectToDevicePixels(const LayoutRect &rect, float pixelSnappingFactor) noexcept
  {
    PointF location = FloorPointToDevicePixels(rect.MinXMinYCorner(), pixelSnappingFactor);
    PointF maxPoint = CeilPointToDevicePixels(rect.MaxXMaxYCorner(), pixelSnappingFactor);
    return RectF(location, maxPoint - location);
  }

  TextStream &operator<<(TextStream &ts, const LayoutRect &value) noexcept
  {
    if (ts.HasFormattingFlag(TextStream::Formatting::LayoutUnitsAsIntegers))
    {
      return ts << SnappedRectI(value);
    }

    return ts << RectF(value);
  }
}
