/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/TypedOM/Transform/CSSTransformComponent.hpp"

namespace krys::boo::css
{
  /// @brief A `skewY()`.
  /// @see https://drafts.css-houdini.org/css-typed-om/#cssskewy
  class CSSSkewY final : public CSSTransformComponent
  {
  private:
    Ref<CSSNumericValue> _ay;

  public:
    /// @return A `TypeError` if @p ay isn't an angle.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssskewy-cssskewy
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSSkewY>> Create(Ref<CSSNumericValue> ay) noexcept;
    /// @brief Reifies a `skewY()` function.
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSSkewY>> Create(const FunctionValue &function,
                                                                 dom::Document &document) noexcept;

    KRYS_NODISCARD const CSSNumericValue &Ay() const noexcept
    {
      return *_ay;
    }
    /// @return A `TypeError` if @p ay isn't an angle.
    dom::ExceptionOr<void> SetAy(Ref<CSSNumericValue> ay) noexcept;

    /// @brief Does nothing: a skew is always 2D.
    void SetIs2D(bool) noexcept final
    {
    }

    void Serialise(StringBuilder &builder) const noexcept final;
    KRYS_NODISCARD dom::ExceptionOr<Ref<DOMMatrix>> ToMatrix() const noexcept final;

    KRYS_NODISCARD CSSTransformType TransformType() const noexcept final
    {
      return CSSTransformType::SkewY;
    }
    KRYS_NODISCARD RefPtr<Value> ToCSSValue() const noexcept final;

  private:
    explicit CSSSkewY(Ref<CSSNumericValue> ay) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSSkewY)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSTransformComponent &component) noexcept
  {
    return component.TransformType() == krys::boo::css::CSSTransformType::SkewY;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
