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

#pragma once

#include "Krystal.Booey/Graphics/Geometry/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatPoint3D.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatSize.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  /// @brief A graphics layer's transform combined with its parent's, its position and its anchor point.
  class GraphicsLayerTransform
  {
  private:
    FloatPoint3D _anchorPoint;
    FloatPoint _position;
    FloatSize _size;
    bool _flattening;
    bool _dirty;
    mutable bool _childrenDirty;
    TransformationMatrix _local;
    TransformationMatrix _children;
    TransformationMatrix _combined;
    mutable TransformationMatrix _combinedForChildren;

  public:
    GraphicsLayerTransform() noexcept;

    void SetPosition(const FloatPoint &position) noexcept;
    void SetSize(const FloatSize &size) noexcept;
    void SetAnchorPoint(const FloatPoint3D &anchorPoint) noexcept;
    void SetFlattening(bool flattening) noexcept;
    void SetLocalTransform(const TransformationMatrix &transform) noexcept;
    void SetChildrenTransform(const TransformationMatrix &transform) noexcept;

    KRYS_NODISCARD const TransformationMatrix &Combined() const noexcept;
    KRYS_NODISCARD const TransformationMatrix &CombinedForChildren() const noexcept;

    void CombineTransforms(const TransformationMatrix &parentTransform) noexcept;

  private:
    void CombineTransformsForChildren() const noexcept;
  };
}
