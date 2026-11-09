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

#include "Krystal.Booey/Graphics/Transforms/TransformOperation.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  struct BlendingContext;

  class SkewTransformOperation final : public TransformOperation
  {
  public:
    KRYS_NODISCARD static Ref<SkewTransformOperation> Create(double, double,
                                                             TransformOperation::Type) noexcept;

    KRYS_NODISCARD Ref<TransformOperation> Clone() const noexcept override
    {
      return AdoptRef(*new SkewTransformOperation(_angleX, _angleY, GetType()));
    }

    KRYS_NODISCARD double AngleX() const noexcept
    {
      return _angleX;
    }
    KRYS_NODISCARD double AngleY() const noexcept
    {
      return _angleY;
    }

  private:
    KRYS_NODISCARD bool Equals(const TransformOperation &) const noexcept override;

    void Apply(TransformationMatrix &transform) const noexcept override
    {
      transform.Skew(_angleX, _angleY);
    }

    KRYS_NODISCARD Ref<TransformOperation> Blend(const TransformOperation *from, const BlendingContext &,
                                                 bool blendToIdentity = false) const noexcept override;

    SkewTransformOperation(double, double, TransformOperation::Type) noexcept;

    double _angleX;
    double _angleY;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_TRANSFORM_OPERATION(krys::boo::SkewTransformOperation,
                                                krys::boo::TransformOperation::IsSkewTransformOperationType)
