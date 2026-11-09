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

#include "Krystal.Booey/Graphics/Geometry/FloatSize.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformOperation.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

#include "Krystal.Core/RefCounted.hpp"

namespace krys::boo
{
  struct BlendingContext;

  namespace style
  {
    enum class TransformFunctionType : uint8
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
    };

    struct TransformFunctionSizeDependencies
    {
      bool IsWidthDependent = false;
      bool IsHeightDependent = false;
    };

    class TransformFunctionBase : public RefCounted<TransformFunctionBase>
    {
    public:
      using Type = TransformFunctionType;

      explicit TransformFunctionBase(Type type) noexcept : _type(type)
      {
      }

      virtual ~TransformFunctionBase() noexcept = default;

      KRYS_NODISCARD virtual Ref<const TransformFunctionBase> Clone() const noexcept = 0;

      KRYS_NODISCARD virtual Ref<TransformOperation>
        ToPlatform(const FloatSize &borderBoxSize) const noexcept = 0;

      KRYS_NODISCARD bool operator==(const TransformFunctionBase &other) const noexcept
      {
        return Equals(other);
      }
      KRYS_NODISCARD virtual bool Equals(const TransformFunctionBase &) const noexcept = 0;

      virtual void Apply(TransformationMatrix &, const FloatSize &borderBoxSize) const noexcept = 0;

      KRYS_NODISCARD virtual Ref<const TransformFunctionBase>
        Blend(const TransformFunctionBase *from, const BlendingContext &,
              bool blendToIdentity = false) const noexcept = 0;

      KRYS_NODISCARD Type GetType() const noexcept
      {
        return _type;
      }
      KRYS_NODISCARD bool IsSameType(const TransformFunctionBase &other) const noexcept
      {
        return GetType() == other.GetType();
      }

      KRYS_NODISCARD virtual Type PrimitiveType() const noexcept
      {
        return _type;
      }
      KRYS_NODISCARD Maybe<Type> SharedPrimitiveType(Type other) const noexcept;
      KRYS_NODISCARD Maybe<Type> SharedPrimitiveType(const TransformFunctionBase *other) const noexcept;

      KRYS_NODISCARD virtual bool IsIdentity() const noexcept = 0;
      KRYS_NODISCARD virtual bool IsAffectedByTransformOrigin() const noexcept
      {
        return false;
      }
      KRYS_NODISCARD virtual bool IsRepresentableIn2D() const noexcept
      {
        return true;
      }

      KRYS_NODISCARD virtual TransformFunctionSizeDependencies ComputeSizeDependencies() const noexcept
      {
        return {};
      }

      KRYS_NODISCARD bool Is3DOperation() const noexcept
      {
        Type opType = GetType();
        return opType == Type::ScaleZ || opType == Type::Scale3D || opType == Type::TranslateZ
               || opType == Type::Translate3D || opType == Type::RotateX || opType == Type::RotateY
               || opType == Type::Rotate3D || opType == Type::Matrix3D || opType == Type::Perspective;
      }

      KRYS_NODISCARD static bool IsRotateTransformFunctionType(Type type) noexcept
      {
        return type == Type::RotateX || type == Type::RotateY || type == Type::RotateZ || type == Type::Rotate
               || type == Type::Rotate3D;
      }

      KRYS_NODISCARD static bool IsScaleTransformFunctionType(Type type) noexcept
      {
        return type == Type::ScaleX || type == Type::ScaleY || type == Type::ScaleZ || type == Type::Scale
               || type == Type::Scale3D;
      }

      KRYS_NODISCARD static bool IsSkewTransformFunctionType(Type type) noexcept
      {
        return type == Type::SkewX || type == Type::SkewY || type == Type::Skew;
      }

      KRYS_NODISCARD static bool IsTranslateTransformFunctionType(Type type) noexcept
      {
        return type == Type::TranslateX || type == Type::TranslateY || type == Type::TranslateZ
               || type == Type::Translate || type == Type::Translate3D;
      }

    private:
      Type _type;
    };

    // MARK: - Platform

    template <>
    struct ToPlatform<TransformFunctionType>
    {
      KRYS_NODISCARD TransformOperationType operator()(TransformFunctionType) noexcept;
    };

  }
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_TRANSFORM_FUNCTION(ToValueTypeName, predicate)                     \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(ToValueTypeName)                                                    \
    KRYS_NODISCARD static bool IsType(const krys::boo::style::TransformFunctionBase &operation) noexcept     \
    {                                                                                                        \
      return predicate(operation.GetType());                                                                 \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
