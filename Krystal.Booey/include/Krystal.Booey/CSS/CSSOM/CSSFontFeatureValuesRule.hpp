#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class FontFeatureValuesBlockRule;
  class FontFeatureValuesRule;

  /// @brief An `@font-feature-values` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-fonts/#om-fontfeaturevalues
  class CSSFontFeatureValuesRule final : public CSSRule
  {
  private:
    Ref<css::FontFeatureValuesRule> _fontFeatureValuesRule;

    CSSFontFeatureValuesRule(css::FontFeatureValuesRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    KRYS_NODISCARD static Ref<CSSFontFeatureValuesRule> Create(css::FontFeatureValuesRule &rule,
                                                               CSSStyleSheet *sheet) noexcept;

    KRYS_NODISCARD const List<CSSOMStringAtom> &FontFamilies() const noexcept;

    // Used by the CSSOM.
    KRYS_NODISCARD CSSOMStringAtom FontFamily() const noexcept;

  private:
    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::FontFeatureValues;
    }

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    void Reattach(Rule &rule) noexcept final;
  };

  /// @brief A feature value block inside an `@font-feature-values` rule, exposed to scripts.
  class CSSFontFeatureValuesBlockRule final : public CSSRule
  {
  private:
    Ref<css::FontFeatureValuesBlockRule> _fontFeatureValuesBlockRule;

    CSSFontFeatureValuesBlockRule(css::FontFeatureValuesBlockRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    KRYS_NODISCARD static Ref<CSSFontFeatureValuesBlockRule> Create(css::FontFeatureValuesBlockRule &rule,
                                                                    CSSStyleSheet *sheet) noexcept;

  private:
    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::FontFeatureValuesBlock;
    }

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    void Reattach(Rule &rule) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSFontFeatureValuesRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::FontFeatureValues;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSFontFeatureValuesBlockRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::FontFeatureValuesBlock;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
