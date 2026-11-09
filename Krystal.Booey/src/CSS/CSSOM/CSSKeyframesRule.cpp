/*
 * Copyright (C) 2007-2024 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/CSSOM/CSSKeyframesRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSKeyframeRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSRuleList.hpp"
#include "Krystal.Booey/CSS/Parser/Parser.hpp"
#include "Krystal.Booey/CSS/Rules/KeyframeRule.hpp"
#include "Krystal.Booey/CSS/Rules/KeyframesRule.hpp"
#include "Krystal.Booey/CSS/StyleSheets/CSSStyleSheet.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::css
{
  CSSKeyframesRule::CSSKeyframesRule(KeyframesRule &keyframesRule, CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _keyframesRule(ShareRef(keyframesRule)),
        _childRuleCSSOMWrappers(keyframesRule.Keyframes().size())
  {
  }

  CSSKeyframesRule::~CSSKeyframesRule() noexcept
  {
    krys_debug_assert(_childRuleCSSOMWrappers.size() == _keyframesRule->Keyframes().size());

    for (auto &wrapper : _childRuleCSSOMWrappers)
    {
      if (wrapper)
      {
        wrapper->SetParentRule(nullptr);
      }
    }
  }

  Ref<CSSKeyframesRule> CSSKeyframesRule::Create(KeyframesRule &rule, CSSStyleSheet *sheet) noexcept
  {
    return AdoptRef(*new CSSKeyframesRule(rule, sheet));
  }

  const CSSOMStringAtom &CSSKeyframesRule::Name() const noexcept
  {
    return _keyframesRule->Name();
  }

  void CSSKeyframesRule::SetName(const CSSOMStringAtom &name) noexcept
  {
    CSSStyleSheet::RuleMutationScope mutationScope(this);

    _keyframesRule->SetName(name);
  }

  void CSSKeyframesRule::AppendRule(const CSSOMString &ruleText) noexcept
  {
    krys_debug_assert(_childRuleCSSOMWrappers.size() == _keyframesRule->Keyframes().size());

    auto keyframe = Parser::ParseKeyframeRule(ruleText, GetParserContext());
    if (!keyframe)
    {
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);

    _keyframesRule->WrapperAppendKeyframe(krys::move(keyframe));

    _childRuleCSSOMWrappers.resize(Length());
  }

  void CSSKeyframesRule::DeleteRule(const CSSOMString &s) noexcept
  {
    krys_debug_assert(_childRuleCSSOMWrappers.size() == _keyframesRule->Keyframes().size());

    auto i = _keyframesRule->FindKeyframeIndex(s);
    if (!i)
    {
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);

    _keyframesRule->WrapperRemoveKeyframe(static_cast<uint32>(*i));

    if (_childRuleCSSOMWrappers[*i])
    {
      _childRuleCSSOMWrappers[*i]->SetParentRule(nullptr);
    }
    _childRuleCSSOMWrappers.erase(_childRuleCSSOMWrappers.begin() + *i);
  }

  CSSKeyframeRule *CSSKeyframesRule::FindRule(const CSSOMString &s) noexcept
  {
    auto i = _keyframesRule->FindKeyframeIndex(s);
    return i ? Item(static_cast<uint32>(*i)) : nullptr;
  }

  CSSOMString CSSKeyframesRule::CssText() const noexcept
  {
    CSSOMString result;
    result.append(u8"@keyframes ").append(Name().View()).append(u8" { \n");
    for (uint32 i = 0, size = Length(); i < size; ++i)
    {
      result.append(u8"  ").append(_keyframesRule->Keyframes()[i]->CssText()).push_back(u8'\n');
    }
    result.push_back(u8'}');
    return result;
  }

  uint32 CSSKeyframesRule::Length() const noexcept
  {
    return static_cast<uint32>(_keyframesRule->Keyframes().size());
  }

  CSSKeyframeRule *CSSKeyframesRule::Item(uint32 index) const noexcept
  {
    if (index >= Length())
    {
      return nullptr;
    }

    krys_debug_assert(_childRuleCSSOMWrappers.size() == _keyframesRule->Keyframes().size());
    auto &rule = _childRuleCSSOMWrappers[index];
    if (!rule)
    {
      rule = AdoptRef(
        *new CSSKeyframeRule(*_keyframesRule->Keyframes()[index], const_cast<CSSKeyframesRule *>(this)));
    }
    return rule.get();
  }

  CSSRuleList &CSSKeyframesRule::CssRules() noexcept
  {
    if (!_ruleListCSSOMWrapper)
    {
      _ruleListCSSOMWrapper = CreateUnique<LiveCSSRuleList<CSSKeyframesRule>>(*this);
    }
    return *_ruleListCSSOMWrapper;
  }

  void CSSKeyframesRule::Reattach(Rule &rule) noexcept
  {
    _keyframesRule = ShareRef(Downcast<KeyframesRule>(rule));
  }
}
