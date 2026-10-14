#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class PropertyRule;

  /// @brief An `@property` rule, exposed to scripts.
  /// @see https://drafts.css-houdini.org/css-properties-values-api/#the-css-property-rule-interface
  class CSSPropertyRule final : public CSSRule
  {
  private:
    Ref<css::PropertyRule> _propertyRule;

    CSSPropertyRule(css::PropertyRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSPropertyRule() noexcept;

    KRYS_NODISCARD static Ref<CSSPropertyRule> Create(css::PropertyRule &rule,
                                                      CSSStyleSheet *parent) noexcept;

    KRYS_NODISCARD CSSOMString Name() const noexcept;
    KRYS_NODISCARD CSSOMString Syntax() const noexcept;
    KRYS_NODISCARD bool Inherits() const noexcept;

    /// @brief The initial value, or null if the rule has none.
    KRYS_NODISCARD Maybe<CSSOMString> InitialValue() const noexcept;

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;

  private:
    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::Property;
    }

    void Reattach(Rule &rule) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSPropertyRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::Property;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
