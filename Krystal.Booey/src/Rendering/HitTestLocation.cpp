#include "Krystal.Booey/Rendering/HitTestLocation.hpp"

namespace krys::boo
{
  HitTestLocation::HitTestLocation(const LayoutPoint &point) noexcept
      : _point(point), _boundingBox(LayoutRect {point, LayoutSize {1, 1}}), _transformedPoint(point),
        _transformedRect(FloatRect(_boundingBox))
  {
  }

  HitTestLocation::HitTestLocation(const FloatPoint &point, const FloatQuad &quad) noexcept
      : _point {FlooredLayoutPoint(point)}, _boundingBox {quad.EnclosingBoundingBox()},
        _transformedPoint {point}, _transformedRect {quad}, _isRectBased {true},
        _isRectilinear {quad.IsRectilinear()}
  {
  }

  HitTestLocation::HitTestLocation(const LayoutRect &rect) noexcept
      : _point {rect.Center()}, // Rounded to an integer point; it may not be the exact center.
        _boundingBox {rect}, _transformedPoint {rect.Center()},
        _transformedRect {FloatQuad {FloatRect(_boundingBox)}}, _isRectBased {true}
  {
  }

  HitTestLocation::HitTestLocation(const HitTestLocation &other, const LayoutSize &offset) noexcept
      : _point(other._point), _boundingBox(other._boundingBox), _transformedPoint(other._transformedPoint),
        _transformedRect(other._transformedRect), _isRectBased(other._isRectBased),
        _isRectilinear(other._isRectilinear)
  {
    Move(offset);
  }

  void HitTestLocation::Move(const LayoutSize &offset) noexcept
  {
    _point.Move(offset);
    _transformedPoint.Move(offset);
    _transformedRect.Move(offset);
    _boundingBox = LayoutRect {_transformedRect.BoundingBox()};
  }

  template <typename TRect>
  bool HitTestLocation::IntersectsRect(const TRect &rect) const noexcept
  {
    // FIXME: When the hit test is not rect based we should use rect.contains(m_point).
    // That does change some corner case tests though.

    // First check if rect even intersects our bounding box.
    if (!rect.Intersects(TRect(_boundingBox)))
    {
      return false;
    }

    // If the transformed rect is rectilinear the bounding box intersection was accurate.
    if (_isRectilinear)
    {
      return true;
    }

    // If rect fully contains our bounding box, we are also sure of an intersection.
    if (rect.Contains(TRect(_boundingBox)))
    {
      return true;
    }

    // Otherwise we need to do a slower quad based intersection test.
    return _transformedRect.IntersectsRect(FloatRect(rect));
  }

  bool HitTestLocation::Intersects(const LayoutRect &rect) const noexcept
  {
    return IntersectsRect(rect);
  }

  bool HitTestLocation::Intersects(const FloatRect &rect) const noexcept
  {
    return IntersectsRect(rect);
  }

  bool HitTestLocation::Intersects(const LayoutRoundedRect &rect) const noexcept
  {
    return rect.IntersectsQuad(_transformedRect);
  }
}
