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

  class TranslateTransformOperation final : public TransformOperation
  {
  public:
    KRYS_NODISCARD static Ref<TranslateTransformOperation> Create(float tx, float ty,
                                                                  TransformOperation::Type type) noexcept
    {
      return AdoptRef(*new TranslateTransformOperation(tx, ty, 0, type));
    }

    KRYS_NODISCARD static Ref<TranslateTransformOperation> Create(float, float, float,
                                                                  TransformOperation::Type) noexcept;

    KRYS_NODISCARD Ref<TransformOperation> Clone() const noexcept override
    {
      return AdoptRef(*new TranslateTransformOperation(_x, _y, _z, GetType()));
    }

    KRYS_NODISCARD float X() const noexcept
    {
      return _x;
    }
    KRYS_NODISCARD float Y() const noexcept
    {
      return _y;
    }
    KRYS_NODISCARD float Z() const noexcept
    {
      return _z;
    }

    KRYS_NODISCARD TransformOperation::Type PrimitiveType() const noexcept final
    {
      return !_z ? Type::Translate : Type::Translate3D;
    }

    void Apply(TransformationMatrix &transform) const noexcept final
    {
      transform.Translate3d(_x, _y, _z);
    }

    KRYS_NODISCARD bool Equals(const TransformOperation &) const noexcept final;

    KRYS_NODISCARD Ref<TransformOperation> Blend(const TransformOperation *from, const BlendingContext &,
                                                 bool blendToIdentity = false) const noexcept final;

  private:
    TranslateTransformOperation(float, float, float, TransformOperation::Type) noexcept;

    float _x;
    float _y;
    float _z;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_TRANSFORM_OPERATION(
  krys::boo::TranslateTransformOperation, krys::boo::TransformOperation::IsTranslateTransformOperationType)
