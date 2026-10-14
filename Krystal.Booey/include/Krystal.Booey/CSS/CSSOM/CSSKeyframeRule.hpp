#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSKeyframesRule;
  class CSSStyleProperties;
  class KeyframeRule;
  class StyleRuleCSSStyleProperties;

  /// @brief A keyframe of an `@keyframes` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-animations/#interface-csskeyframerule
  class CSSKeyframeRule final : public CSSRule
  {
    friend class CSSKeyframesRule;

  private:
    const Ref<KeyframeRule> _keyframe;
    mutable RefPtr<StyleRuleCSSStyleProperties> _propertiesCSSOMWrapper;

    CSSKeyframeRule(KeyframeRule &keyframe, CSSKeyframesRule *parent) noexcept;

  public:
    ~CSSKeyframeRule() noexcept;

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    void Reattach(Rule &rule) noexcept final;

    KRYS_NODISCARD CSSOMString KeyText() const noexcept;
    void SetKeyText(const CSSOMString &text) noexcept;

    KRYS_NODISCARD CSSStyleProperties &Style() noexcept;

  private:
    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::Keyframe;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSKeyframeRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::Keyframe;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
