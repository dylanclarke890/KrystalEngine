/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003, 2005-2008, 2017 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Graham Dennis (graham.dennis@gmail.com)
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

#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/TransformFunctionBase.hpp"

namespace krys::boo::style
{
  // skew() = skew( [ <angle> | <zero> ] , [ <angle> | <zero> ]? )
  // https://drafts.csswg.org/css-transforms/#funcdef-transform-skew
  // skewX() = skewX( [ <angle> | <zero> ] )
  // https://drafts.csswg.org/css-transforms/#funcdef-transform-skew
  // skewY() = skewY( [ <angle> | <zero> ] )
  // https://drafts.csswg.org/css-transforms/#funcdef-transform-skewy

  class SkewTransformFunction final : public TransformFunctionBase
  {
  public:
    KRYS_NODISCARD static Ref<const SkewTransformFunction> Create(Angle<>, Angle<>,
                                                                  TransformFunctionBase::Type) noexcept;

    KRYS_NODISCARD Ref<const TransformFunctionBase> Clone() const noexcept override;
    KRYS_NODISCARD Ref<TransformOperation> ToPlatform(const FloatSize &) const noexcept override;

    KRYS_NODISCARD Angle<> AngleX() const noexcept
    {
      return _angleX;
    }
    KRYS_NODISCARD Angle<> AngleY() const noexcept
    {
      return _angleY;
    }

    KRYS_NODISCARD bool IsIdentity() const noexcept override
    {
      return _angleX.IsZero() && _angleY.IsZero();
    }
    KRYS_NODISCARD bool IsAffectedByTransformOrigin() const noexcept override
    {
      return !IsIdentity();
    }

    KRYS_NODISCARD bool Equals(const TransformFunctionBase &) const noexcept override;

    void Apply(TransformationMatrix &, const FloatSize &borderBoxSize) const noexcept override;

    KRYS_NODISCARD Ref<const TransformFunctionBase>
      Blend(const TransformFunctionBase *from, const BlendingContext &,
            bool blendToIdentity = false) const noexcept override;

  private:
    SkewTransformFunction(Angle<>, Angle<>, TransformFunctionBase::Type) noexcept;

    Angle<> _angleX;
    Angle<> _angleY;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_TRANSFORM_FUNCTION(
  krys::boo::style::SkewTransformFunction,
  krys::boo::style::TransformFunctionBase::IsSkewTransformFunctionType)
