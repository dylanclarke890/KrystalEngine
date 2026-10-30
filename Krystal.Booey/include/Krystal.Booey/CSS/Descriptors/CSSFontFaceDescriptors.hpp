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
    KRYS_NODISCARD CSSOMString FontStretch() const noexcept;
    dom::ExceptionOr<void> SetFontStretch(const CSSOMString &value) noexcept;
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
