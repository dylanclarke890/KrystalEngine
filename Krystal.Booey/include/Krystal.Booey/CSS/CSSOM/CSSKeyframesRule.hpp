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

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::css
{
  class CSSKeyframeRule;
  class CSSRuleList;
  class KeyframesRule;

  /// @brief An `@keyframes` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-animations/#interface-csskeyframesrule
  class CSSKeyframesRule final : public CSSRule
  {
  private:
    Ref<KeyframesRule> _keyframesRule;
    mutable List<RefPtr<CSSKeyframeRule>> _childRuleCSSOMWrappers;
    mutable UniquePtr<CSSRuleList> _ruleListCSSOMWrapper;

    CSSKeyframesRule(KeyframesRule &keyframesRule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSKeyframesRule() noexcept;

    KRYS_NODISCARD static Ref<CSSKeyframesRule> Create(KeyframesRule &rule, CSSStyleSheet *sheet) noexcept;

    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::Keyframes;
    }

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    void Reattach(Rule &rule) noexcept final;

    KRYS_NODISCARD const CSSOMStringAtom &Name() const noexcept;
    void SetName(const CSSOMStringAtom &name) noexcept;

    KRYS_NODISCARD CSSRuleList &CssRules() noexcept;

    void AppendRule(const CSSOMString &rule) noexcept;
    void DeleteRule(const CSSOMString &key) noexcept;
    KRYS_NODISCARD CSSKeyframeRule *FindRule(const CSSOMString &key) noexcept;

    // For IndexedGetter and CSSRuleList.
    KRYS_NODISCARD uint32 Length() const noexcept;
    KRYS_NODISCARD CSSKeyframeRule *Item(uint32 index) const noexcept;

    KRYS_NODISCARD bool IsSupportedPropertyIndex(uint32 index) const noexcept
    {
      return index < Length();
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSKeyframesRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::Keyframes;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
