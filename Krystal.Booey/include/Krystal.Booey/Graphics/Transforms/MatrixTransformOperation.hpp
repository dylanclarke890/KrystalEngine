/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003, 2005-2008, 2017 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Graham Dennis (graham.dennis@gmail.com)
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

#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformOperation.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  struct BlendingContext;

  class MatrixTransformOperation final : public TransformOperation
  {
  public:
    KRYS_NODISCARD static Ref<MatrixTransformOperation> CreateIdentity() noexcept
    {
      return AdoptRef(*new MatrixTransformOperation(1, 0, 0, 1, 0, 0));
    }

    KRYS_NODISCARD static Ref<MatrixTransformOperation> Create(double a, double b, double c, double d,
                                                               double e, double f) noexcept
    {
      return AdoptRef(*new MatrixTransformOperation(a, b, c, d, e, f));
    }

    KRYS_NODISCARD static Ref<MatrixTransformOperation> Create(const TransformationMatrix &) noexcept;

    KRYS_NODISCARD Ref<TransformOperation> Clone() const noexcept override
    {
      return AdoptRef(*new MatrixTransformOperation(Matrix()));
    }

    KRYS_NODISCARD TransformationMatrix Matrix() const noexcept
    {
      return TransformationMatrix(_a, _b, _c, _d, _e, _f);
    }

  private:
    KRYS_NODISCARD bool Equals(const TransformOperation &) const noexcept override;

    void Apply(TransformationMatrix &transform) const noexcept override
    {
      TransformationMatrix matrix(_a, _b, _c, _d, _e, _f);
      transform.Multiply(matrix);
    }

    KRYS_NODISCARD Ref<TransformOperation> Blend(const TransformOperation *from, const BlendingContext &,
                                                 bool blendToIdentity = false) const noexcept override;

    MatrixTransformOperation(double a, double b, double c, double d, double e, double f) noexcept
        : TransformOperation(TransformOperation::Type::Matrix), _a(a), _b(b), _c(c), _d(d), _e(e), _f(f)
    {
    }

    MatrixTransformOperation(const TransformationMatrix &) noexcept;

    double _a;
    double _b;
    double _c;
    double _d;
    double _e;
    double _f;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_TRANSFORM_OPERATION(krys::boo::MatrixTransformOperation,
                                                krys::boo::TransformOperation::Type::Matrix ==)
