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
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class DOMMatrixReadOnly;

  /// @see https://drafts.css-houdini.org/css-typed-om/#dictdef-cssmatrixcomponentoptions
  struct CSSMatrixComponentOptions
  {
    Maybe<bool> Is2D;
  };

  /// @brief A `matrix()` or `matrix3d()`.
  /// @see https://drafts.css-houdini.org/css-typed-om/#cssmatrixcomponent
  class CSSMatrixComponent final : public CSSTransformComponent
  {
  private:
    Ref<DOMMatrix> _matrix;

  public:
    /// @brief A matrix component of a copy of @p matrix, 2D if @p options say so, or if they don't and the
    /// matrix is.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssmatrixcomponent-cssmatrixcomponent
    KRYS_NODISCARD static Ref<CSSMatrixComponent> Create(const DOMMatrixReadOnly &matrix,
                                                         CSSMatrixComponentOptions options = {}) noexcept;
    /// @brief Reifies a `matrix()` or `matrix3d()` function.
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSMatrixComponent>> Create(const FunctionValue &function,
                                                                           dom::Document &document) noexcept;

    ~CSSMatrixComponent() noexcept override;

    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssmatrixcomponent-matrix
    KRYS_NODISCARD DOMMatrix &Matrix() const noexcept
    {
      return *_matrix;
    }
    void SetMatrix(Ref<DOMMatrix> matrix) noexcept;

    void Serialise(StringBuilder &builder) const noexcept final;
    KRYS_NODISCARD dom::ExceptionOr<Ref<DOMMatrix>> ToMatrix() const noexcept final;

    KRYS_NODISCARD CSSTransformType TransformType() const noexcept final
    {
      return CSSTransformType::MatrixComponent;
    }
    KRYS_NODISCARD RefPtr<Value> ToCSSValue() const noexcept final;

  private:
    CSSMatrixComponent(const DOMMatrixReadOnly &matrix, bool is2D) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSMatrixComponent)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSTransformComponent &component) noexcept
  {
    return component.TransformType() == krys::boo::css::CSSTransformType::MatrixComponent;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
