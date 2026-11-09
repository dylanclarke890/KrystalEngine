/*
 * Copyright (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/Style/Values/Transforms/Functions/SkewTransformFunction.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Transforms/SkewTransformOperation.hpp"

#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"

namespace krys::boo::style
{
  SkewTransformFunction::SkewTransformFunction(Angle<> angleX, Angle<> angleY,
                                               TransformFunctionBase::Type type) noexcept
      : TransformFunctionBase(type), _angleX(angleX), _angleY(angleY)
  {
    krys_assert(IsSkewTransformFunctionType(type));
  }

  Ref<const SkewTransformFunction> SkewTransformFunction::Create(Angle<> angleX, Angle<> angleY,
                                                                 TransformFunctionBase::Type type) noexcept
  {
    return AdoptRef(*new SkewTransformFunction(angleX, angleY, type));
  }

  Ref<const TransformFunctionBase> SkewTransformFunction::Clone() const noexcept
  {
    return AdoptRef(*new SkewTransformFunction(_angleX, _angleY, GetType()));
  }

  Ref<TransformOperation> SkewTransformFunction::ToPlatform(const FloatSize &) const noexcept
  {
    return SkewTransformOperation::Create(_angleX.Value, _angleY.Value, style::ConvertToPlatform(GetType()));
  }

  bool SkewTransformFunction::Equals(const TransformFunctionBase &other) const noexcept
  {
    if (!IsSameType(other))
    {
      return false;
    }

    auto &otherSkew = Downcast<SkewTransformFunction>(other);
    return _angleX == otherSkew._angleX && _angleY == otherSkew._angleY;
  }

  void SkewTransformFunction::Apply(TransformationMatrix &transform, const FloatSize &) const noexcept
  {
    transform.Skew(_angleX.Value, _angleY.Value);
  }

  Ref<const TransformFunctionBase> SkewTransformFunction::Blend(const TransformFunctionBase *from,
                                                                const BlendingContext &context,
                                                                bool blendToIdentity) const noexcept
  {
    if (blendToIdentity)
    {
      return SkewTransformFunction::Create(style::Blend(_angleX, Angle<> {0.0}, context),
                                           style::Blend(_angleY, Angle<> {0.0}, context), GetType());
    }

    auto outputType = SharedPrimitiveType(from);
    if (!outputType)
    {
      return ShareRef(*this);
    }

    auto *fromOp = Downcast<SkewTransformFunction>(from);
    auto fromAngleX = fromOp ? fromOp->_angleX : Angle<> {0};
    auto fromAngleY = fromOp ? fromOp->_angleY : Angle<> {0};
    return SkewTransformFunction::Create(style::Blend(fromAngleX, _angleX, context),
                                         style::Blend(fromAngleY, _angleY, context), *outputType);
  }

}
