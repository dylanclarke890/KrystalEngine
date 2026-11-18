/*
 * Copyright (C) 2018-2025 Apple Inc. All rights reserved.
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
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class CachedImage;
  class Document;
}

namespace krys::boo::css
{
  class ImageValue;
}

namespace krys::boo
{
  /// @brief An image value (`url()`), as the Typed OM represents it.
  /// @see https://drafts.css-houdini.org/css-typed-om/#cssimagevalue
  class CSSImageValue final : public CSSStyleValue
  {
  private:
    const Ref<css::ImageValue> _cssValue;
    WeakPtr<boo::Document> _document;

  public:
    KRYS_NODISCARD static Ref<CSSImageValue> Create(Ref<css::ImageValue> cssValue,
                                                    boo::Document &document) noexcept;

    ~CSSImageValue() noexcept override;

    void Serialise(StringBuilder &builder, EnumSet<SerialisationArguments> arguments) const noexcept final;

    /// @brief The image, once it has started loading.
    KRYS_NODISCARD CachedImage *Image() const noexcept;

    KRYS_NODISCARD boo::Document *Document() const noexcept;

    KRYS_NODISCARD CSSStyleValueType StyleValueType() const noexcept final
    {
      return CSSStyleValueType::CSSImageValue;
    }

    KRYS_NODISCARD RefPtr<css::Value> ToCSSValue() const noexcept final;

  private:
    CSSImageValue(Ref<css::ImageValue> cssValue, boo::Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::CSSImageValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::CSSStyleValue &styleValue) noexcept
  {
    return styleValue.StyleValueType() == krys::boo::CSSStyleValueType::CSSImageValue;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
