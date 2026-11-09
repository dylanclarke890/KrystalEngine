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

#include "Krystal.Booey/Style/Values/Transforms/Functions/TranslateTransformFunction.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"

#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

#include "Krystal.Booey/Graphics/Transforms/TranslateTransformOperation.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  TranslateTransformFunction::TranslateTransformFunction(const LengthPercentage &x, const LengthPercentage &y,
                                                         const Length &z,
                                                         TransformFunctionBase::Type type) noexcept
      : TransformFunctionBase(type), _x(x), _y(y), _z(z)
  {
    krys_assert(IsTranslateTransformFunctionType(type));
  }

  Ref<const TranslateTransformFunction>
    TranslateTransformFunction::Create(const LengthPercentage &x, const LengthPercentage &y,
                                       TransformFunctionBase::Type type) noexcept
  {
    return AdoptRef(*new TranslateTransformFunction(x, y, 0_css_px, type));
  }

  Ref<const TranslateTransformFunction>
    TranslateTransformFunction::Create(const LengthPercentage &x, const LengthPercentage &y, const Length &z,
                                       TransformFunctionBase::Type type) noexcept
  {
    return AdoptRef(*new TranslateTransformFunction(x, y, z, type));
  }

  Ref<const TransformFunctionBase> TranslateTransformFunction::Clone() const noexcept
  {
    return AdoptRef(*new TranslateTransformFunction(_x, _y, _z, GetType()));
  }

  Ref<TransformOperation>
    TranslateTransformFunction::ToPlatform(const FloatSize &borderBoxSize) const noexcept
  {
    return TranslateTransformOperation::Create(Evaluate<float>(_x, borderBoxSize.Width(), ZoomNeeded {}),
                                               Evaluate<float>(_y, borderBoxSize.Height(), ZoomNeeded {}),
                                               Evaluate<float>(_z, ZoomNeeded {}),
                                               style::ConvertToPlatform(GetType()));
  }

  TransformFunctionSizeDependencies TranslateTransformFunction::ComputeSizeDependencies() const noexcept
  {
    return {
      .IsWidthDependent = _x.IsPercent(),
      .IsHeightDependent = _y.IsPercent(),
    };
  }

  bool TranslateTransformFunction::Equals(const TransformFunctionBase &other) const noexcept
  {
    if (!IsSameType(other))
    {
      return false;
    }

    auto &otherTranslate = Downcast<TranslateTransformFunction>(other);
    return _x == otherTranslate._x && _y == otherTranslate._y && _z == otherTranslate._z;
  }

  void TranslateTransformFunction::Apply(TransformationMatrix &transform,
                                         const FloatSize &borderBoxSize) const noexcept
  {
    transform.Translate3d(Evaluate<float>(_x, borderBoxSize.Width(), ZoomNeeded {}),
                          Evaluate<float>(_y, borderBoxSize.Height(), ZoomNeeded {}),
                          Evaluate<float>(_z, ZoomNeeded {}));
  }

  Ref<const TransformFunctionBase> TranslateTransformFunction::Blend(const TransformFunctionBase *from,
                                                                     const BlendingContext &context,
                                                                     bool blendToIdentity) const noexcept
  {
    if (blendToIdentity)
    {
      return TranslateTransformFunction::Create(style::Blend(_x, LengthPercentage {0_css_px}, context),
                                                style::Blend(_y, LengthPercentage {0_css_px}, context),
                                                style::Blend(_z, Length {0_css_px}, context), GetType());
    }

    auto outputType = SharedPrimitiveType(from);
    if (!outputType)
    {
      return ShareRef(*this);
    }

    const auto *fromOp = Downcast<TranslateTransformFunction>(from);
    auto fromX = fromOp ? fromOp->_x : LengthPercentage {0_css_px};
    auto fromY = fromOp ? fromOp->_y : LengthPercentage {0_css_px};
    auto fromZ = fromOp ? fromOp->_z : Length {0_css_px};

    return TranslateTransformFunction::Create(style::Blend(fromX, X(), context),
                                              style::Blend(fromY, Y(), context),
                                              style::Blend(fromZ, Z(), context), *outputType);
  }

}
