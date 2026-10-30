#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSGroupingRule.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::css
{
  /// @brief An `@layer` block rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-cascade-5/#the-csslayerblockrule-interface
  class CSSLayerBlockRule final : public CSSGroupingRule
  {
  private:
    CSSLayerBlockRule(LayerRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    KRYS_NODISCARD static Ref<CSSLayerBlockRule> Create(LayerRule &rule, CSSStyleSheet *parent) noexcept;

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    KRYS_NODISCARD CSSOMString Name() const noexcept;

  private:
    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::LayerBlock;
    }
  };

  /// @brief @p name as a dotted, escaped layer name (`a.b`).
  KRYS_NODISCARD CSSOMString StringFromCascadeLayerName(const CascadeLayerName &name) noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSLayerBlockRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::LayerBlock;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
