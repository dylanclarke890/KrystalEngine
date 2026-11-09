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
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  LayoutRect::LayoutRect(const FloatRect &r) noexcept
      : _location(LayoutPoint(r.Location())), _size(LayoutSize(r.Size()))
  {
  }

  bool LayoutRect::Intersects(const LayoutRect &other) const noexcept
  {
    // Checking emptiness handles negative widths as well as zero.
    return !IsEmpty() && !other.IsEmpty() && X() < other.MaxX() && other.X() < MaxX() && Y() < other.MaxY()
           && other.Y() < MaxY();
  }

  bool LayoutRect::Contains(const LayoutRect &other) const noexcept
  {
    return X() <= other.X() && MaxX() >= other.MaxX() && Y() <= other.Y() && MaxY() >= other.MaxY();
  }

  void LayoutRect::Intersect(const LayoutRect &other) noexcept
  {
    LayoutPoint newLocation(std::max(X(), other.X()), std::max(Y(), other.Y()));
    LayoutPoint newMaxPoint(std::min(MaxX(), other.MaxX()), std::min(MaxY(), other.MaxY()));

    // Return a clean empty rectangle for non-intersecting cases.
    if (newLocation.X() >= newMaxPoint.X() || newLocation.Y() >= newMaxPoint.Y())
    {
      newLocation = LayoutPoint(0, 0);
      newMaxPoint = LayoutPoint(0, 0);
    }

    _location = newLocation;
    _size = newMaxPoint - newLocation;
  }

  bool LayoutRect::EdgeInclusiveIntersect(const LayoutRect &other) noexcept
  {
    LayoutPoint newLocation(std::max(X(), other.X()), std::max(Y(), other.Y()));
    LayoutPoint newMaxPoint(std::min(MaxX(), other.MaxX()), std::min(MaxY(), other.MaxY()));

    bool intersects = true;

    // Return a clean empty rectangle for non-intersecting cases.
    if (newLocation.X() > newMaxPoint.X() || newLocation.Y() > newMaxPoint.Y())
    {
      newLocation = {};
      newMaxPoint = {};
      intersects = false;
    }

    _location = newLocation;
    _size = newMaxPoint - newLocation;
    return intersects;
  }

  void LayoutRect::Unite(const LayoutRect &other) noexcept
  {
    // Handle empty special cases first.
    if (other.IsEmpty())
    {
      return;
    }
    if (IsEmpty())
    {
      *this = other;
      return;
    }

    UniteEvenIfEmpty(other);
  }

  void LayoutRect::UniteEvenIfEmpty(const LayoutRect &other) noexcept
  {
    auto minX = std::min(X(), other.X());
    auto minY = std::min(Y(), other.Y());
    auto maxX = std::max(MaxX(), other.MaxX());
    auto maxY = std::max(MaxY(), other.MaxY());

    ShiftEdgesTo(minX, minY, maxX, maxY);
  }

  bool LayoutRect::CheckedUnite(const LayoutRect &other) noexcept
  {
    if (other.IsEmpty())
    {
      return true;
    }
    if (IsEmpty())
    {
      *this = other;
      return true;
    }
    if (!IsMaxXMaxYRepresentable() || !other.IsMaxXMaxYRepresentable())
    {
      return false;
    }

    FloatPoint topLeft = FloatPoint(std::min<float>(X(), other.X()), std::min<float>(Y(), other.Y()));
    FloatPoint bottomRight =
      FloatPoint(std::max<float>(MaxX(), other.MaxX()), std::max<float>(MaxY(), other.MaxY()));
    FloatSize size = bottomRight - topLeft;

    if (size.Width() >= LayoutUnit::NearlyMax() || size.Height() >= LayoutUnit::NearlyMax())
    {
      return false;
    }

    _location = LayoutPoint(topLeft);
    _size = LayoutSize(size);
    return true;
  }

  void LayoutRect::UniteIfNonZero(const LayoutRect &other) noexcept
  {
    // Handle empty special cases first.
    if (!other.Width() && !other.Height())
    {
      return;
    }
    if (!Width() && !Height())
    {
      *this = other;
      return;
    }

    LayoutPoint newLocation(std::min(X(), other.X()), std::min(Y(), other.Y()));
    LayoutPoint newMaxPoint(std::max(MaxX(), other.MaxX()), std::max(MaxY(), other.MaxY()));

    _location = newLocation;
    _size = newMaxPoint - newLocation;
  }

  void LayoutRect::Scale(float scaleValue) noexcept
  {
    Scale(scaleValue, scaleValue);
  }

  void LayoutRect::Scale(float xScale, float yScale) noexcept
  {
    if (IsInfinite())
    {
      return;
    }

    _location.Scale(xScale, yScale);
    _size.Scale(xScale, yScale);
  }

  void LayoutRect::ExpandToInfiniteY() noexcept
  {
    LayoutRect infRect = LayoutRect::InfiniteRect();
    SetY(infRect.Y());
    SetHeight(infRect.Height());
  }

  void LayoutRect::ExpandToInfiniteX() noexcept
  {
    LayoutRect infRect = LayoutRect::InfiniteRect();
    SetX(infRect.X());
    SetWidth(infRect.Width());
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

  IntRect EnclosingIntRect(const LayoutRect &rect) noexcept
  {
    // Empty rects with fractional x, y values turn into non-empty rects when converting to enclosing.
    // We need to ensure that empty rects stay empty after the conversion, because the selection code expects
    // them to be empty.
    IntPoint location = FlooredIntPoint(rect.MinXMinYCorner());
    IntPoint maxPoint = IntPoint(rect.Width() ? rect.MaxX().Ceil() : location.X(),
                                 rect.Height() ? rect.MaxY().Ceil() : location.Y());
    return IntRect(location, maxPoint - location);
  }

  LayoutRect EnclosingLayoutRect(const FloatRect &rect) noexcept
  {
    LayoutPoint location = FlooredLayoutPoint(rect.MinXMinYCorner());
    LayoutPoint maxPoint = CeiledLayoutPoint(rect.MaxXMaxYCorner());
    return LayoutRect(location, maxPoint - location);
  }

  FloatRect EncloseRectToDevicePixels(const LayoutRect &rect, float pixelSnappingFactor) noexcept
  {
    FloatPoint location = FloorPointToDevicePixels(rect.MinXMinYCorner(), pixelSnappingFactor);
    FloatPoint maxPoint = CeilPointToDevicePixels(rect.MaxXMaxYCorner(), pixelSnappingFactor);
    return FloatRect(location, maxPoint - location);
  }

  TextStream &operator<<(TextStream &ts, const LayoutRect &r) noexcept
  {
    if (ts.HasFormattingFlag(TextStream::Formatting::LayoutUnitsAsIntegers))
    {
      return ts << SnappedIntRect(r);
    }

    return ts << FloatRect(r);
  }
}
