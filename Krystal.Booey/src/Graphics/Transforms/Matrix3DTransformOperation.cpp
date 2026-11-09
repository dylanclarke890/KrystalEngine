/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Transforms/Matrix3DTransformOperation.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include <algorithm>

namespace krys::boo
{
  Ref<Matrix3DTransformOperation>
    Matrix3DTransformOperation::Create(const TransformationMatrix &matrix) noexcept
  {
    return AdoptRef(*new Matrix3DTransformOperation(matrix));
  }

  Matrix3DTransformOperation::Matrix3DTransformOperation(const TransformationMatrix &mat) noexcept
      : TransformOperation(TransformOperation::Type::Matrix3D), _matrix(mat)
  {
  }

  bool Matrix3DTransformOperation::Equals(const TransformOperation &other) const noexcept
  {
    return IsSameType(other) && _matrix == Downcast<Matrix3DTransformOperation>(other)._matrix;
  }

  KRYS_NODISCARD static Ref<TransformOperation> CreateOperation(TransformationMatrix &to,
                                                                TransformationMatrix &from,
                                                                const BlendingContext &context) noexcept
  {
    to.Blend(from, context.Progress, context.CompositeOperation);
    return Matrix3DTransformOperation::Create(to);
  }

  Ref<TransformOperation> Matrix3DTransformOperation::Blend(const TransformOperation *from,
                                                            const BlendingContext &context,
                                                            bool blendToIdentity) const noexcept
  {
    if (!SharedPrimitiveType(from))
    {
      return ShareRef(const_cast<Matrix3DTransformOperation &>(*this));
    }

    // Convert the TransformOperations into matrices
    TransformationMatrix fromT;
    TransformationMatrix toT;
    if (from)
    {
      from->Apply(fromT);
    }

    Apply(toT);

    if (blendToIdentity)
    {
      return CreateOperation(fromT, toT, context);
    }
    return CreateOperation(toT, fromT, context);
  }

}
