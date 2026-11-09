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
  /// @brief A `translate()`, `translate3d()` or single-axis translation.
  /// @see https://drafts.css-houdini.org/css-typed-om/#csstranslate
  class CSSTranslate final : public CSSTransformComponent
  {
  private:
    Ref<CSSNumericValue> _x;
    Ref<CSSNumericValue> _y;
    Ref<CSSNumericValue> _z;

  public:
    /// @brief A 2D translation without @p z, or a 3D one with it.
    /// @return A `TypeError` if @p x or @p y isn't a length or percentage, or @p z isn't a length.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-csstranslate-csstranslate
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSTranslate>>
      Create(Ref<CSSNumericValue> x, Ref<CSSNumericValue> y, RefPtr<CSSNumericValue> z = {}) noexcept;
    /// @brief Reifies a translation function.
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSTranslate>> Create(const FunctionValue &function,
                                                                     dom::Document &document) noexcept;

    KRYS_NODISCARD const CSSNumericValue &X() const noexcept
    {
      return *_x;
    }
    KRYS_NODISCARD const CSSNumericValue &Y() const noexcept
    {
      return *_y;
    }
    KRYS_NODISCARD const CSSNumericValue &Z() const noexcept
    {
      return *_z;
    }
    /// @return A `TypeError` if @p x isn't a length or percentage.
    dom::ExceptionOr<void> SetX(Ref<CSSNumericValue> x) noexcept;
    /// @return A `TypeError` if @p y isn't a length or percentage.
    dom::ExceptionOr<void> SetY(Ref<CSSNumericValue> y) noexcept;
    /// @return A `TypeError` if @p z isn't a length.
    dom::ExceptionOr<void> SetZ(Ref<CSSNumericValue> z) noexcept;

    void Serialise(StringBuilder &builder) const noexcept final;
    KRYS_NODISCARD dom::ExceptionOr<Ref<DOMMatrix>> ToMatrix() const noexcept final;

    KRYS_NODISCARD CSSTransformType TransformType() const noexcept final
    {
      return CSSTransformType::Translate;
    }
    KRYS_NODISCARD RefPtr<Value> ToCSSValue() const noexcept final;

  private:
    CSSTranslate(bool is2D, Ref<CSSNumericValue> x, Ref<CSSNumericValue> y, Ref<CSSNumericValue> z) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSTranslate)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSTransformComponent &component) noexcept
  {
    return component.TransformType() == krys::boo::css::CSSTransformType::Translate;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
