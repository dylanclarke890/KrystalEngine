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
  // matrix() = matrix( <number>#{6} )
  // https://drafts.csswg.org/css-transforms/#funcdef-transform-matrix

  class MatrixTransformFunction final : public TransformFunctionBase
  {
  public:
    KRYS_NODISCARD static Ref<const MatrixTransformFunction> CreateIdentity() noexcept;
    KRYS_NODISCARD static Ref<const MatrixTransformFunction> Create(Number<>, Number<>, Number<>, Number<>,
                                                                    Number<>, Number<>) noexcept;
    KRYS_NODISCARD static Ref<const MatrixTransformFunction> Create(const TransformationMatrix &) noexcept;

    KRYS_NODISCARD Ref<const TransformFunctionBase> Clone() const noexcept override;
    KRYS_NODISCARD Ref<TransformOperation> ToPlatform(const FloatSize &) const noexcept override;

    KRYS_NODISCARD TransformationMatrix Matrix() const noexcept
    {
      return TransformationMatrix(_a.Value, _b.Value, _c.Value, _d.Value, _e.Value, _f.Value);
    }

    KRYS_NODISCARD bool IsIdentity() const noexcept override
    {
      return _a == 1 && _b == 0 && _c == 0 && _d == 1 && _e == 0 && _f == 0;
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
    MatrixTransformFunction(Number<>, Number<>, Number<>, Number<>, Number<>, Number<>) noexcept;
    MatrixTransformFunction(const TransformationMatrix &) noexcept;

    Number<> _a;
    Number<> _b;
    Number<> _c;
    Number<> _d;
    Number<> _e;
    Number<> _f;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_TRANSFORM_FUNCTION(krys::boo::style::MatrixTransformFunction,
                                                     krys::boo::style::TransformFunctionBase::Type::Matrix ==)
