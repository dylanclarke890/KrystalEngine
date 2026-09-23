#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"

namespace krys::boo::css
{
  class CSSFontFaceDescriptors;
  class FontFaceRule;

  class CSSFontFaceRule final : public CSSRule
  {
  private:
    Ref<FontFaceRule> _rule;
    RefPtr<CSSFontFaceDescriptors> _propertiesCSSOMWrapper;

    CSSFontFaceRule(FontFaceRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    virtual ~CSSFontFaceRule() noexcept;

    KRYS_NODISCARD static Ref<CSSFontFaceRule> create(FontFaceRule &rule, CSSStyleSheet *sheet) noexcept
    {
      return AdoptRef(*new CSSFontFaceRule(rule, sheet));
    }

    KRYS_NODISCARD CSSFontFaceDescriptors &Style() noexcept;

  private:
    KRYS_NODISCARD RuleType Type() const noexcept final
    {
      return RuleType::FontFace;
    }

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;

    KRYS_NODISCARD CSSOMString CssText(const SerialisationContext &context) const noexcept final;

    KRYS_NODISCARD CSSOMString CssTextInternal(const CSSOMString &declarations) const noexcept;

    void Reattach(RuleBase &rule) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSFontFaceRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.Type() == krys::boo::css::RuleType::FontFace;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
