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

#include "Krystal.Booey/Rendering/Shapes/RectangleLayoutShape.hpp"
#include <cmath>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD float EllipseXIntercept(float y, float rx, float ry) noexcept
    {
      krys_debug_assert(ry > 0);
      return rx * std::sqrt(1 - (y * y) / (ry * ry));
    }
  }

  FloatRect RectangleLayoutShape::ShapeMarginBounds() const noexcept
  {
    krys_debug_assert(ShapeMargin() >= 0);
    if (!ShapeMargin())
    {
      return _bounds;
    }

    float boundsX = X() - ShapeMargin();
    float boundsY = Y() - ShapeMargin();
    float boundsWidth = Width() + ShapeMargin() * 2;
    float boundsHeight = Height() + ShapeMargin() * 2;
    return FloatRect(boundsX, boundsY, boundsWidth, boundsHeight);
  }

  LineSegment RectangleLayoutShape::GetExcludedInterval(LayoutUnit logicalTop,
                                                        LayoutUnit logicalHeight) const noexcept
  {
    auto bounds = ShapeMarginBounds();
    if (bounds.IsEmpty())
    {
      return {};
    }

    float y1 = logicalTop.ToFloat();
    float y2 = (logicalTop + logicalHeight).ToFloat();

    if (y2 < bounds.Y() || y1 >= bounds.MaxY())
    {
      return {};
    }

    float x1 = bounds.X();
    float x2 = bounds.MaxX();

    float marginRadiusX = Rx() + ShapeMargin();
    float marginRadiusY = Ry() + ShapeMargin();

    if (marginRadiusY > 0)
    {
      if (y2 < bounds.Y() + marginRadiusY)
      {
        float yi = y2 - bounds.Y() - marginRadiusY;
        float xi = EllipseXIntercept(yi, marginRadiusX, marginRadiusY);
        x1 = bounds.X() + marginRadiusX - xi;
        x2 = bounds.MaxX() - marginRadiusX + xi;
      }
      else if (y1 > bounds.MaxY() - marginRadiusY)
      {
        float yi = y1 - (bounds.MaxY() - marginRadiusY);
        float xi = EllipseXIntercept(yi, marginRadiusX, marginRadiusY);
        x1 = bounds.X() + marginRadiusX - xi;
        x2 = bounds.MaxX() - marginRadiusX + xi;
      }
    }

    if (ShouldFlipStartAndEndPoints(GetWritingMode()))
    {
      return {std::max(0.f, _boxLogicalWidth - x2), std::max(0.f, _boxLogicalWidth - x1)};
    }
    return {x1, x2};
  }
}
