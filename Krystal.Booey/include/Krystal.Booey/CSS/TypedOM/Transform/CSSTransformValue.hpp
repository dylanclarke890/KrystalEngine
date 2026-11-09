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

#include "Krystal.Booey/CSS/TypedOM/CSSStyleValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/Transform/CSSTransformComponent.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::css
{
  class DOMMatrix;
  class TransformListValue;

  /// @brief A `<transform-list>`, as a list of transform components.
  /// @see https://drafts.css-houdini.org/css-typed-om/#csstransformvalue
  class CSSTransformValue final : public CSSStyleValue
  {
  private:
    List<Ref<CSSTransformComponent>> _components;

  public:
    /// @brief Reifies a transform list.
    /// @see https://drafts.css-houdini.org/css-typed-om/#reify-a-transform-list
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSTransformValue>> Create(const TransformListValue &list,
                                                                          dom::Document &document) noexcept;
    /// @return A `TypeError` if @p transforms is empty.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-csstransformvalue-csstransformvalue
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSTransformValue>>
      Create(List<Ref<CSSTransformComponent>> transforms) noexcept;

    ~CSSTransformValue() noexcept override;

    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-csstransformvalue-length
    KRYS_NODISCARD size_t Length() const noexcept
    {
      return _components.size();
    }
    KRYS_NODISCARD bool IsSupportedPropertyIndex(size_t index) const noexcept
    {
      return index < _components.size();
    }
    /// @return Null if @p index is out of range.
    KRYS_NODISCARD RefPtr<CSSTransformComponent> Item(size_t index) const noexcept;
    /// @brief Replaces the component at @p index, or appends one at the end.
    /// @return A `RangeError` if @p index is past the end.
    dom::ExceptionOr<Ref<CSSTransformComponent>> SetItem(size_t index,
                                                         Ref<CSSTransformComponent> component) noexcept;
    KRYS_NODISCARD const List<Ref<CSSTransformComponent>> &Components() const noexcept
    {
      return _components;
    }

    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-csstransformvalue-is2d
    KRYS_NODISCARD bool Is2D() const noexcept;

    /// @brief The components' matrices, multiplied in order.
    /// @return A `TypeError` if a component has no matrix.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-csstransformvalue-tomatrix
    KRYS_NODISCARD dom::ExceptionOr<Ref<DOMMatrix>> ToMatrix() const noexcept;

    KRYS_NODISCARD CSSStyleValueType StyleValueType() const noexcept final
    {
      return CSSStyleValueType::CSSTransformValue;
    }
    KRYS_NODISCARD RefPtr<Value> ToCSSValue() const noexcept final;

    void Serialise(StringBuilder &builder, EnumSet<SerialisationArguments> arguments) const noexcept final;

  private:
    explicit CSSTransformValue(List<Ref<CSSTransformComponent>> transforms) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSTransformValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleValue &styleValue) noexcept
  {
    return styleValue.StyleValueType() == krys::boo::css::CSSStyleValueType::CSSTransformValue;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
