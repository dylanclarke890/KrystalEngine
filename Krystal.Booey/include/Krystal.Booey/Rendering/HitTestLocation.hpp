#pragma once

#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Graphics/LayoutRoundedRect.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  /// @brief Where a hit test looks: a point, or for rect-based tests an area (a quad, under transforms).
  class HitTestLocation
  {
  private:
    // These are the cached forms of the more accurate point and rect below.
    LayoutPoint _point;
    LayoutRect _boundingBox;

    FloatPoint _transformedPoint;
    FloatQuad _transformedRect;

    bool _isRectBased {false};
    bool _isRectilinear {true};

  public:
    HitTestLocation() noexcept = default;
    explicit HitTestLocation(const LayoutPoint &point) noexcept;
    HitTestLocation(const FloatPoint &point, const FloatQuad &quad) noexcept;
    explicit HitTestLocation(const LayoutRect &rect) noexcept;

    /// @brief A copy of @p other moved by @p offset, into another coordinate space.
    HitTestLocation(const HitTestLocation &other, const LayoutSize &offset) noexcept;
    HitTestLocation(const HitTestLocation &other) noexcept = default;
    HitTestLocation &operator=(const HitTestLocation &other) noexcept = default;

    KRYS_NODISCARD const LayoutPoint &Point() const noexcept
    {
      return _point;
    }
    KRYS_NODISCARD IntPoint RoundedPoint() const noexcept
    {
      return RoundedIntPoint(_point);
    }

    // Rect-based hit test related methods.
    KRYS_NODISCARD bool IsRectBasedTest() const noexcept
    {
      return _isRectBased;
    }
    KRYS_NODISCARD bool IsRectilinear() const noexcept
    {
      return _isRectilinear;
    }
    KRYS_NODISCARD LayoutRect BoundingBox() const noexcept
    {
      return _boundingBox;
    }

    KRYS_NODISCARD bool Intersects(const LayoutRect &rect) const noexcept;
    KRYS_NODISCARD bool Intersects(const FloatRect &rect) const noexcept;
    KRYS_NODISCARD bool Intersects(const LayoutRoundedRect &rect) const noexcept;

    KRYS_NODISCARD const FloatPoint &TransformedPoint() const noexcept
    {
      return _transformedPoint;
    }
    KRYS_NODISCARD const FloatQuad &TransformedRect() const noexcept
    {
      return _transformedRect;
    }

  private:
    template <typename TRect>
    KRYS_NODISCARD bool IntersectsRect(const TRect &rect) const noexcept;

    void Move(const LayoutSize &offset) noexcept;
  };
}
