#pragma once

#include "Krystal.Booey/Core/RectEdges.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include <algorithm>

namespace krys::boo
{
  class FloatQuad;
  class FloatRoundedRect;

  /// @brief The four corner radii of a `LayoutRoundedRect`.
  class LayoutRoundedRectRadii
  {
  private:
    LayoutSize _topLeft;
    LayoutSize _topRight;
    LayoutSize _bottomLeft;
    LayoutSize _bottomRight;

  public:
    LayoutRoundedRectRadii() noexcept = default;
    LayoutRoundedRectRadii(const LayoutSize &topLeft, const LayoutSize &topRight,
                           const LayoutSize &bottomLeft, const LayoutSize &bottomRight) noexcept
        : _topLeft(topLeft), _topRight(topRight), _bottomLeft(bottomLeft), _bottomRight(bottomRight)
    {
    }

    void SetTopLeft(const LayoutSize &size) noexcept
    {
      _topLeft = size;
    }
    void SetTopRight(const LayoutSize &size) noexcept
    {
      _topRight = size;
    }
    void SetBottomLeft(const LayoutSize &size) noexcept
    {
      _bottomLeft = size;
    }
    void SetBottomRight(const LayoutSize &size) noexcept
    {
      _bottomRight = size;
    }
    KRYS_NODISCARD const LayoutSize &TopLeft() const noexcept
    {
      return _topLeft;
    }
    KRYS_NODISCARD const LayoutSize &TopRight() const noexcept
    {
      return _topRight;
    }
    KRYS_NODISCARD const LayoutSize &BottomLeft() const noexcept
    {
      return _bottomLeft;
    }
    KRYS_NODISCARD const LayoutSize &BottomRight() const noexcept
    {
      return _bottomRight;
    }

    void SetRadiiForEdges(const LayoutRoundedRectRadii &, RectEdges<bool> includeEdges) noexcept;

    KRYS_NODISCARD bool IsZero() const noexcept;

    KRYS_NODISCARD bool AreRenderableInRect(const LayoutRect &) const noexcept;
    void MakeRenderableInRect(const LayoutRect &) noexcept;

    void Scale(float factor) noexcept;
    void Expand(LayoutUnit topWidth, LayoutUnit bottomWidth, LayoutUnit leftWidth,
                LayoutUnit rightWidth) noexcept;
    void Expand(LayoutUnit size) noexcept
    {
      Expand(size, size, size, size);
    }
    void Shrink(LayoutUnit topWidth, LayoutUnit bottomWidth, LayoutUnit leftWidth,
                LayoutUnit rightWidth) noexcept
    {
      Expand(-topWidth, -bottomWidth, -leftWidth, -rightWidth);
    }
    void Shrink(LayoutUnit size) noexcept
    {
      Shrink(size, size, size, size);
    }

    KRYS_NODISCARD LayoutRoundedRectRadii TransposedRadii() const noexcept
    {
      return {_topLeft.TransposedSize(), _topRight.TransposedSize(), _bottomLeft.TransposedSize(),
              _bottomRight.TransposedSize()};
    }

    KRYS_NODISCARD LayoutUnit MinimumRadius() const noexcept
    {
      return std::min({_topLeft.MinDimension(), _topRight.MinDimension(), _bottomLeft.MinDimension(),
                       _bottomRight.MinDimension()});
    }
    KRYS_NODISCARD LayoutUnit MaximumRadius() const noexcept
    {
      return std::max({_topLeft.MinDimension(), _topRight.MinDimension(), _bottomLeft.MinDimension(),
                       _bottomRight.MinDimension()});
    }

    KRYS_NODISCARD bool operator==(const LayoutRoundedRectRadii &) const noexcept = default;
  };

  /// @brief A layout rectangle with rounded corners.
  class LayoutRoundedRect
  {
  public:
    using Radii = LayoutRoundedRectRadii;

  private:
    LayoutRect _rect;
    Radii _radii;

  public:
    explicit LayoutRoundedRect(const LayoutRect &, const Radii & = Radii()) noexcept;
    LayoutRoundedRect(LayoutUnit, LayoutUnit, LayoutUnit width, LayoutUnit height) noexcept;
    LayoutRoundedRect(const LayoutRect &, const LayoutSize &topLeft, const LayoutSize &topRight,
                      const LayoutSize &bottomLeft, const LayoutSize &bottomRight) noexcept;

    KRYS_NODISCARD const LayoutRect &Rect() const noexcept
    {
      return _rect;
    }
    KRYS_NODISCARD const Radii &GetRadii() const noexcept
    {
      return _radii;
    }
    KRYS_NODISCARD bool IsRounded() const noexcept
    {
      return !_radii.IsZero();
    }
    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _rect.IsEmpty();
    }

    void SetRect(const LayoutRect &rect) noexcept
    {
      _rect = rect;
    }
    void SetRadii(const Radii &radii) noexcept
    {
      _radii = radii;
    }
    void SetRadiiForEdges(const Radii &radii, RectEdges<bool> includeEdges) noexcept
    {
      _radii.SetRadiiForEdges(radii, includeEdges);
    }

    void Move(const LayoutSize &size) noexcept
    {
      _rect.Move(size);
    }
    void MoveBy(const LayoutPoint &offset) noexcept
    {
      _rect.MoveBy(offset);
    }
    void Inflate(LayoutUnit size) noexcept
    {
      _rect.Inflate(size);
    }
    void InflateWithRadii(LayoutUnit size) noexcept;
    void ExpandRadii(LayoutUnit size) noexcept
    {
      _radii.Expand(size);
    }
    void ShrinkRadii(LayoutUnit size) noexcept
    {
      _radii.Shrink(size);
    }

    KRYS_NODISCARD bool IsRenderable() const noexcept;
    void AdjustRadii() noexcept;

    // Tests whether the quad intersects any part of this rounded rectangle.
    // This only works for convex quads.
    KRYS_NODISCARD bool IntersectsQuad(const FloatQuad &) const noexcept;
    KRYS_NODISCARD bool Contains(const LayoutRect &) const noexcept;

    KRYS_NODISCARD FloatRoundedRect
      PixelSnappedRoundedRectForPainting(float deviceScaleFactor) const noexcept;

    KRYS_NODISCARD LayoutRoundedRect TransposedRect() const noexcept
    {
      return LayoutRoundedRect(_rect.TransposedRect(), _radii.TransposedRadii());
    }

    KRYS_NODISCARD bool operator==(const LayoutRoundedRect &) const noexcept = default;
  };
}
