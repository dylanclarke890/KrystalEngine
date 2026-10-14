/*
 * Copyright (C) 2007, 2008, 2012, 2014 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/CSS/Rules/KeyframesRule.hpp"
#include "CSSRule.h"
#include <memory>

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"

namespace krys::boo::css
{

  class CSSKeyframeRule;
  class CSSRuleList;
  class StyleRuleKeyframe;

  class CSSKeyframesRule final : public CSSRule
  {
  public:
    static Ref<CSSKeyframesRule> create(StyleRuleKeyframes &rule, CSSStyleSheet *sheet)
    {
      return adoptRef(*new CSSKeyframesRule(rule, sheet));
    }

    virtual ~CSSKeyframesRule();

    RuleType RuleType() const final
    {
      return RuleType::Keyframes;
    }
    String cssText() const final;
    void reattach(StyleRuleBase &) final;

    const CSSOMStringAtom &name() const
    {
      return m_keyframesRule->name();
    }
    void setName(const CSSOMStringAtom &);

    CSSRuleList &cssRules();

    void appendRule(const String &rule);
    void deleteRule(const String &key);
    CSSKeyframeRule *findRule(const String &key);

    // For IndexedGetter and CSSRuleList.
    unsigned length() const;
    CSSKeyframeRule *item(unsigned index) const;
    bool isSupportedPropertyIndex(unsigned index) const
    {
      return index < length();
    }

  private:
    CSSKeyframesRule(StyleRuleKeyframes &, CSSStyleSheet *parent);

    Ref<StyleRuleKeyframes> m_keyframesRule;
    mutable SmallList<RefPtr<CSSKeyframeRule>> m_childRuleCSSOMWrappers;
    const std::unique_ptr<CSSRuleList> m_ruleListCSSOMWrapper;
  };

} // namespace krys::boo::css

SPECIALIZE_TYPE_TRAITS_CSS_RULE(CSSKeyframesRule, RuleType::Keyframes)

