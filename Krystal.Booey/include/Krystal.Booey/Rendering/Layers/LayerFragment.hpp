/*
 * Copyright (C) 2015-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Layers/ClipRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  /// @brief A layer's bounds and clip rects in one fragment (column or page) of its pagination model, or the
  /// layer's whole bounds when it isn't paginated.
  class LayerFragment
  {
  public:
    class Rects
    {
    public:
      LayoutRect LayerBoundsRect;
      LayoutRect PaintDirtyRect;
      ClipRect BackgroundRect;
      ClipRect ForegroundRect;
      Maybe<LayoutRect> BoundingBoxRect;

      Rects() noexcept = default;

      Rects(const LayoutRect &layerBounds, const LayoutRect &paintDirtyRect, const ClipRect &backgroundRect,
            const ClipRect &foregroundRect, const Maybe<LayoutRect> &boundingBox = null) noexcept
          : LayerBoundsRect(layerBounds), PaintDirtyRect(paintDirtyRect), BackgroundRect(backgroundRect),
            ForegroundRect(foregroundRect), BoundingBoxRect(boundingBox)
      {
      }

      Rects(const Rects &other, const LayoutRect &boundingBox) noexcept
          : Rects(other.LayerBoundsRect, other.PaintDirtyRect, other.BackgroundRect, other.ForegroundRect,
                  boundingBox)
      {
      }

      KRYS_NODISCARD LayoutRect LayerBounds() const noexcept
      {
        return LayerBoundsRect;
      }

      KRYS_NODISCARD ClipRect GetBackgroundRect() const noexcept
      {
        return BackgroundRect;
      }
      KRYS_NODISCARD ClipRect DirtyBackgroundRect() const noexcept
      {
        return Intersection(PaintDirtyRect, BackgroundRect);
      }
      KRYS_NODISCARD ClipRect DirtyForegroundRect() const noexcept
      {
        return Intersection(PaintDirtyRect, ForegroundRect);
      }

      KRYS_NODISCARD Maybe<LayoutRect> BoundingBox() const noexcept
      {
        return BoundingBoxRect;
      }

      void MoveBy(const LayoutPoint &offset) noexcept
      {
        LayerBoundsRect.MoveBy(offset);
        PaintDirtyRect.MoveBy(offset);
        BackgroundRect.MoveBy(offset);
        ForegroundRect.MoveBy(offset);
        if (BoundingBoxRect)
        {
          BoundingBoxRect->MoveBy(offset);
        }
      }

      void Intersect(const LayoutRect &rect) noexcept
      {
        BackgroundRect.Intersect(rect);
        ForegroundRect.Intersect(rect);
        if (BoundingBoxRect)
        {
          BoundingBoxRect->Intersect(rect);
        }
      }

      void Intersect(const ClipRect &clipRect) noexcept
      {
        BackgroundRect.Intersect(clipRect);
        ForegroundRect.Intersect(clipRect);
      }
    };

    bool ShouldPaintContent {false};

    Rects FragmentRects;

    // Unique to paginated fragments. The physical translation to apply to shift the layer when
    // painting/hit-testing.
    LayoutSize PaginationOffset;

    // Also unique to paginated fragments. An additional clip that applies to the layer. It is in layer-local
    // (physical) coordinates.
    LayoutRect PaginationClip;

    LayerFragment() noexcept = default;

    explicit LayerFragment(Rects &&rects) noexcept : FragmentRects(krys::move(rects))
    {
    }

    KRYS_NODISCARD LayoutRect LayerBounds() const noexcept
    {
      return FragmentRects.LayerBounds();
    }

    KRYS_NODISCARD ClipRect BackgroundRect() const noexcept
    {
      return FragmentRects.GetBackgroundRect();
    }
    KRYS_NODISCARD ClipRect DirtyBackgroundRect() const noexcept
    {
      return FragmentRects.DirtyBackgroundRect();
    }
    KRYS_NODISCARD ClipRect DirtyForegroundRect() const noexcept
    {
      return FragmentRects.DirtyForegroundRect();
    }

    KRYS_NODISCARD Maybe<LayoutRect> BoundingBox() const noexcept
    {
      return FragmentRects.BoundingBox();
    }

    void MoveBy(const LayoutPoint &offset) noexcept
    {
      FragmentRects.MoveBy(offset);
      PaginationClip.MoveBy(offset);
    }

    void Intersect(const LayoutRect &rect) noexcept
    {
      FragmentRects.Intersect(rect);
    }
    void Intersect(const ClipRect &clipRect) noexcept
    {
      FragmentRects.Intersect(clipRect);
    }
  };

  using LayerFragments = List<LayerFragment>;
}
