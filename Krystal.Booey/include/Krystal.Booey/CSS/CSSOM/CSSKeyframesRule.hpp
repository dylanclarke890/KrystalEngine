#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::css
{
  class CSSKeyframeRule;
  class CSSRuleList;
  class KeyframesRule;

  /// @brief An `@keyframes` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-animations/#interface-csskeyframesrule
  class CSSKeyframesRule final : public CSSRule
  {
  private:
    Ref<KeyframesRule> _keyframesRule;
    mutable List<RefPtr<CSSKeyframeRule>> _childRuleCSSOMWrappers;
    mutable UniquePtr<CSSRuleList> _ruleListCSSOMWrapper;

    CSSKeyframesRule(KeyframesRule &keyframesRule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSKeyframesRule() noexcept;

    KRYS_NODISCARD static Ref<CSSKeyframesRule> Create(KeyframesRule &rule, CSSStyleSheet *sheet) noexcept;

    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::Keyframes;
    }

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    void Reattach(Rule &rule) noexcept final;

    KRYS_NODISCARD const CSSOMStringAtom &Name() const noexcept;
    void SetName(const CSSOMStringAtom &name) noexcept;

    KRYS_NODISCARD CSSRuleList &CssRules() noexcept;

    void AppendRule(const CSSOMString &rule) noexcept;
    void DeleteRule(const CSSOMString &key) noexcept;
    KRYS_NODISCARD CSSKeyframeRule *FindRule(const CSSOMString &key) noexcept;

    // For IndexedGetter and CSSRuleList.
    KRYS_NODISCARD uint32 Length() const noexcept;
    KRYS_NODISCARD CSSKeyframeRule *Item(uint32 index) const noexcept;

    KRYS_NODISCARD bool IsSupportedPropertyIndex(uint32 index) const noexcept
    {
      return index < Length();
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSKeyframesRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::Keyframes;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
