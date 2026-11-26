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

#include "Krystal.Booey/Css/CSSOM/CSSFontFaceDescriptors.hpp"
#include "Krystal.Booey/Css/CSSOM/CSSFontFaceRule.hpp"
#include "Krystal.Booey/Dom/Types/ExceptionOr.hpp"

namespace krys::boo
{
  CSSFontFaceDescriptors::CSSFontFaceDescriptors(css::MutableStyleProperties &propertySet,
                                                 CSSFontFaceRule &parentRule) noexcept
      : PropertySetCSSDescriptors(propertySet, parentRule)
  {
  }

  CSSFontFaceDescriptors::~CSSFontFaceDescriptors() noexcept = default;

  Ref<CSSFontFaceDescriptors> CSSFontFaceDescriptors::Create(css::MutableStyleProperties &propertySet,
                                                             CSSFontFaceRule &parentRule) noexcept
  {
    return AdoptRef(*new CSSFontFaceDescriptors(propertySet, parentRule));
  }

  css::RuleType CSSFontFaceDescriptors::GetRuleType() const noexcept
  {
    return css::RuleType::FontFace;
  }

#pragma region Descriptors

  // @font-face 'src'
  CSSOMString CSSFontFaceDescriptors::Src() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::Src);
  }

  ExceptionOr<void> CSSFontFaceDescriptors::SetSrc(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::Src, value, css::IsImportant::No);
  }

  // @font-face 'fontFamily'
  CSSOMString CSSFontFaceDescriptors::FontFamily() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::FontFamily);
  }

  ExceptionOr<void> CSSFontFaceDescriptors::SetFontFamily(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::FontFamily, value, css::IsImportant::No);
  }

  // @font-face 'font-style'
  CSSOMString CSSFontFaceDescriptors::FontStyle() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::FontStyle);
  }

  ExceptionOr<void> CSSFontFaceDescriptors::SetFontStyle(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::FontStyle, value, css::IsImportant::No);
  }

  // @font-face 'font-weight'
  CSSOMString CSSFontFaceDescriptors::FontWeight() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::FontWeight);
  }

  ExceptionOr<void> CSSFontFaceDescriptors::SetFontWeight(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::FontWeight, value, css::IsImportant::No);
  }

  // @font-face 'font-width'
  CSSOMString CSSFontFaceDescriptors::FontWidth() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::FontWidth);
  }

  ExceptionOr<void> CSSFontFaceDescriptors::SetFontWidth(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::FontWidth, value, css::IsImportant::No);
  }

  // @font-face 'size-adjust'
  CSSOMString CSSFontFaceDescriptors::SizeAdjust() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::SizeAdjust);
  }

  ExceptionOr<void> CSSFontFaceDescriptors::SetSizeAdjust(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::SizeAdjust, value, css::IsImportant::No);
  }

  // @font-face 'unicode-range'
  CSSOMString CSSFontFaceDescriptors::UnicodeRange() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::UnicodeRange);
  }

  ExceptionOr<void> CSSFontFaceDescriptors::SetUnicodeRange(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::UnicodeRange, value, css::IsImportant::No);
  }

  // @font-face 'font-feature-settings'
  CSSOMString CSSFontFaceDescriptors::FontFeatureSettings() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::FontFeatureSettings);
  }

  ExceptionOr<void> CSSFontFaceDescriptors::SetFontFeatureSettings(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::FontFeatureSettings, value, css::IsImportant::No);
  }

  // @font-face 'font-display'
  CSSOMString CSSFontFaceDescriptors::FontDisplay() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::FontDisplay);
  }

  ExceptionOr<void> CSSFontFaceDescriptors::SetFontDisplay(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::FontDisplay, value, css::IsImportant::No);
  }

#pragma endregion
}
