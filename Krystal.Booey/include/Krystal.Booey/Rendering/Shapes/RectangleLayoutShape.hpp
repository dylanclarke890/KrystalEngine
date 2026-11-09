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

#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatSize.hpp"
#include "Krystal.Booey/Rendering/Shapes/LayoutShape.hpp"

namespace krys::boo
{
  /// @brief An ellipse inscribed in a rect: a `circle()` or an `ellipse()`.
  class RectangleLayoutShape final : public LayoutShape
  {
  private:
    FloatRect _bounds;
    FloatSize _radii;
    float _boxLogicalWidth {0.f};

  public:
    RectangleLayoutShape(const FloatRect &bounds, const FloatSize &radii, float boxLogicalWidth) noexcept
        : _bounds(bounds), _radii(radii), _boxLogicalWidth(boxLogicalWidth)
    {
    }

    KRYS_NODISCARD LayoutRect ShapeMarginLogicalBoundingBox() const noexcept override
    {
      return LayoutRect(ShapeMarginBounds());
    }
    KRYS_NODISCARD bool IsEmpty() const noexcept override
    {
      return _bounds.IsEmpty();
    }
    KRYS_NODISCARD LineSegment GetExcludedInterval(LayoutUnit logicalTop,
                                                   LayoutUnit logicalHeight) const noexcept override;

  private:
    KRYS_NODISCARD FloatRect ShapeMarginBounds() const noexcept;

    KRYS_NODISCARD float Rx() const noexcept
    {
      return _radii.Width();
    }
    KRYS_NODISCARD float Ry() const noexcept
    {
      return _radii.Height();
    }
    KRYS_NODISCARD float X() const noexcept
    {
      return _bounds.X();
    }
    KRYS_NODISCARD float Y() const noexcept
    {
      return _bounds.Y();
    }
    KRYS_NODISCARD float Width() const noexcept
    {
      return _bounds.Width();
    }
    KRYS_NODISCARD float Height() const noexcept
    {
      return _bounds.Height();
    }
  };
}
