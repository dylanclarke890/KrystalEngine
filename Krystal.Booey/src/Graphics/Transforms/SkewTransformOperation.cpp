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

#include "Krystal.Booey/Graphics/Transforms/SkewTransformOperation.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"

namespace krys::boo
{
  Ref<SkewTransformOperation> SkewTransformOperation::Create(double angleX, double angleY,
                                                             TransformOperation::Type type) noexcept
  {
    return AdoptRef(*new SkewTransformOperation(angleX, angleY, type));
  }

  SkewTransformOperation::SkewTransformOperation(double angleX, double angleY,
                                                 TransformOperation::Type type) noexcept
      : TransformOperation(type), _angleX(angleX), _angleY(angleY)
  {
    krys_assert(IsSkewTransformOperationType(type));
  }

  bool SkewTransformOperation::Equals(const TransformOperation &other) const noexcept
  {
    if (!IsSameType(other))
    {
      return false;
    }
    const SkewTransformOperation &s = Downcast<SkewTransformOperation>(other);
    return _angleX == s._angleX && _angleY == s._angleY;
  }

  Ref<TransformOperation> SkewTransformOperation::Blend(const TransformOperation *from,
                                                        const BlendingContext &context,
                                                        bool blendToIdentity) const noexcept
  {
    if (blendToIdentity)
    {
      return SkewTransformOperation::Create(boo::Blend(_angleX, 0.0, context),
                                            boo::Blend(_angleY, 0.0, context), GetType());
    }

    auto outputType = SharedPrimitiveType(from);
    if (!outputType)
    {
      return ShareRef(const_cast<SkewTransformOperation &>(*this));
    }

    const SkewTransformOperation *fromOp = Downcast<SkewTransformOperation>(from);
    double fromAngleX = fromOp ? fromOp->_angleX : 0;
    double fromAngleY = fromOp ? fromOp->_angleY : 0;
    return SkewTransformOperation::Create(boo::Blend(fromAngleX, _angleX, context),
                                          boo::Blend(fromAngleY, _angleY, context), *outputType);
  }

}
