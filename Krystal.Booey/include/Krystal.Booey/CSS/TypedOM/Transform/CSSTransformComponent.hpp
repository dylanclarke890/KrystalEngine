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

#include "Krystal.Booey/CSS/TypedOM/CSSNumericValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSStyleValue.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::css
{
  class DOMMatrix;
  class FunctionValue;

  /// @brief The kinds of transform components.
  enum class CSSTransformType : uint8
  {
    MatrixComponent,
    Perspective,
    Rotate,
    Scale,
    Skew,
    SkewX,
    SkewY,
    Translate
  };

  /// @brief A transform function in a transform value.
  /// @see https://drafts.css-houdini.org/css-typed-om/#csstransformcomponent
  class CSSTransformComponent : public RefCounted<CSSTransformComponent>
  {
    KRYS_NON_COPY_MOVABLE(CSSTransformComponent);

  private:
    bool _is2D;

  public:
    virtual ~CSSTransformComponent() noexcept;

    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-csstransformcomponent-tostring
    KRYS_NODISCARD CSSOMString ToString() const noexcept;
    virtual void Serialise(StringBuilder &builder) const noexcept = 0;

    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-csstransformcomponent-is2d
    KRYS_NODISCARD bool Is2D() const noexcept
    {
      return _is2D;
    }
    virtual void SetIs2D(bool is2D) noexcept
    {
      _is2D = is2D;
    }

    /// @return A `TypeError` if a length or angle in the component isn't one with an absolute unit.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-csstransformcomponent-tomatrix
    KRYS_NODISCARD virtual dom::ExceptionOr<Ref<DOMMatrix>> ToMatrix() const noexcept = 0;

    KRYS_NODISCARD virtual CSSTransformType TransformType() const noexcept = 0;
    /// @brief The transform function as a CSS value, or null if a numeric value in it has no CSS value.
    KRYS_NODISCARD virtual RefPtr<Value> ToCSSValue() const noexcept = 0;

  protected:
    explicit CSSTransformComponent(bool is2D) noexcept : _is2D(is2D)
    {
    }

    /// @brief Reifies the arguments of @p function, each of which must be a numeric value.
    KRYS_NODISCARD static dom::ExceptionOr<List<Ref<CSSNumericValue>>>
      ReifyArguments(const FunctionValue &function, dom::Document &document) noexcept;
  };
}
