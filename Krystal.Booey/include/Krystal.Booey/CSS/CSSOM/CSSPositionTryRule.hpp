#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSPositionTryDescriptors;
  class PositionTryRule;

  /// @brief An `@position-try` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-anchor-position/#interfaces
  class CSSPositionTryRule final : public CSSRule
  {
  private:
    Ref<css::PositionTryRule> _positionTryRule;
    RefPtr<CSSPositionTryDescriptors> _propertiesCSSOMWrapper;

    CSSPositionTryRule(css::PositionTryRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSPositionTryRule() noexcept;

    KRYS_NODISCARD static Ref<CSSPositionTryRule> Create(css::PositionTryRule &rule,
                                                         CSSStyleSheet *parent) noexcept;

    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::PositionTry;
    }

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    void Reattach(Rule &rule) noexcept final;

    KRYS_NODISCARD Ref<css::PositionTryRule> ProtectedPositionTryRule() const noexcept;

    KRYS_NODISCARD CSSOMStringAtom Name() const noexcept;
    KRYS_NODISCARD CSSPositionTryDescriptors &Style() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSPositionTryRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::PositionTry;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
