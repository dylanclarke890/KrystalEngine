#include "Krystal.Booey/Css/CSSOM/CSSRule.hpp"
#include "Krystal.Booey/Css/CSSOM/CSSScopeRule.hpp"
#include "Krystal.Booey/Css/CSSOM/CSSStyleRule.hpp"
#include "Krystal.Booey/Css/CSSOM/CSSStyleSheet.hpp"
#include "Krystal.Booey/Css/Parser/ParserContext.hpp"
#include "Krystal.Booey/Css/Rules/Rule.hpp"
#include "Krystal.Booey/Css/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Booey/Dom/Types/ExceptionOr.hpp"

namespace krys::boo
{
  ExceptionOr<void> CSSRule::SetCssText(const CSSOMString &) noexcept
  {
    return {};
  }

  uint16 CSSRule::TypeForCSSOM() const noexcept
  {
    switch (StyleRuleType())
    {
      case css::RuleType::Style:             return 1;
      case css::RuleType::Import:            return 3;
      case css::RuleType::Media:             return 4;
      case css::RuleType::FontFace:          return 5;
      case css::RuleType::Keyframes:         return 7;
      case css::RuleType::Keyframe:          return 8;
      case css::RuleType::Margin:            return 9;
      case css::RuleType::Namespace:         return 10;
      case css::RuleType::CounterStyle:      return 11;
      case css::RuleType::Supports:          return 12;
      case css::RuleType::FontFeatureValues: return 14;
      default:                               return 0;
    }
  }

  bool CSSRule::HasStyleRuleAncestor() const noexcept
  {
    for (auto *rule = ParentRule(); rule; rule = rule->ParentRule())
    {
      if (rule->StyleRuleType() == css::RuleType::Style)
      {
        return true;
      }
    }
    return false;
  }

  css::NestedContext CSSRule::GetNestedContext() const noexcept
  {
    for (auto *rule = ParentRule(); rule; rule = rule->ParentRule())
    {
      if (Is<CSSStyleRule>(*rule))
      {
        return css::NestedParserContextType::Style;
      }
      if (Is<CSSScopeRule>(*rule))
      {
        return css::NestedParserContextType::Scope;
      }
    }
    return {};
  }

  RefPtr<css::StyleRuleWithNesting> CSSRule::PrepareChildStyleRuleForNesting(css::StyleRule &) noexcept
  {
    return nullptr;
  }

  void CSSRule::GetChildStyleSheets(HashSet<RefPtr<CSSStyleSheet>> &) noexcept
  {
  }

  const css::ParserContext &CSSRule::GetParserContext() const noexcept
  {
    auto *styleSheet = ParentStyleSheet();
    return styleSheet ? styleSheet->Contents().GetParserContext() : css::StrictParserContext();
  }
}
