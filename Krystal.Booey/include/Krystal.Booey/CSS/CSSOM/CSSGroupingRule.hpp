#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <memory>

namespace krys::boo::css
{
  class CSSRuleList;
  class GroupRule;

  class CSSGroupingRule : public CSSRule
  {
  private:
    Ref<GroupRule> _rule;
    mutable SmallList<RefPtr<CSSRule>> _childRuleCSSOMWrappers;
    const UniquePtr<CSSRuleList> _ruleListCSSOMWrapper;

  protected:
    CSSGroupingRule(GroupRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    virtual ~CSSGroupingRule() noexcept;

    KRYS_NODISCARD CSSRuleList &CssRules() const noexcept;
    
    dom::ExceptionOr<size_t> InsertRule(const CSSOMString &rule, size_t index) noexcept;
    
    dom::ExceptionOr<void> DeleteRule(size_t index) noexcept;
    
    KRYS_NODISCARD size_t Length() const noexcept;
    
    KRYS_NODISCARD CSSRule *Item(size_t index) const noexcept;

    virtual bool IsCSSConditionRule() const noexcept
    {
      return false;
    }

  protected:
    KRYS_NODISCARD const GroupRule &GroupRule() const noexcept
    {
      return *_rule;
    }

    KRYS_NODISCARD GroupRule &GroupRule() noexcept
    {
      return *_rule;
    }

    void Reattach(RuleBase &rule) noexcept override;

    void AppendCSSTextForItems(CSSOMString &builder) const noexcept;

    void AppendCSSTextWithReplacementURLsForItems(CSSOMString &builder, const SerialisationContext &context) const noexcept;

    RefPtr<StyleRuleWithNesting> PrepareChildStyleRuleForNesting(Rule &rule) noexcept override;

  private:
    KRYS_NODISCARD bool IsGroupingRule() const noexcept final
    {
      return true;
    }

    void AppendCSSTextForItemsInternal(CSSOMString &builder, CSSOMString &replacement) const noexcept;

    void CssTextForRules(CSSOMString &builder) const noexcept;

    void CssTextForRulesWithReplacementURLs(CSSOMString &builder, const SerialisationContext &context) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_BEGIN(krys::boo::css::CSSGroupingRule)
  KRYS_NO_DISCARD static bool isType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.IsGroupingRule();
  }
KRYS_SPECIALIZE_TYPE_TRAITS_END()
