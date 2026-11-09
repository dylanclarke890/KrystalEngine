/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/TypedOM/Color/CSSColorValue.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::css
{
  /// @brief A colour in a colour space given by name (a `color()` function).
  /// @see https://drafts.css-houdini.org/css-typed-om/#csscolor
  class CSSColor final : public CSSColorValue
  {
  private:
    Ref<CSSKeywordValue> _colorSpace;
    List<RectifiedCSSColorComponent> _channels;
    CSSNumberish _alpha;

  public:
    /// @return A `SyntaxError` if a channel is neither a percentage nor `none`.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-csscolor-csscolor
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSColor>>
      Create(CSSKeywordish colorSpace, List<CSSColorPercent> channels, CSSNumberish alpha = 1.0) noexcept;

    KRYS_NODISCARD Ref<CSSKeywordValue> ColorSpace() const noexcept final;
    void SetColorSpace(CSSKeywordish colorSpace) noexcept;
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-csscolor-channels
    KRYS_NODISCARD List<CSSColorPercent> Channels() const noexcept;
    KRYS_NODISCARD const CSSNumberish &Alpha() const noexcept
    {
      return _alpha;
    }
    void SetAlpha(CSSNumberish alpha) noexcept
    {
      _alpha = krys::move(alpha);
    }

    KRYS_NODISCARD CSSStyleValueType StyleValueType() const noexcept final
    {
      return CSSStyleValueType::CSSColor;
    }

  private:
    CSSColor(Ref<CSSKeywordValue> colorSpace, List<RectifiedCSSColorComponent> channels,
             CSSNumberish alpha) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSColor)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleValue &styleValue) noexcept
  {
    return styleValue.StyleValueType() == krys::boo::css::CSSStyleValueType::CSSColor;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
