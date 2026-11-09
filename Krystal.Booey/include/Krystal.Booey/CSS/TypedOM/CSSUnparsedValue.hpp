/*
 * Copyright (C) 2018 Apple Inc. All rights reserved.
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
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::css
{
  class CSSVariableReferenceValue;
  class TokenRange;

  /// @brief A segment of an unparsed value: a string, or a variable reference.
  /// @see https://drafts.css-houdini.org/css-typed-om/#typedefdef-cssunparsedsegment
  using CSSUnparsedSegment = Variant<CSSOMString, RefPtr<CSSVariableReferenceValue>>;

  /// @brief A value that contains variable references (or a custom property's value), as a list of string
  /// segments and variable references.
  /// @see https://drafts.css-houdini.org/css-typed-om/#cssunparsedvalue
  class CSSUnparsedValue final : public CSSStyleValue
  {
  private:
    List<CSSUnparsedSegment> _segments;

  public:
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssunparsedvalue-cssunparsedvalue
    KRYS_NODISCARD static Ref<CSSUnparsedValue> Create(List<CSSUnparsedSegment> segments) noexcept;
    /// @brief Splits @p tokens into string segments and the variable references (`var()`) in them.
    /// @see https://drafts.css-houdini.org/css-typed-om/#reify-a-list-of-component-values
    KRYS_NODISCARD static Ref<CSSUnparsedValue> Create(TokenRange tokens) noexcept;

    ~CSSUnparsedValue() noexcept override;

    void Serialise(StringBuilder &builder, EnumSet<SerialisationArguments> arguments) const noexcept final;

    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssunparsedvalue-length
    KRYS_NODISCARD size_t Length() const noexcept
    {
      return _segments.size();
    }
    KRYS_NODISCARD bool IsSupportedPropertyIndex(size_t index) const noexcept
    {
      return index < _segments.size();
    }
    /// @return Nothing if @p index is out of range.
    KRYS_NODISCARD Maybe<CSSUnparsedSegment> Item(size_t index) const noexcept;
    /// @brief Replaces the segment at @p index, or appends one at the end.
    /// @return A `RangeError` if @p index is past the end.
    dom::ExceptionOr<CSSUnparsedSegment> SetItem(size_t index, CSSUnparsedSegment segment) noexcept;
    KRYS_NODISCARD const List<CSSUnparsedSegment> &Segments() const noexcept
    {
      return _segments;
    }

    KRYS_NODISCARD CSSStyleValueType StyleValueType() const noexcept final
    {
      return CSSStyleValueType::CSSUnparsedValue;
    }

    KRYS_NODISCARD RefPtr<Value> ToCSSValue() const noexcept final;

  private:
    explicit CSSUnparsedValue(List<CSSUnparsedSegment> segments) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSUnparsedValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleValue &styleValue) noexcept
  {
    return styleValue.StyleValueType() == krys::boo::css::CSSStyleValueType::CSSUnparsedValue;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
