/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Values/Transforms/Functions/Matrix3DTransformFunction.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Transforms/Matrix3DTransformOperation.hpp"

#include <algorithm>

namespace krys::boo::style
{
  Matrix3DTransformFunction::Matrix3DTransformFunction(const TransformationMatrix &matrix) noexcept
      : TransformFunctionBase(TransformFunctionBase::Type::Matrix3D), _matrix(matrix)
  {
  }

  Ref<const Matrix3DTransformFunction>
    Matrix3DTransformFunction::Create(const TransformationMatrix &matrix) noexcept
  {
    return AdoptRef(*new Matrix3DTransformFunction(matrix));
  }

  Ref<const TransformFunctionBase> Matrix3DTransformFunction::Clone() const noexcept
  {
    return AdoptRef(*new Matrix3DTransformFunction(_matrix));
  }

  Ref<TransformOperation> Matrix3DTransformFunction::ToPlatform(const FloatSize &) const noexcept
  {
    return Matrix3DTransformOperation::Create(_matrix);
  }

  bool Matrix3DTransformFunction::Equals(const TransformFunctionBase &other) const noexcept
  {
    if (!IsSameType(other))
    {
      return false;
    }

    auto &otherMatrix3D = Downcast<Matrix3DTransformFunction>(other);
    return _matrix == otherMatrix3D._matrix;
  }

  void Matrix3DTransformFunction::Apply(TransformationMatrix &transform, const FloatSize &) const noexcept
  {
    transform.Multiply(_matrix);
  }

  Ref<const TransformFunctionBase> Matrix3DTransformFunction::Blend(const TransformFunctionBase *from,
                                                                    const BlendingContext &context,
                                                                    bool blendToIdentity) const noexcept
  {
    auto createOperation =
      [](TransformationMatrix &to, TransformationMatrix &from, const BlendingContext &context)
    {
      to.Blend(from, context.Progress, context.CompositeOperation);
      return Matrix3DTransformFunction::Create(to);
    };

    if (!SharedPrimitiveType(from))
    {
      return ShareRef(*this);
    }

    // Convert the TransformFunctions into matrices
    TransformationMatrix toT = Matrix();
    TransformationMatrix fromT;
    if (from)
    {
      fromT = Downcast<Matrix3DTransformFunction>(*from).Matrix();
    }

    if (blendToIdentity)
    {
      return createOperation(fromT, toT, context);
    }
    return createOperation(toT, fromT, context);
  }

}
