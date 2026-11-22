#include "Krystal.Booey/CSS/CSSOM/CSSStyleRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSRuleList.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSStyleProperties.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Parser/Parser.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParser.hpp"
#include "Krystal.Booey/CSS/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Booey/CSS/TypedOM/DeclaredStylePropertyMap.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Style/Rules/RuleData.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Memory/NeverDestroyed.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"

namespace krys::boo
{
  CSSStyleRule::CSSStyleRule(css::StyleRule &rule, CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _rule(ShareRef(rule))
  {
    _childRules.resize(NestedRules().size());
  }

  CSSStyleRule::~CSSStyleRule() noexcept
  {
    // The declaration block can outlive the rule, which is no longer its parent.
    if (_style)
    {
      _style->ClearParentRule();
    }
  }

  Ref<CSSStyleRule> CSSStyleRule::Create(css::StyleRule &rule, CSSStyleSheet *parent) noexcept
  {
    return AdoptRef(*new CSSStyleRule(rule, parent));
  }

  CSSOMString CSSStyleRule::SelectorText() const noexcept
  {
    // A nested rule's selectors are kept as written, as well as resolved against its parent's.
    if (auto *rule = DynamicDowncast<css::StyleRuleWithNesting>(_rule.get()))
    {
      return rule->OriginalSelectorList().SelectorsText();
    }
    return _rule->GetSelectorList().SelectorsText();
  }

  void CSSStyleRule::SetSelectorText(const CSSOMString &text) noexcept
  {
    // 1. Run the parse a group of selectors algorithm on the given value.
    auto *styleSheet = ParentStyleSheet();
    auto selectors = css::SelectorParser::ParseSelectorList(
      text, css::SelectorParserContext(GetParserContext()), styleSheet ? &styleSheet->Contents() : nullptr,
      GetNestedContext());

    // 3. Otherwise, if the algorithm returns a null value, do nothing.
    // (Nor for selectors too long for the style system's rules; a parsed style sheet splits them into rules.)
    if (!selectors || selectors->ComponentCount() > style::RuleData::MaximumSelectorComponentCount)
    {
      return;
    }

    // 2. If the algorithm returns a non-null value replace the associated selector list with the returned
    //    value.
    CSSStyleSheet::RuleMutationScope mutationScope(this);
    if (auto *rule = DynamicDowncast<css::StyleRuleWithNesting>(_rule.get()))
    {
      rule->WrapperAdoptOriginalSelectorList(krys::move(*selectors));
    }
    else
    {
      _rule->WrapperAdoptSelectorList(krys::move(*selectors));
    }
  }

  CSSStyleProperties &CSSStyleRule::Style() noexcept
  {
    if (!_style)
    {
      _style = StyleRuleCSSStyleProperties::Create(_rule->MutableProperties(), *this);
    }
    return *_style;
  }

  StylePropertyMap &CSSStyleRule::StyleMap() noexcept
  {
    if (!_styleMap)
    {
      _styleMap = DeclaredStylePropertyMap::Create(*this);
    }
    return *_styleMap;
  }

  CSSRuleList &CSSStyleRule::CssRules() const noexcept
  {
    if (!_cssRules)
    {
      _cssRules = CreateUnique<LiveCSSRuleList<CSSStyleRule>>(const_cast<CSSStyleRule &>(*this));
    }
    return *_cssRules;
  }

  ExceptionOr<uint32> CSSStyleRule::InsertRule(const CSSOMString &rule, uint32 index) noexcept
  {
    // To insert a CSS rule rule in a CSS rule list list at index index, with a flag nested (set):
    krys_debug_assert(_childRules.size() == NestedRules().size());

    // 1. Set length to the number of items in list.
    // 2. If index is greater than length, then throw an IndexSizeError exception.
    if (index > Length())
    {
      return ExceptionCode::IndexSizeError;
    }

    // 3. Set new rule to the results of performing parse a CSS rule on argument rule.
    auto *styleSheet = ParentStyleSheet();
    auto newRule =
      css::Parser::ParseRule(rule, GetParserContext(), styleSheet ? &styleSheet->Contents() : nullptr,
                             css::Parser::AllowedRules::ImportRules, css::NestedParserContextType::Style);

    // 4. If new rule is a syntax error, and nested is set, perform the following substeps: Set declarations
    //    to the results of performing parse a CSS declaration block, on argument rule.; If declarations is
    //    empty, throw a SyntaxError exception.; Otherwise, set new rule to a new nested declarations rule
    //    with declarations as it contents.
    // 5. If new rule is a syntax error, throw a SyntaxError exception.
    if (!newRule)
    {
      newRule = css::Parser::ParseNestedDeclarations(GetParserContext(), rule);
      if (!newRule)
      {
        return ExceptionCode::SyntaxError;
      }
    }

    // 6. If new rule cannot be inserted into list at the zero-indexed position index due to constraints
    //    specified by CSS, then throw a HierarchyRequestError exception.
    // (A style rule holds style rules, conditional group rules and nested declarations.)
    if (!newRule->IsStyleRule() && !newRule->IsGroupRule() && !newRule->IsNestedDeclarationsRule())
    {
      return ExceptionCode::HierarchyRequestError;
    }

    // 7. (Skipped: an @namespace rule can't be nested, so step 6 refused it.)

    // 8. Insert new rule into list at the zero-indexed position index.
    CSSStyleSheet::RuleMutationScope mutationScope(this);
    if (!_rule->IsStyleRuleWithNesting())
    {
      // The rule becomes one that can hold nested rules, in its parent's place.
      RefPtr<css::StyleRuleWithNesting> ruleWithNesting;
      if (auto *parentRule = ParentRule())
      {
        ruleWithNesting = parentRule->PrepareChildStyleRuleForNesting(*_rule);
      }
      else if (styleSheet)
      {
        ruleWithNesting = styleSheet->PrepareChildStyleRuleForNesting(*_rule);
      }
      else
      {
        ruleWithNesting = css::StyleRuleWithNesting::Create(krys::move(*_rule));
      }
      krys_debug_assert(ruleWithNesting);
      _rule = krys::move(ruleWithNesting);
    }
    auto &nestedRules = Downcast<css::StyleRuleWithNesting>(*_rule).NestedRules();
    nestedRules.insert(nestedRules.begin() + index, krys::move(newRule));
    _childRules.insert(_childRules.begin() + index, nullptr);

    // 9. Return index.
    return index;
  }

  ExceptionOr<void> CSSStyleRule::DeleteRule(uint32 index) noexcept
  {
    // To remove a CSS rule from a CSS rule list list at index index:
    krys_debug_assert(_childRules.size() == NestedRules().size());

    // 1. Set length to the number of items in list.
    // 2. If index is greater than or equal to length, then throw an IndexSizeError exception.
    if (index >= Length())
    {
      return ExceptionCode::IndexSizeError;
    }

    // 3. Set old rule to the indexth item in list.
    // 4. (Skipped: an @namespace rule can't be nested.)
    // 5. Remove rule old rule from list at the zero-indexed position index.
    CSSStyleSheet::RuleMutationScope mutationScope(this);
    auto &nestedRules = Downcast<css::StyleRuleWithNesting>(*_rule).NestedRules();
    nestedRules.erase(nestedRules.begin() + index);

    // 6. Set old rule's parent CSS rule and parent CSS style sheet to null.
    if (auto &oldRule = _childRules[index])
    {
      oldRule->SetParentRule(nullptr);
    }
    _childRules.erase(_childRules.begin() + index);
    return {};
  }

  uint32 CSSStyleRule::Length() const noexcept
  {
    return static_cast<uint32>(NestedRules().size());
  }

  CSSRule *CSSStyleRule::Item(uint32 index) const noexcept
  {
    if (index >= Length())
    {
      return nullptr;
    }
    krys_debug_assert(_childRules.size() == NestedRules().size());
    auto &rule = _childRules[index];
    if (!rule)
    {
      rule = NestedRules()[index]->CreateCSSOMWrapper(const_cast<CSSStyleRule &>(*this));
    }
    return rule.get();
  }

  CSSOMString CSSStyleRule::CssText() const noexcept
  {
    return CssText(css::DefaultSerialisationContext());
  }

  CSSOMString CSSStyleRule::CssText(const css::SerialisationContext &context) const noexcept
  {
    // 1. Let s initially be the result of performing serialize a group of selectors on the rule's associated
    //    selectors, followed by the string " {", i.e., a single SPACE (U+0020), followed by LEFT CURLY
    //    BRACKET (U+007B).
    StringBuilder s;
    s.Append(SelectorText(), u8" {");

    // 2. Let decls be the result of performing serialize a CSS declaration block on the rule's associated
    //    declarations, or null if there are no such declarations.
    auto declarations = _rule->Properties().AsText(context);

    // 3. Let rules be the result of performing serialize a CSS rule on each rule in the rule's cssRules list,
    //    or null if there are no such rules.
    // 4. If decls and rules are both null, append " }" to s (i.e. a single SPACE (U+0020) followed by RIGHT
    //    CURLY BRACKET (U+007D)) and return s.
    // 5. If rules is null:
    if (Length() == 0)
    {
      // 5a. Append a single SPACE (U+0020) to s
      // 5b. Append decls to s
      // 5c. Append " }" to s (i.e. a single SPACE (U+0020) followed by RIGHT CURLY BRACKET (U+007D)).
      // 5d. Return s.
      if (!declarations.empty())
      {
        s.Append(u8' ', declarations);
      }
      s.Append(u8" }");
      return krys::move(s).ToString();
    }

    // 6. Otherwise:
    // 6a. If decls is not null, prepend it to rules.
    // 6b. For each rule in rules: If rule is the empty string, do nothing.; Otherwise:
    //   6b.i. Append a newline followed by two spaces to s.
    //   6b.ii. Append rule to s.
    auto appendRule = [&](const CSSOMString &rule)
    {
      if (!rule.empty())
      {
        s.Append(u8"\n  ", rule);
      }
    };
    appendRule(declarations);
    for (uint32 index = 0; index < Length(); ++index)
    {
      appendRule(Item(index)->CssText(context));
    }

    // 6c. Append a newline followed by RIGHT CURLY BRACKET (U+007D) to s.
    // 6d. Return s.
    s.Append(u8"\n}");
    return krys::move(s).ToString();
  }

  void CSSStyleRule::Reattach(css::Rule &rule) noexcept
  {
    _rule = ShareRef(Downcast<css::StyleRule>(rule));
    if (_style)
    {
      _style->Reattach(_rule->MutableProperties());
    }
  }

  void CSSStyleRule::GetChildStyleSheets(HashSet<RefPtr<CSSStyleSheet>> &childStyleSheets) noexcept
  {
    for (uint32 index = 0; index < Length(); ++index)
    {
      Item(index)->GetChildStyleSheets(childStyleSheets);
    }
  }

  const List<Ref<css::Rule>> &CSSStyleRule::NestedRules() const noexcept
  {
    if (auto *rule = DynamicDowncast<css::StyleRuleWithNesting>(_rule.get()))
    {
      return rule->NestedRules();
    }
    static MainThreadNeverDestroyed<List<Ref<css::Rule>>> noRules;
    return noRules;
  }
}
