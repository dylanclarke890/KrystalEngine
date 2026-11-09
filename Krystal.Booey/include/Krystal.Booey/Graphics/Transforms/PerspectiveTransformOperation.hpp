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

#pragma once

#include "Krystal.Booey/Graphics/Transforms/TransformOperation.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <optional>

namespace krys::boo
{
  struct BlendingContext;

  class PerspectiveTransformOperation final : public TransformOperation
  {
  public:
    KRYS_NODISCARD static Ref<PerspectiveTransformOperation> Create(const Maybe<float> &) noexcept;

    KRYS_NODISCARD Ref<TransformOperation> Clone() const noexcept override
    {
      return AdoptRef(*new PerspectiveTransformOperation(_p));
    }

    KRYS_NODISCARD Maybe<float> Perspective() const noexcept
    {
      return _p;
    }

  private:
    KRYS_NODISCARD bool IsIdentity() const noexcept
    {
      return !_p;
    }

    KRYS_NODISCARD bool Equals(const TransformOperation &) const noexcept override;

    KRYS_NODISCARD Maybe<float> FloatValue() const noexcept
    {
      if (!_p)
      {
        return {};
      }

      // From https://www.w3.org/TR/css-transforms-2/#perspective-property:
      // "As very small <length> values can produce bizarre rendering results and stress the numerical
      // accuracy of transform calculations, values less than 1px must be treated as 1px for rendering
      // purposes. (This clamping does not affect the underlying value, so perspective: 0; in a stylesheet
      // will still serialize back as 0.)"
      return std::max(1.0f, *_p);
    }

    void Apply(TransformationMatrix &transform) const noexcept override
    {
      if (auto value = FloatValue())
      {
        transform.ApplyPerspective(*value);
      }
    }

    KRYS_NODISCARD Ref<TransformOperation> Blend(const TransformOperation *from, const BlendingContext &,
                                                 bool blendToIdentity = false) const noexcept override;

    PerspectiveTransformOperation(const Maybe<float> &) noexcept;

    Maybe<float> _p;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_TRANSFORM_OPERATION(krys::boo::PerspectiveTransformOperation,
                                                krys::boo::TransformOperation::Type::Perspective ==)
