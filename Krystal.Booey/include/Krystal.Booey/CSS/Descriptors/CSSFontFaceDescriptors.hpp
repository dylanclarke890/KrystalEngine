/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Descriptors/PropertySetCSSDescriptors.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::css
{
  class CSSFontFaceRule;

  /// @brief The descriptors of an `@font-face` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-fonts/#cssfontfacedescriptors
  class CSSFontFaceDescriptors final : public PropertySetCSSDescriptors
  {
  private:
    CSSFontFaceDescriptors(MutableStyleProperties &propertySet, CSSFontFaceRule &parentRule) noexcept;

  public:
    ~CSSFontFaceDescriptors() noexcept;

    KRYS_NODISCARD static Ref<CSSFontFaceDescriptors> Create(MutableStyleProperties &propertySet,
                                                             CSSFontFaceRule &parentRule) noexcept;

    KRYS_NODISCARD StyleDeclarationType GetStyleDeclarationType() const noexcept final
    {
      return StyleDeclarationType::FontFace;
    }

    KRYS_NODISCARD CSSOMString Src() const noexcept;
    dom::ExceptionOr<void> SetSrc(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString FontFamily() const noexcept;
    dom::ExceptionOr<void> SetFontFamily(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString FontStyle() const noexcept;
    dom::ExceptionOr<void> SetFontStyle(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString FontWeight() const noexcept;
    dom::ExceptionOr<void> SetFontWeight(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString FontWidth() const noexcept;
    dom::ExceptionOr<void> SetFontWidth(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString SizeAdjust() const noexcept;
    dom::ExceptionOr<void> SetSizeAdjust(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString UnicodeRange() const noexcept;
    dom::ExceptionOr<void> SetUnicodeRange(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString FontFeatureSettings() const noexcept;
    dom::ExceptionOr<void> SetFontFeatureSettings(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString FontDisplay() const noexcept;
    dom::ExceptionOr<void> SetFontDisplay(const CSSOMString &value) noexcept;

  private:
    KRYS_NODISCARD RuleType GetRuleType() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSFontFaceDescriptors)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleDeclaration &declaration) noexcept
  {
    return declaration.GetStyleDeclarationType() == krys::boo::css::StyleDeclarationType::FontFace;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
