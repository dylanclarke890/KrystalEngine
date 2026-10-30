#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSScopeRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSStyleRule.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"

namespace krys::boo::css
{
  namespace
  {
    struct SameSizeAsCSSRule : public RefCounted<SameSizeAsCSSRule>, public CanMakeWeakPtr<SameSizeAsCSSRule>
    {
      virtual ~SameSizeAsCSSRule() noexcept;
      uint8 Bitfields;
      void *PointerUnion;
    };

    static_assert(sizeof(CSSRule) == sizeof(SameSizeAsCSSRule), "CSSRule should stay small");
  }

  uint16 CSSRule::TypeForCSSOM() const noexcept
  {
    // "This enumeration is thus frozen in its current state, and no new new values will be
    // added to reflect additional at-rules; all at-rules beyond the ones listed above will return 0."
    // https://drafts.csswg.org/cssom/#the-cssrule-interface
    if (StyleRuleType() >= FirstUnexposedRuleType)
    {
      return 0;
    }
    return static_cast<uint16>(StyleRuleType());
  }

  dom::ExceptionOr<void> CSSRule::SetCssText(const CSSOMString &) noexcept
  {
    return {};
  }

  const ParserContext &CSSRule::GetParserContext() const noexcept
  {
    auto *styleSheet = ParentStyleSheet();
    return styleSheet ? styleSheet->Contents().GetParserContext() : StrictParserContext();
  }

  bool CSSRule::HasStyleRuleAncestor() const noexcept
  {
    for (auto *current = ParentRule(); current; current = current->ParentRule())
    {
      if (current->StyleRuleType() == RuleType::Style)
      {
        return true;
      }
    }
    return false;
  }

  css::NestedContext CSSRule::GetNestedContext() const noexcept
  {
    for (auto *parentRule = ParentRule(); parentRule; parentRule = parentRule->ParentRule())
    {
      if (Is<CSSStyleRule>(*parentRule))
      {
        return NestedParserContextType::Style;
      }
      if (Is<CSSScopeRule>(*parentRule))
      {
        return NestedParserContextType::Scope;
      }
    }
    return {};
  }

  RefPtr<StyleRuleWithNesting> CSSRule::PrepareChildStyleRuleForNesting(StyleRule &) noexcept
  {
    return nullptr;
  }
}
