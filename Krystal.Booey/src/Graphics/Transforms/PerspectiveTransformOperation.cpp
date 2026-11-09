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

#include "Krystal.Booey/Graphics/Transforms/PerspectiveTransformOperation.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include <cmath>

namespace krys::boo
{
  Ref<PerspectiveTransformOperation> PerspectiveTransformOperation::Create(const Maybe<float> &p) noexcept
  {
    return AdoptRef(*new PerspectiveTransformOperation(p));
  }

  PerspectiveTransformOperation::PerspectiveTransformOperation(const Maybe<float> &p) noexcept
      : TransformOperation(TransformOperation::Type::Perspective), _p(p)
  {
  }

  bool PerspectiveTransformOperation::Equals(const TransformOperation &other) const noexcept
  {
    if (!IsSameType(other))
    {
      return false;
    }
    return _p == Downcast<PerspectiveTransformOperation>(other)._p;
  }

  Ref<TransformOperation> PerspectiveTransformOperation::Blend(const TransformOperation *from,
                                                               const BlendingContext &context,
                                                               bool blendToIdentity) const noexcept
  {
    if (!SharedPrimitiveType(from))
    {
      return ShareRef(const_cast<PerspectiveTransformOperation &>(*this));
    }

    // https://drafts.csswg.org/css-transforms-2/#interpolation-of-transform-functions
    // says that we should run matrix decomposition and then run the rules for
    // interpolation of matrices, but we know what those rules are going to
    // yield, so just do that directly.
    auto getInverse = [](const auto &operation)
    {
      return !operation->IsIdentity() ? (1.0 / (*operation->FloatValue())) : 0.0;
    };

    double ourInverse = getInverse(this);
    double fromPInverse, toPInverse;
    if (blendToIdentity)
    {
      fromPInverse = ourInverse;
      toPInverse = 0.0;
    }
    else
    {
      fromPInverse = from ? getInverse(Downcast<PerspectiveTransformOperation>(from)) : 0.0;
      toPInverse = ourInverse;
    }

    double pInverse = boo::Blend(fromPInverse, toPInverse, context);
    Maybe<float> p;
    if (pInverse > 0.0 && std::isnormal(pInverse))
    {
      p = static_cast<float>(1.0 / pInverse);
    }
    return PerspectiveTransformOperation::Create(p);
  }

}
