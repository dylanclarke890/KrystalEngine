#pragma once

#include "Krystal.Booey/Rendering/ClipRect.hpp"
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
