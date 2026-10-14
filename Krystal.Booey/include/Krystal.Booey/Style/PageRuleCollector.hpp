#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Style/MatchResult.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::css
{
  class PageRule;
}

namespace krys::boo::style
{
  class RuleSet;
  class ScopeRuleSets;

  /// @brief Collects the `@page` rules that apply to a page.
  class PageRuleCollector
  {
    KRYS_NON_COPYABLE(PageRuleCollector);

  private:
    ScopeRuleSets &_ruleSets;
    WritingMode _rootWritingMode;

    const Ref<MatchResult> _result;

  public:
    PageRuleCollector(ScopeRuleSets &ruleSets, WritingMode rootWritingMode) noexcept
        : _ruleSets(ruleSets), _rootWritingMode(rootWritingMode), _result(MatchResult::Create())
    {
    }

    void MatchAllPageRules(int32 pageIndex) noexcept;

    KRYS_NODISCARD const MatchResult &GetMatchResult() const noexcept
    {
      return *_result;
    }

  private:
    KRYS_NODISCARD bool IsLeftPage(int32 pageIndex) const noexcept;

    KRYS_NODISCARD bool IsRightPage(int32 pageIndex) const noexcept
    {
      return !IsLeftPage(pageIndex);
    }

    KRYS_NODISCARD bool IsFirstPage(int32 pageIndex) const noexcept;
    KRYS_NODISCARD css::CSSOMString PageName(int32 pageIndex) const noexcept;

    void MatchPageRules(const RuleSet *rules, bool isLeftPage, bool isFirstPage,
                        const css::CSSOMString &pageName) noexcept;
    void MatchPageRulesForList(List<css::PageRule *> &matchedRules, const List<css::PageRule *> &rules,
                               bool isLeftPage, bool isFirstPage, const css::CSSOMString &pageName) noexcept;
  };
}
