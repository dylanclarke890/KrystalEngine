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

#include "Krystal.Booey/Style/Values/Transforms/Functions/ScaleTransformFunction.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Transforms/ScaleTransformOperation.hpp"

#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"

namespace krys::boo::style
{
  ScaleTransformFunction::ScaleTransformFunction(NumberOrPercentageResolvedToNumber<> x,
                                                 NumberOrPercentageResolvedToNumber<> y,
                                                 NumberOrPercentageResolvedToNumber<> z,
                                                 TransformFunctionBase::Type type) noexcept
      : TransformFunctionBase(type), _x(x), _y(y), _z(z)
  {
    krys_assert(IsScaleTransformFunctionType(type));
  }

  Ref<const ScaleTransformFunction> ScaleTransformFunction::Create(NumberOrPercentageResolvedToNumber<> x,
                                                                   NumberOrPercentageResolvedToNumber<> y,
                                                                   TransformFunctionBase::Type type) noexcept
  {
    return AdoptRef(*new ScaleTransformFunction(x, y, NumberOrPercentageResolvedToNumber<> {1}, type));
  }

  Ref<const ScaleTransformFunction> ScaleTransformFunction::Create(NumberOrPercentageResolvedToNumber<> x,
                                                                   NumberOrPercentageResolvedToNumber<> y,
                                                                   NumberOrPercentageResolvedToNumber<> z,
                                                                   TransformFunctionBase::Type type) noexcept
  {
    return AdoptRef(*new ScaleTransformFunction(x, y, z, type));
  }

  Ref<const TransformFunctionBase> ScaleTransformFunction::Clone() const noexcept
  {
    return AdoptRef(*new ScaleTransformFunction(_x, _y, _z, GetType()));
  }

  Ref<TransformOperation> ScaleTransformFunction::ToPlatform(const FloatSize &) const noexcept
  {
    return ScaleTransformOperation::Create(_x.Value.Value, _y.Value.Value, _z.Value.Value,
                                           style::ConvertToPlatform(GetType()));
  }

  bool ScaleTransformFunction::Equals(const TransformFunctionBase &other) const noexcept
  {
    if (!IsSameType(other))
    {
      return false;
    }

    auto &otherScale = Downcast<ScaleTransformFunction>(other);
    return _x == otherScale._x && _y == otherScale._y && _z == otherScale._z;
  }

  void ScaleTransformFunction::Apply(TransformationMatrix &transform, const FloatSize &) const noexcept
  {
    transform.Scale3d(_x.Value.Value, _y.Value.Value, _z.Value.Value);
  }

  KRYS_NODISCARD static NumberOrPercentageResolvedToNumber<>
    BlendScaleComponent(NumberOrPercentageResolvedToNumber<> from, NumberOrPercentageResolvedToNumber<> to,
                        const BlendingContext &context) noexcept
  {
    switch (context.CompositeOperation)
    {
      case CompositeOperation::Replace: return style::Blend(from, to, context);
      case CompositeOperation::Add:
        krys_debug_assert(context.Progress == 1.0);
        return NumberOrPercentageResolvedToNumber<> {from.Value.Value * to.Value.Value};
      case CompositeOperation::Accumulate:
        krys_debug_assert(context.Progress == 1.0);
        return NumberOrPercentageResolvedToNumber<> {from.Value.Value + to.Value.Value - 1};
    }
    krys_unreachable();
  }

  Ref<const TransformFunctionBase> ScaleTransformFunction::Blend(const TransformFunctionBase *from,
                                                                 const BlendingContext &context,
                                                                 bool blendToIdentity) const noexcept
  {
    if (blendToIdentity)
    {
      return ScaleTransformFunction::Create(BlendScaleComponent(_x, 1.0, context),
                                            BlendScaleComponent(_y, 1.0, context),
                                            BlendScaleComponent(_z, 1.0, context), GetType());
    }

    auto outputType = SharedPrimitiveType(from);
    if (!outputType)
    {
      return ShareRef(*this);
    }

    auto *fromOp = Downcast<ScaleTransformFunction>(from);
    auto fromX = fromOp ? fromOp->_x : NumberOrPercentageResolvedToNumber<> {1.0};
    auto fromY = fromOp ? fromOp->_y : NumberOrPercentageResolvedToNumber<> {1.0};
    auto fromZ = fromOp ? fromOp->_z : NumberOrPercentageResolvedToNumber<> {1.0};
    return ScaleTransformFunction::Create(BlendScaleComponent(fromX, _x, context),
                                          BlendScaleComponent(fromY, _y, context),
                                          BlendScaleComponent(fromZ, _z, context), *outputType);
  }

}
