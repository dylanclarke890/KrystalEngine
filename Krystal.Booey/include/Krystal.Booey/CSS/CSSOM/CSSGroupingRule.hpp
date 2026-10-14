#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::css
{
  class CSSRuleList;
  class GroupRule;

  /// @brief The base of the rules that contain other rules.
  /// @see https://drafts.csswg.org/cssom/#the-cssgroupingrule-interface
  class CSSGroupingRule : public CSSRule
  {
  private:
    Ref<css::GroupRule> _groupRule;
    mutable List<RefPtr<CSSRule>> _childRuleCSSOMWrappers;
    mutable UniquePtr<CSSRuleList> _ruleListCSSOMWrapper;

  protected:
    CSSGroupingRule(css::GroupRule &groupRule, CSSStyleSheet *parent) noexcept;

  public:
    virtual ~CSSGroupingRule() noexcept;

    KRYS_NODISCARD CSSRuleList &CssRules() const noexcept;

    dom::ExceptionOr<uint32> InsertRule(const CSSOMString &rule, uint32 index) noexcept;
    dom::ExceptionOr<void> DeleteRule(uint32 index) noexcept;

    KRYS_NODISCARD uint32 Length() const noexcept;
    KRYS_NODISCARD CSSRule *Item(uint32 index) const noexcept;

    KRYS_NODISCARD virtual bool IsCSSConditionRule() const noexcept
    {
      return false;
    }

  protected:
    KRYS_NODISCARD const css::GroupRule &GetGroupRule() const noexcept
    {
      return *_groupRule;
    }

    KRYS_NODISCARD css::GroupRule &GetGroupRule() noexcept
    {
      return *_groupRule;
    }

    void Reattach(Rule &rule) noexcept override;

    void AppendCSSTextForItems(CSSOMString &builder) const noexcept;
    void AppendCSSTextWithReplacementURLsForItems(CSSOMString &builder,
                                                  const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD RefPtr<StyleRuleWithNesting>
      PrepareChildStyleRuleForNesting(StyleRule &styleRule) noexcept override;

  private:
    KRYS_NODISCARD bool IsGroupingRule() const noexcept final
    {
      return true;
    }

    static void AppendCSSTextForItemsInternal(CSSOMString &builder, const CSSOMString &rules) noexcept;
    void CssTextForRules(CSSOMString &rules) const noexcept;
    void CssTextForRulesWithReplacementURLs(CSSOMString &rules,
                                            const SerialisationContext &context) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSGroupingRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.IsGroupingRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
