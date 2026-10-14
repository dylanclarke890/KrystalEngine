#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class FontPaletteValuesRule;

  /// @brief An `@font-palette-values` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-fonts/#om-fontpalettevalues
  class CSSFontPaletteValuesRule final : public CSSRule
  {
  private:
    Ref<css::FontPaletteValuesRule> _fontPaletteValuesRule;

    CSSFontPaletteValuesRule(css::FontPaletteValuesRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSFontPaletteValuesRule() noexcept;

    KRYS_NODISCARD static Ref<CSSFontPaletteValuesRule> Create(css::FontPaletteValuesRule &rule,
                                                               CSSStyleSheet *sheet) noexcept;

    KRYS_NODISCARD CSSOMString Name() const noexcept;
    KRYS_NODISCARD CSSOMString FontFamily() const noexcept;
    KRYS_NODISCARD CSSOMString BasePalette() const noexcept;
    KRYS_NODISCARD CSSOMString OverrideColors() const noexcept;

  private:
    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::FontPaletteValues;
    }

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    void Reattach(Rule &rule) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSFontPaletteValuesRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::FontPaletteValues;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
