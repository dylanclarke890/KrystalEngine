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

#include "Krystal.Booey/CSS/Descriptors/CSSFontFaceDescriptors.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSFontFaceRule.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"

namespace krys::boo::css
{
  CSSFontFaceDescriptors::CSSFontFaceDescriptors(MutableStyleProperties &propertySet,
                                                 CSSFontFaceRule &parentRule) noexcept
      : PropertySetCSSDescriptors(propertySet, parentRule)
  {
  }

  CSSFontFaceDescriptors::~CSSFontFaceDescriptors() noexcept = default;

  Ref<CSSFontFaceDescriptors> CSSFontFaceDescriptors::Create(MutableStyleProperties &propertySet,
                                                             CSSFontFaceRule &parentRule) noexcept
  {
    return AdoptRef(*new CSSFontFaceDescriptors(propertySet, parentRule));
  }

  RuleType CSSFontFaceDescriptors::GetRuleType() const noexcept
  {
    return RuleType::FontFace;
  }

#pragma region Descriptors

  // @font-face 'src'
  CSSOMString CSSFontFaceDescriptors::Src() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::Src);
  }

  dom::ExceptionOr<void> CSSFontFaceDescriptors::SetSrc(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::Src, value, IsImportant(false));
  }

  // @font-face 'fontFamily'
  CSSOMString CSSFontFaceDescriptors::FontFamily() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::FontFamily);
  }

  dom::ExceptionOr<void> CSSFontFaceDescriptors::SetFontFamily(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::FontFamily, value, IsImportant(false));
  }

  // @font-face 'font-style'
  CSSOMString CSSFontFaceDescriptors::FontStyle() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::FontStyle);
  }

  dom::ExceptionOr<void> CSSFontFaceDescriptors::SetFontStyle(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::FontStyle, value, IsImportant(false));
  }

  // @font-face 'font-weight'
  CSSOMString CSSFontFaceDescriptors::FontWeight() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::FontWeight);
  }

  dom::ExceptionOr<void> CSSFontFaceDescriptors::SetFontWeight(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::FontWeight, value, IsImportant(false));
  }

  // @font-face 'font-width'
  CSSOMString CSSFontFaceDescriptors::FontWidth() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::FontWidth);
  }

  dom::ExceptionOr<void> CSSFontFaceDescriptors::SetFontWidth(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::FontWidth, value, IsImportant(false));
  }

  // @font-face 'size-adjust'
  CSSOMString CSSFontFaceDescriptors::SizeAdjust() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::SizeAdjust);
  }

  dom::ExceptionOr<void> CSSFontFaceDescriptors::SetSizeAdjust(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::SizeAdjust, value, IsImportant(false));
  }

  // @font-face 'unicode-range'
  CSSOMString CSSFontFaceDescriptors::UnicodeRange() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::UnicodeRange);
  }

  dom::ExceptionOr<void> CSSFontFaceDescriptors::SetUnicodeRange(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::UnicodeRange, value, IsImportant(false));
  }

  // @font-face 'font-feature-settings'
  CSSOMString CSSFontFaceDescriptors::FontFeatureSettings() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::FontFeatureSettings);
  }

  dom::ExceptionOr<void> CSSFontFaceDescriptors::SetFontFeatureSettings(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::FontFeatureSettings, value, IsImportant(false));
  }

  // @font-face 'font-display'
  CSSOMString CSSFontFaceDescriptors::FontDisplay() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::FontDisplay);
  }

  dom::ExceptionOr<void> CSSFontFaceDescriptors::SetFontDisplay(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::FontDisplay, value, IsImportant(false));
  }

#pragma endregion
}
