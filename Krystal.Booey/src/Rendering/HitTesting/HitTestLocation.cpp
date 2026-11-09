/*
 * Copyright (C) 2006, 2008, 2011, 2020 Apple Inc. All rights reserved.
 * Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "Krystal.Booey/Rendering/HitTesting/HitTestLocation.hpp"

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
