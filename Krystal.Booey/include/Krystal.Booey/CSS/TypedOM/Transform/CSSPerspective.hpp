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

#include "Krystal.Booey/CSS/TypedOM/CSSKeywordValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/Transform/CSSTransformComponent.hpp"

namespace krys::boo::css
{
  /// @brief A perspective's length: a length, or `none` (a string or keyword value).
  /// @see https://drafts.css-houdini.org/css-typed-om/#typedefdef-cssperspectivevalue
  using CSSPerspectiveValue = Variant<RefPtr<CSSNumericValue>, CSSOMString, RefPtr<CSSKeywordValue>>;

  /// @brief A `perspective()`.
  /// @see https://drafts.css-houdini.org/css-typed-om/#cssperspective
  class CSSPerspective final : public CSSTransformComponent
  {
  private:
    /// @brief A numeric value, or a keyword value of `none`, never a string.
    CSSPerspectiveValue _length;

  public:
    /// @return A `TypeError` if @p length isn't a length or `none`.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssperspective-cssperspective
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSPerspective>> Create(CSSPerspectiveValue length) noexcept;
    /// @brief Reifies a `perspective()` function.
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSPerspective>> Create(const FunctionValue &function,
                                                                       dom::Document &document) noexcept;

    ~CSSPerspective() noexcept override;

    /// @brief The length: a numeric value, or a keyword value of `none`.
    KRYS_NODISCARD const CSSPerspectiveValue &Length() const noexcept
    {
      return _length;
    }
    /// @return A `TypeError` if @p length isn't a length or `none`.
    dom::ExceptionOr<void> SetLength(CSSPerspectiveValue length) noexcept;

    /// @brief Does nothing: a perspective is always 3D.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssperspective-is2d
    void SetIs2D(bool) noexcept final
    {
    }

    void Serialise(StringBuilder &builder) const noexcept final;
    KRYS_NODISCARD dom::ExceptionOr<Ref<DOMMatrix>> ToMatrix() const noexcept final;

    KRYS_NODISCARD CSSTransformType TransformType() const noexcept final
    {
      return CSSTransformType::Perspective;
    }
    KRYS_NODISCARD RefPtr<Value> ToCSSValue() const noexcept final;

  private:
    explicit CSSPerspective(CSSPerspectiveValue length) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSPerspective)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSTransformComponent &component) noexcept
  {
    return component.TransformType() == krys::boo::css::CSSTransformType::Perspective;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
