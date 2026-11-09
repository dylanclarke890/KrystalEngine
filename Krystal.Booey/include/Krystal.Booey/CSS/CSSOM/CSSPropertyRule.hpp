/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class PropertyRule;

  /// @brief An `@property` rule, exposed to scripts.
  /// @see https://drafts.css-houdini.org/css-properties-values-api/#the-css-property-rule-interface
  class CSSPropertyRule final : public CSSRule
  {
  private:
    Ref<css::PropertyRule> _propertyRule;

    CSSPropertyRule(css::PropertyRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSPropertyRule() noexcept;

    KRYS_NODISCARD static Ref<CSSPropertyRule> Create(css::PropertyRule &rule,
                                                      CSSStyleSheet *parent) noexcept;

    KRYS_NODISCARD CSSOMString Name() const noexcept;
    KRYS_NODISCARD CSSOMString Syntax() const noexcept;
    KRYS_NODISCARD bool Inherits() const noexcept;

    /// @brief The initial value, or null if the rule has none.
    KRYS_NODISCARD Maybe<CSSOMString> InitialValue() const noexcept;

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;

  private:
    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::Property;
    }

    void Reattach(Rule &rule) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSPropertyRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::Property;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
