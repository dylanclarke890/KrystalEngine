#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Web/Css/CSSOM/CSSRule.hpp"

namespace krys::web::css
{
  class GroupRule;
}

namespace krys::web
{
  class CSSRuleList;

  /// @brief A rule that holds other rules, its child CSS rules, such as `@media` or `@layer`.
  /// @see https://drafts.csswg.org/cssom/#the-cssgroupingrule-interface
  class CSSGroupingRule : public CSSRule
  {
  private:
    Ref<css::GroupRule> _groupRule;
    // The wrappers of the child rules made so far, by index, and the live list of them.
    mutable List<RefPtr<CSSRule>> _childRuleCSSOMWrappers;
    mutable UniquePtr<CSSRuleList> _ruleListCSSOMWrapper;

  protected:
    CSSGroupingRule(css::GroupRule &groupRule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSGroupingRule() noexcept override;

#pragma region CSSGroupingRule - https://drafts.csswg.org/cssom/#the-cssgroupingrule-interface

    /// @brief A live list of the child rules.
    /// @see https://drafts.csswg.org/cssom/#dom-cssgroupingrule-cssrules
    KRYS_NODISCARD CSSRuleList &CssRules() const noexcept;

    /// @brief Parses @p rule and inserts it among the child rules at @p index, returning the index.
    /// @see https://drafts.csswg.org/cssom/#dom-cssgroupingrule-insertrule
    ExceptionOr<uint32> InsertRule(const CSSOMString &rule, uint32 index) noexcept;

    /// @brief Removes the child rule at @p index.
    /// @see https://drafts.csswg.org/cssom/#dom-cssgroupingrule-deleterule
    ExceptionOr<void> DeleteRule(uint32 index) noexcept;

#pragma endregion

    /// @brief The number of child rules.
    KRYS_NODISCARD uint32 Length() const noexcept;

    /// @brief The wrapper of the child rule at @p index, made when first asked for; null past the end.
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

    void Reattach(css::Rule &rule) noexcept override;

    /// @brief Appends ` {`, each child rule's `cssText` on a line of its own, and `}`.
    void AppendCSSTextForItems(StringBuilder &builder) const noexcept;

    /// @brief As `AppendCSSTextForItems`, with the child rules serialised in @p context.
    void AppendCSSTextWithReplacementUrlsForItems(StringBuilder &builder,
                                                  const css::SerialisationContext &context) const noexcept;

    KRYS_NODISCARD RefPtr<css::StyleRuleWithNesting>
      PrepareChildStyleRuleForNesting(css::StyleRule &styleRule) noexcept override;

  private:
    KRYS_NODISCARD bool IsGroupingRule() const noexcept final
    {
      return true;
    }

    static void AppendCSSTextForItemsInternal(StringBuilder &builder, const StringBuilder &rules) noexcept;

    void CssTextForRules(StringBuilder &rules) const noexcept;

    void CssTextForRulesWithReplacementUrls(StringBuilder &rules,
                                            const css::SerialisationContext &context) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::CSSGroupingRule)
  KRYS_NODISCARD static bool IsType(const krys::web::CSSRule &rule) noexcept
  {
    return rule.IsGroupingRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
