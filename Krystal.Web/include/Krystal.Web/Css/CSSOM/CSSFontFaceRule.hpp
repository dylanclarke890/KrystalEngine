#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Web/Css/CSSOM/CSSRule.hpp"

namespace krys::web::css
{
  class FontFaceRule;
}

namespace krys::web
{
  class CSSFontFaceDescriptors;

  /// @brief An `@font-face` rule, as scripts see it.
  /// @see https://drafts.csswg.org/css-fonts/#cssfontfacerule
  class CSSFontFaceRule final : public CSSRule
  {
  private:
    Ref<css::FontFaceRule> _rule;
    RefPtr<CSSFontFaceDescriptors> _style;

    CSSFontFaceRule(css::FontFaceRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSFontFaceRule() noexcept override;

    KRYS_NODISCARD static Ref<CSSFontFaceRule> Create(css::FontFaceRule &rule,
                                                      CSSStyleSheet *parent) noexcept;

    /// @brief The rule's descriptors, made when first asked for.
    /// @see https://drafts.csswg.org/css-fonts/#dom-cssfontfacerule-style
    KRYS_NODISCARD CSSFontFaceDescriptors &Style() noexcept;

  private:
    KRYS_NODISCARD css::RuleType StyleRuleType() const noexcept final
    {
      return css::RuleType::FontFace;
    }

    /// @see https://drafts.csswg.org/cssom/#serialize-a-css-rule
    KRYS_NODISCARD CSSOMString CssText() const noexcept final;

    /// @see https://drafts.csswg.org/cssom/#serialize-a-css-rule
    KRYS_NODISCARD CSSOMString CssText(const css::SerialisationContext &context) const noexcept final;

    void Reattach(css::Rule &rule) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::CSSFontFaceRule)
  KRYS_NODISCARD static bool IsType(const krys::web::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::web::css::RuleType::FontFace;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
