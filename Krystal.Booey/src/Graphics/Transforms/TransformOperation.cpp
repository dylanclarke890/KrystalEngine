/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Transforms/TransformOperation.hpp"

#include "Krystal.Booey/Graphics/Transforms/IdentityTransformOperation.hpp"

namespace krys::boo
{

  Maybe<TransformOperation::Type> TransformOperation::SharedPrimitiveType(Type other) const noexcept
  {
    // https://drafts.csswg.org/css-transforms-2/#interpolation-of-transform-functions
    // "If both transform functions share a primitive in the two-dimensional space, both transform
    // functions get converted to the two-dimensional primitive. If one or both transform functions
    // are three-dimensional transform functions, the common three-dimensional primitive is used."
    auto type = PrimitiveType();
    if (type == other)
    {
      return type;
    }
    static constexpr std::array sharedPrimitives {std::array {Type::Rotate, Type::Rotate3D},
                                                  std::array {Type::Scale, Type::Scale3D},
                                                  std::array {Type::Translate, Type::Translate3D}};
    for (auto typePair : sharedPrimitives)
    {
      if ((type == typePair[0] || type == typePair[1]) && (other == typePair[0] || other == typePair[1]))
      {
        return typePair[1];
      }
    }
    return null;
  }

  Maybe<TransformOperation::Type>
    TransformOperation::SharedPrimitiveType(const TransformOperation *other) const noexcept
  {
    // Blending with a null operation is always supported via blending with identity.
    if (!other)
    {
      return GetType();
    }

    // In case we have the same type, make sure to preserve it.
    if (other->GetType() == GetType())
    {
      return GetType();
    }

    return SharedPrimitiveType(other->PrimitiveType());
  }
}
