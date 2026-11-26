/*
 * Copyright (C) 2011 Adobe Systems Incorporated. All rights reserved.
 * Copyright (C) 2012-2024 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "Krystal.Booey/Css/CSSOM/CSSGroupingRule.hpp"
#include "Krystal.Booey/Css/CSSOM/CSSRuleList.hpp"
#include "Krystal.Booey/Css/CSSOM/CSSStyleSheet.hpp"
#include "Krystal.Booey/Css/Parser/Parser.hpp"
#include "Krystal.Booey/Css/Rules/Rule.hpp"
#include "Krystal.Booey/Css/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Booey/Dom/Types/ExceptionOr.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"

namespace krys::boo
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

  ExceptionOr<uint32> CSSGroupingRule::InsertRule(const CSSOMString &ruleString, uint32 index) noexcept
  {
    krys_debug_assert(_childRuleCSSOMWrappers.size() == _groupRule->ChildRules().size());

    if (index > _groupRule->ChildRules().size())
    {
      // IndexSizeError: Raised if the specified index is not a valid insertion point.
      return ExceptionCode::IndexSizeError;
    }

    auto *styleSheet = ParentStyleSheet();
    auto nestedContextWithCurrentRule = [&]() -> css::NestedContext
    {
      if (_groupRule->IsStyleRule())
      {
        // This is handled in CSSStyleRule.
        krys_debug_assert(false);
        return css::NestedParserContextType::Style;
      }

      if (_groupRule->IsScopeRule())
      {
        return css::NestedParserContextType::Scope;
      }

      // Find the context in the ancestor chain.
      return GetNestedContext();
    }();

    auto newRule =
      css::Parser::ParseRule(ruleString, GetParserContext(), styleSheet ? &styleSheet->Contents() : nullptr,
                             css::Parser::AllowedRules::ImportRules, nestedContextWithCurrentRule);
    if (!newRule)
    {
      // CSSNestedDeclarations parsing is allowed if there is an ancestor style rule or an ancestor scope
      // rule.
      if (!nestedContextWithCurrentRule)
      {
        return ExceptionCode::SyntaxError;
      }

      newRule = css::Parser::ParseNestedDeclarations(GetParserContext(), ruleString);
      if (!newRule)
      {
        return ExceptionCode::SyntaxError;
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
      return ExceptionCode::HierarchyRequestError;
    }

    if (HasStyleRuleAncestor() && !newRule->IsStyleRule() && !newRule->IsGroupRule()
        && !newRule->IsNestedDeclarationsRule())
    {
      return ExceptionCode::HierarchyRequestError;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);

    _groupRule->WrapperInsertRule(index, krys::move(newRule));

    _childRuleCSSOMWrappers.insert(_childRuleCSSOMWrappers.begin() + index, RefPtr<CSSRule>());
    return index;
  }

  ExceptionOr<void> CSSGroupingRule::DeleteRule(uint32 index) noexcept
  {
    krys_debug_assert(_childRuleCSSOMWrappers.size() == _groupRule->ChildRules().size());

    if (index >= _groupRule->ChildRules().size())
    {
      // IndexSizeError: Raised if the specified index does not correspond to a
      // rule in the media rule list.
      return ExceptionCode::IndexSizeError;
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

  void CSSGroupingRule::AppendCSSTextWithReplacementUrlsForItems(
    StringBuilder &builder, const css::SerialisationContext &context) const noexcept
  {
    StringBuilder rules;
    CssTextForRulesWithReplacementUrls(rules, context);
    AppendCSSTextForItemsInternal(builder, rules);
  }

  void CSSGroupingRule::CssTextForRulesWithReplacementUrls(
    StringBuilder &rules, const css::SerialisationContext &context) const noexcept
  {
    const auto &childRules = _groupRule->ChildRules();
    for (uint32 index = 0; index < childRules.size(); index++)
    {
      auto *wrappedRule = Item(index);
      rules.Append(u8"\n  ", wrappedRule->CssText(context));
    }
  }

  RefPtr<css::StyleRuleWithNesting>
    CSSGroupingRule::PrepareChildStyleRuleForNesting(css::StyleRule &styleRule) noexcept
  {
    CSSStyleSheet::RuleMutationScope scope(this);
    auto &rules = _groupRule->_childRules;
    for (auto &rule : rules)
    {
      if (rule.get() == &styleRule)
      {
        auto styleRuleWithNesting = css::StyleRuleWithNesting::Create(krys::move(styleRule));
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

  void CSSGroupingRule::Reattach(css::Rule &rule) noexcept
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
