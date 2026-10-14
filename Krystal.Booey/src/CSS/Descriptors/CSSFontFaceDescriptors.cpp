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

  // @font-face 'font-stretch'
  CSSOMString CSSFontFaceDescriptors::FontStretch() const noexcept
  {
    return GetPropertyValueInternal(
      PropertyId::FontWidth); // NOTE: 'font-stretch' is an alias for 'font-width'.
  }

  dom::ExceptionOr<void> CSSFontFaceDescriptors::SetFontStretch(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::FontWidth, value,
                               IsImportant(false)); // NOTE: 'font-stretch' is an alias for 'font-width'.
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
