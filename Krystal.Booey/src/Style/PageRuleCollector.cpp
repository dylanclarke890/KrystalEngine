#include "Krystal.Booey/Style/PageRuleCollector.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Style/RuleSet.hpp"
#include "Krystal.Booey/Style/ScopeRuleSets.hpp"
#include "Krystal.Booey/Style/UserAgentStyle.hpp"
#include <algorithm>

namespace krys::boo::style
{
  namespace
  {
    KRYS_NODISCARD bool ComparePageRules(const css::PageRule *r1, const css::PageRule *r2) noexcept
    {
      return r1->Selector()->SpecificityForPage() < r2->Selector()->SpecificityForPage();
    }

    KRYS_NODISCARD bool CheckPageSelectorComponents(const css::Selector *selector, bool isLeftPage,
                                                    bool isFirstPage,
                                                    const css::CSSOMString &pageName) noexcept
    {
      for (auto *component = selector; component; component = component->PrecedingComplexSelectorComponent())
      {
        if (component->Match() == css::SelectorMatch::Type)
        {
          auto &localName = component->TagQualifiedName().LocalName();
          if (localName != dom::StarAtom() && localName.View() != css::CSSOMStringView(pageName))
          {
            return false;
          }
        }
        else if (component->Match() == css::SelectorMatch::PagePseudoClass)
        {
          auto pseudoType = component->PagePseudoClass();
          if ((pseudoType == css::SelectorPagePseudoClass::Left && !isLeftPage)
              || (pseudoType == css::SelectorPagePseudoClass::Right && isLeftPage)
              || (pseudoType == css::SelectorPagePseudoClass::First && !isFirstPage))
          {
            return false;
          }
        }
      }
      return true;
    }
  }

  bool PageRuleCollector::IsLeftPage(int32 pageIndex) const noexcept
  {
    bool isFirstPageLeft = !_rootWritingMode.IsAnyLeftToRight();
    return (pageIndex + (isFirstPageLeft ? 1 : 0)) % 2;
  }

  bool PageRuleCollector::IsFirstPage(int32 pageIndex) const noexcept
  {
    // FIXME: In case of forced left/right page, page at index 1 (not 0) can be the first page.
    return !pageIndex;
  }

  css::CSSOMString PageRuleCollector::PageName(int32 /* pageIndex */) const noexcept
  {
    // FIXME: Implement page index to page name mapping.
    return {};
  }

  void PageRuleCollector::MatchAllPageRules(int32 pageIndex) noexcept
  {
    const bool isLeft = IsLeftPage(pageIndex);
    const bool isFirst = IsFirstPage(pageIndex);
    const auto page = PageName(pageIndex);

    MatchPageRules(UserAgentStyle::DefaultPrintStyle, isLeft, isFirst, page);
    MatchPageRules(_ruleSets.UserStyle(), isLeft, isFirst, page);
    // Only consider the global author RuleSet for @page rules, as per the HTML5 spec.
    if (_ruleSets.IsAuthorStyleDefined())
    {
      MatchPageRules(&_ruleSets.AuthorStyle(), isLeft, isFirst, page);
    }
  }

  void PageRuleCollector::MatchPageRules(const RuleSet *rules, bool isLeftPage, bool isFirstPage,
                                         const css::CSSOMString &pageName) noexcept
  {
    if (!rules)
    {
      return;
    }

    List<css::PageRule *> matchedPageRules;
    MatchPageRulesForList(matchedPageRules, rules->PageRules(), isLeftPage, isFirstPage, pageName);
    if (matchedPageRules.empty())
    {
      return;
    }

    std::ranges::stable_sort(matchedPageRules, ComparePageRules);

    for (auto *pageRule : matchedPageRules)
    {
      _result->AuthorDeclarations.push_back(MatchedProperties {ShareRef(pageRule->Properties())});
    }
  }

  void PageRuleCollector::MatchPageRulesForList(List<css::PageRule *> &matchedRules,
                                                const List<css::PageRule *> &rules, bool isLeftPage,
                                                bool isFirstPage, const css::CSSOMString &pageName) noexcept
  {
    for (auto *rule : rules)
    {
      if (!CheckPageSelectorComponents(rule->Selector(), isLeftPage, isFirstPage, pageName))
      {
        continue;
      }

      // If the rule has no properties to apply, then ignore it.
      auto &properties = rule->Properties();
      if (properties.empty())
      {
        continue;
      }

      // Add this rule to our list of matched rules.
      matchedRules.push_back(rule);
    }
  }
}
