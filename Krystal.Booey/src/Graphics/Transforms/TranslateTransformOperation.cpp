/*
 * Copyright (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Transforms/TranslateTransformOperation.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"

namespace krys::boo
{
  Ref<TranslateTransformOperation> TranslateTransformOperation::Create(float tx, float ty, float tz,
                                                                       TransformOperation::Type type) noexcept
  {
    return AdoptRef(*new TranslateTransformOperation(tx, ty, tz, type));
  }

  TranslateTransformOperation::TranslateTransformOperation(float tx, float ty, float tz,
                                                           TransformOperation::Type type) noexcept
      : TransformOperation(type), _x(tx), _y(ty), _z(tz)
  {
    krys_assert(IsTranslateTransformOperationType(type));
  }

  bool TranslateTransformOperation::Equals(const TransformOperation &other) const noexcept
  {
    if (!IsSameType(other))
    {
      return false;
    }
    const TranslateTransformOperation &t = Downcast<TranslateTransformOperation>(other);
    return _x == t._x && _y == t._y && _z == t._z;
  }

  Ref<TransformOperation> TranslateTransformOperation::Blend(const TransformOperation *from,
                                                             const BlendingContext &context,
                                                             bool blendToIdentity) const noexcept
  {
    if (blendToIdentity)
    {
      return TranslateTransformOperation::Create(boo::Blend(_x, 0.0f, context), boo::Blend(_y, 0.0f, context),
                                                 boo::Blend(_z, 0.0f, context), GetType());
    }

    auto outputType = SharedPrimitiveType(from);
    if (!outputType)
    {
      return ShareRef(const_cast<TranslateTransformOperation &>(*this));
    }

    const TranslateTransformOperation *fromOp = Downcast<TranslateTransformOperation>(from);
    auto fromX = fromOp ? fromOp->_x : 0.0f;
    auto fromY = fromOp ? fromOp->_y : 0.0f;
    auto fromZ = fromOp ? fromOp->_z : 0.0f;
    return TranslateTransformOperation::Create(boo::Blend(fromX, X(), context),
                                               boo::Blend(fromY, Y(), context),
                                               boo::Blend(fromZ, Z(), context), *outputType);
  }

}
