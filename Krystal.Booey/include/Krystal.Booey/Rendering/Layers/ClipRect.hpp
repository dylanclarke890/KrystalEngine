/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestLocation.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  /// @brief A clip rect, which may be infinite, and whether a border radius affects it.
  class ClipRect
  {
  private:
    LayoutRect _rect;
    bool _affectedByRadius {false};

  public:
    ClipRect() noexcept = default;

    ClipRect(const LayoutRect &rect) noexcept : _rect(rect)
    {
    }

    KRYS_NODISCARD const LayoutRect &Rect() const noexcept
    {
      return _rect;
    }

    void Reset() noexcept
    {
      _rect = LayoutRect::InfiniteRect();
    }

    KRYS_NODISCARD bool AffectedByRadius() const noexcept
    {
      return _affectedByRadius;
    }
    void SetAffectedByRadius(bool affectedByRadius) noexcept
    {
      _affectedByRadius = affectedByRadius;
    }

    KRYS_NODISCARD friend bool operator==(const ClipRect &, const ClipRect &) noexcept = default;

    void Intersect(const LayoutRect &other) noexcept
    {
      if (other.IsInfinite())
      {
        return;
      }
      if (IsInfinite())
      {
        _rect = other;
      }
      else
      {
        _rect.Intersect(other);
      }
    }

    void Intersect(const ClipRect &other) noexcept
    {
      Intersect(other.Rect());
      if (other.AffectedByRadius())
      {
        _affectedByRadius = true;
      }
    }

    void Move(LayoutUnit x, LayoutUnit y) noexcept
    {
      _rect.Move(x, y);
    }
    void Move(const LayoutSize &size) noexcept
    {
      _rect.Move(size);
    }
    void MoveBy(const LayoutPoint &point) noexcept
    {
      _rect.MoveBy(point);
    }

    KRYS_NODISCARD bool Intersects(const LayoutRect &rect) const noexcept
    {
      if (IsInfinite() || rect.IsInfinite())
      {
        return true;
      }
      return _rect.Intersects(rect);
    }
    KRYS_NODISCARD bool Intersects(const HitTestLocation &hitTestLocation) const noexcept
    {
      if (IsInfinite())
      {
        return true;
      }
      return hitTestLocation.Intersects(_rect);
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _rect.IsEmpty();
    }
    KRYS_NODISCARD bool IsInfinite() const noexcept
    {
      return _rect.IsInfinite();
    }

    void InflateX(LayoutUnit dx) noexcept
    {
      _rect.InflateX(dx);
    }
    void InflateY(LayoutUnit dy) noexcept
    {
      _rect.InflateY(dy);
    }
    void Inflate(LayoutUnit d) noexcept
    {
      InflateX(d);
      InflateY(d);
    }
  };

  KRYS_NODISCARD inline ClipRect Intersection(const ClipRect &a, const ClipRect &b) noexcept
  {
    ClipRect c = a;
    c.Intersect(b);
    return c;
  }
}
