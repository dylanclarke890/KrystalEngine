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

#include "Krystal.Booey/Graphics/Animation/CompositeOperation.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatSize.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"

#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  struct BlendingContext;

  enum class TransformOperationType : uint8
  {
    ScaleX,
    ScaleY,
    Scale,
    TranslateX,
    TranslateY,
    Translate,
    RotateX,
    RotateY,
    Rotate,
    SkewX,
    SkewY,
    Skew,
    Matrix,
    ScaleZ,
    Scale3D,
    TranslateZ,
    Translate3D,
    RotateZ,
    Rotate3D,
    Matrix3D,
    Perspective,
    Identity,
    None
  };

  class TransformOperation : public RefCounted<TransformOperation>
  {
  public:
    using Type = TransformOperationType;

    explicit TransformOperation(Type type) noexcept : _type(type)
    {
    }
    virtual ~TransformOperation() noexcept = default;

    KRYS_NODISCARD virtual Ref<TransformOperation> Clone() const noexcept = 0;

    KRYS_NODISCARD bool operator==(const TransformOperation &other) const noexcept
    {
      return Equals(other);
    }
    KRYS_NODISCARD virtual bool Equals(const TransformOperation &) const noexcept = 0;

    virtual void Apply(TransformationMatrix &) const noexcept = 0;
    virtual void ApplyUnrounded(TransformationMatrix &transform) const noexcept
    {
      Apply(transform);
    }

    KRYS_NODISCARD virtual Ref<TransformOperation> Blend(const TransformOperation *from,
                                                         const BlendingContext &,
                                                         bool blendToIdentity = false) const noexcept = 0;

    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }
    KRYS_NODISCARD bool IsSameType(const TransformOperation &other) const noexcept
    {
      return GetType() == other.GetType();
    }

    KRYS_NODISCARD virtual Type PrimitiveType() const noexcept
    {
      return _type;
    }
    KRYS_NODISCARD Maybe<Type> SharedPrimitiveType(Type other) const noexcept;
    KRYS_NODISCARD Maybe<Type> SharedPrimitiveType(const TransformOperation *other) const noexcept;

    KRYS_NODISCARD static bool IsRotateTransformOperationType(Type type) noexcept
    {
      return type == Type::RotateX || type == Type::RotateY || type == Type::RotateZ || type == Type::Rotate
             || type == Type::Rotate3D;
    }

    KRYS_NODISCARD static bool IsScaleTransformOperationType(Type type) noexcept
    {
      return type == Type::ScaleX || type == Type::ScaleY || type == Type::ScaleZ || type == Type::Scale
             || type == Type::Scale3D;
    }

    KRYS_NODISCARD static bool IsSkewTransformOperationType(Type type) noexcept
    {
      return type == Type::SkewX || type == Type::SkewY || type == Type::Skew;
    }

    KRYS_NODISCARD static bool IsTranslateTransformOperationType(Type type) noexcept
    {
      return type == Type::TranslateX || type == Type::TranslateY || type == Type::TranslateZ
             || type == Type::Translate || type == Type::Translate3D;
    }

  private:
    Type _type;
  };
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_TRANSFORM_OPERATION(ToValueTypeName, predicate)                          \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(ToValueTypeName)                                                    \
    KRYS_NODISCARD static bool IsType(const krys::boo::TransformOperation &operation) noexcept               \
    {                                                                                                        \
      return predicate(operation.GetType());                                                                 \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
