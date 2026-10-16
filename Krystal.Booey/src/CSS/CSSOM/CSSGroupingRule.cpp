#include "Krystal.Booey/CSS/CSSOM/CSSGroupingRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSRuleList.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Parser/Parser.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  CSSGroupingRule::CSSGroupingRule(css::GroupRule &groupRule, CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _groupRule(ShareRef(groupRule)),
        _childRuleCSSOMWrappers(groupRule.ChildRules().size())
  {
  }

  CSSGroupingRule::~CSSGroupingRule() noexcept
  {
    krys_debug_assert(_childRuleCSSOMWrappers.size() == _groupRule->ChildRules().size());
    for (auto &wrapper : _childRuleCSSOMWrappers)
    {
      if (wrapper)
      {
        wrapper->SetParentRule(nullptr);
      }
    }
  }

  dom::ExceptionOr<uint32> CSSGroupingRule::InsertRule(const CSSOMString &ruleString, uint32 index) noexcept
  {
    krys_debug_assert(_childRuleCSSOMWrappers.size() == _groupRule->ChildRules().size());

    if (index > _groupRule->ChildRules().size())
    {
      // IndexSizeError: Raised if the specified index is not a valid insertion point.
      return dom::ExceptionCode::IndexSizeError;
    }

    auto *styleSheet = ParentStyleSheet();
    auto nestedContextWithCurrentRule = [&]() -> css::NestedContext
    {
      if (_groupRule->IsStyleRule())
      {
        // This is handled in CSSStyleRule.
        krys_debug_assert(false);
        return NestedParserContextType::Style;
      }

      if (_groupRule->IsScopeRule())
      {
        return NestedParserContextType::Scope;
      }

      // Find the context in the ancestor chain.
      return GetNestedContext();
    }();

    auto newRule =
      Parser::ParseRule(ruleString, GetParserContext(), styleSheet ? &styleSheet->Contents() : nullptr,
                        Parser::AllowedRules::ImportRules, nestedContextWithCurrentRule);
    if (!newRule)
    {
      // CSSNestedDeclarations parsing is allowed if there is an ancestor style rule or an ancestor scope
      // rule.
      if (!nestedContextWithCurrentRule)
      {
        return dom::ExceptionCode::SyntaxError;
      }

      newRule = Parser::ParseNestedDeclarations(GetParserContext(), ruleString);
      if (!newRule)
      {
        return dom::ExceptionCode::SyntaxError;
      }
    }

    if (newRule->IsImportRule() || newRule->IsNamespaceRule())
    {
      // FIXME: an HierarchyRequestError should also be thrown for a @charset.
      // They are currently not getting parsed, resulting in a SyntaxError
      // to get raised above.

      // HierarchyRequestError: Raised if the rule cannot be inserted at the specified
      // index, e.g., if an @import rule is inserted after a standard rule set or other
      // at-rule.
      return dom::ExceptionCode::HierarchyRequestError;
    }

    if (HasStyleRuleAncestor() && !newRule->IsStyleRule() && !newRule->IsGroupRule()
        && !newRule->IsNestedDeclarationsRule())
    {
      return dom::ExceptionCode::HierarchyRequestError;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);

    _groupRule->WrapperInsertRule(index, Ref<Rule>(krys::move(newRule)));

    _childRuleCSSOMWrappers.insert(_childRuleCSSOMWrappers.begin() + index, RefPtr<CSSRule>());
    return index;
  }

  dom::ExceptionOr<void> CSSGroupingRule::DeleteRule(uint32 index) noexcept
  {
    krys_debug_assert(_childRuleCSSOMWrappers.size() == _groupRule->ChildRules().size());

    if (index >= _groupRule->ChildRules().size())
    {
      // IndexSizeError: Raised if the specified index does not correspond to a
      // rule in the media rule list.
      return dom::ExceptionCode::IndexSizeError;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);

    _groupRule->WrapperRemoveRule(index);

    if (_childRuleCSSOMWrappers[index])
    {
      _childRuleCSSOMWrappers[index]->SetParentRule(nullptr);
    }
    _childRuleCSSOMWrappers.erase(_childRuleCSSOMWrappers.begin() + index);

    return {};
  }

  void CSSGroupingRule::AppendCSSTextForItemsInternal(StringBuilder &builder,
                                                      const StringBuilder &rules) noexcept
  {
    builder.Append(u8" {");
    if (rules.IsEmpty())
    {
      builder.Append(u8"\n}");
      return;
    }

    builder.Append(rules, u8"\n}");
  }

  void CSSGroupingRule::AppendCSSTextForItems(StringBuilder &builder) const noexcept
  {
    StringBuilder rules;
    CssTextForRules(rules);
    AppendCSSTextForItemsInternal(builder, rules);
  }

  void CSSGroupingRule::CssTextForRules(StringBuilder &rules) const noexcept
  {
    const auto &childRules = _groupRule->ChildRules();
    for (uint32 index = 0; index < childRules.size(); ++index)
    {
      auto ruleText = Item(index)->CssText();
      if (!ruleText.empty())
      {
        rules.Append(u8"\n  ", ruleText);
      }
    }
  }

  void CSSGroupingRule::AppendCSSTextWithReplacementURLsForItems(
    StringBuilder &builder, const SerialisationContext &context) const noexcept
  {
    StringBuilder rules;
    CssTextForRulesWithReplacementURLs(rules, context);
    AppendCSSTextForItemsInternal(builder, rules);
  }

  void CSSGroupingRule::CssTextForRulesWithReplacementURLs(StringBuilder &rules,
                                                           const SerialisationContext &context) const noexcept
  {
    const auto &childRules = _groupRule->ChildRules();
    for (uint32 index = 0; index < childRules.size(); index++)
    {
      auto *wrappedRule = Item(index);
      rules.Append(u8"\n  ", wrappedRule->CssText(context));
    }
  }

  RefPtr<StyleRuleWithNesting> CSSGroupingRule::PrepareChildStyleRuleForNesting(StyleRule &styleRule) noexcept
  {
    CSSStyleSheet::RuleMutationScope scope(this);
    auto &rules = _groupRule->_childRules;
    for (auto &rule : rules)
    {
      if (rule.get() == &styleRule)
      {
        auto styleRuleWithNesting = StyleRuleWithNesting::Create(krys::move(styleRule));
        rule = styleRuleWithNesting;
        return styleRuleWithNesting;
      }
    }
    return nullptr;
  }

  uint32 CSSGroupingRule::Length() const noexcept
  {
    return static_cast<uint32>(_groupRule->ChildRules().size());
  }

  CSSRule *CSSGroupingRule::Item(uint32 index) const noexcept
  {
    if (index >= Length())
    {
      return nullptr;
    }

    krys_debug_assert(_childRuleCSSOMWrappers.size() == _groupRule->ChildRules().size());
    auto &rule = _childRuleCSSOMWrappers[index];
    if (!rule)
    {
      rule = _groupRule->ChildRules()[index]->CreateCSSOMWrapper(const_cast<CSSGroupingRule &>(*this));
    }
    return rule.get();
  }

  CSSRuleList &CSSGroupingRule::CssRules() const noexcept
  {
    if (!_ruleListCSSOMWrapper)
    {
      _ruleListCSSOMWrapper =
        CreateUnique<LiveCSSRuleList<CSSGroupingRule>>(const_cast<CSSGroupingRule &>(*this));
    }
    return *_ruleListCSSOMWrapper;
  }

  void CSSGroupingRule::Reattach(Rule &rule) noexcept
  {
    _groupRule = ShareRef(Downcast<css::GroupRule>(rule));
    for (uint32 i = 0; i < _childRuleCSSOMWrappers.size(); ++i)
    {
      if (_childRuleCSSOMWrappers[i])
      {
        _childRuleCSSOMWrappers[i]->Reattach(*_groupRule->ChildRules()[i]);
      }
    }
  }
}
