#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class LayerRule;

  /// @brief An `@layer` statement rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-cascade-5/#the-csslayerstatementrule-interface
  class CSSLayerStatementRule final : public CSSRule
  {
  private:
    Ref<LayerRule> _layerRule;

    CSSLayerStatementRule(LayerRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSLayerStatementRule() noexcept;

    KRYS_NODISCARD static Ref<CSSLayerStatementRule> Create(LayerRule &rule, CSSStyleSheet *parent) noexcept;

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    KRYS_NODISCARD List<CSSOMString> NameList() const noexcept;

  private:
    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::LayerStatement;
    }

    void Reattach(Rule &rule) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSLayerStatementRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::LayerStatement;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
