#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSFontFaceDescriptors;
  class FontFaceRule;

  /// @brief An `@font-face` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-fonts/#cssfontfacerule
  class CSSFontFaceRule final : public CSSRule
  {
  private:
    Ref<FontFaceRule> _fontFaceRule;
    RefPtr<CSSFontFaceDescriptors> _propertiesCSSOMWrapper;

    CSSFontFaceRule(FontFaceRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSFontFaceRule() noexcept;

    KRYS_NODISCARD static Ref<CSSFontFaceRule> Create(FontFaceRule &rule, CSSStyleSheet *sheet) noexcept;

    KRYS_NODISCARD CSSFontFaceDescriptors &Style() noexcept;

  private:
    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::FontFace;
    }

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    KRYS_NODISCARD CSSOMString CssText(const SerialisationContext &context) const noexcept final;
    KRYS_NODISCARD static CSSOMString CssTextInternal(const CSSOMString &declarations) noexcept;
    void Reattach(Rule &rule) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSFontFaceRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::FontFace;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
