/*
 Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies)

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/Graphics/Layers/GraphicsLayerTransform.hpp"

namespace krys::boo
{
  GraphicsLayerTransform::GraphicsLayerTransform() noexcept
      : _anchorPoint(0.5, 0.5, 0), _flattening(true),
        _dirty(false), // false by default since all default values would be combined as the identity matrix
        _childrenDirty(false)
  {
  }

  void GraphicsLayerTransform::SetPosition(const FloatPoint &position) noexcept
  {
    if (_position == position)
    {
      return;
    }
    _position = position;
    _dirty = true;
  }

  void GraphicsLayerTransform::SetSize(const FloatSize &size) noexcept
  {
    if (_size == size)
    {
      return;
    }
    _size = size;
    _dirty = true;
  }

  void GraphicsLayerTransform::SetAnchorPoint(const FloatPoint3D &anchorPoint) noexcept
  {
    if (_anchorPoint == anchorPoint)
    {
      return;
    }
    _anchorPoint = anchorPoint;
    _dirty = true;
  }

  void GraphicsLayerTransform::SetFlattening(bool flattening) noexcept
  {
    if (_flattening == flattening)
    {
      return;
    }
    _flattening = flattening;
    _dirty = true;
  }

  void GraphicsLayerTransform::SetLocalTransform(const TransformationMatrix &transform) noexcept
  {
    if (_local == transform)
    {
      return;
    }
    _local = transform;
    _dirty = true;
  }

  void GraphicsLayerTransform::SetChildrenTransform(const TransformationMatrix &transform) noexcept
  {
    if (_children == transform)
    {
      return;
    }
    _children = transform;
    _dirty = true;
  }

  const TransformationMatrix &GraphicsLayerTransform::Combined() const noexcept
  {
    krys_debug_assert(!_dirty);
    return _combined;
  }

  const TransformationMatrix &GraphicsLayerTransform::CombinedForChildren() const noexcept
  {
    krys_debug_assert(!_dirty);
    if (_childrenDirty)
    {
      CombineTransformsForChildren();
    }
    return _combinedForChildren;
  }

  void GraphicsLayerTransform::CombineTransforms(const TransformationMatrix &parentTransform) noexcept
  {
    float originX = _anchorPoint.X() * _size.Width();
    float originY = _anchorPoint.Y() * _size.Height();
    _combined = parentTransform;
    _combined.Translate3d(originX + _position.X(), originY + _position.Y(), _anchorPoint.Z())
      .Multiply(_local);

    // The children transform will take it from here, if it gets used.
    _combinedForChildren = _combined;
    _combined.Translate3d(-originX, -originY, -_anchorPoint.Z());

    _dirty = false;
    _childrenDirty = true;
  }

  void GraphicsLayerTransform::CombineTransformsForChildren() const noexcept
  {
    krys_debug_assert(!_dirty);
    krys_debug_assert(_childrenDirty);

    float originX = _anchorPoint.X() * _size.Width();
    float originY = _anchorPoint.Y() * _size.Height();

    // In case a parent had preserves3D and this layer has not, flatten our children.
    if (_flattening)
    {
      _combinedForChildren = _combinedForChildren.To2dTransform();
    }
    _combinedForChildren.Multiply(_children);
    _combinedForChildren.Translate3d(-originX, -originY, -_anchorPoint.Z());

    _childrenDirty = false;
  }
}
