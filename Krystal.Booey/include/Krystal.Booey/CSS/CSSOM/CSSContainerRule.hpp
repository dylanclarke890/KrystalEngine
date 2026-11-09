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

#include "Krystal.Booey/CSS/CSSOM/CSSConditionRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::css
{
  class ContainerRule;

  /// @see https://drafts.csswg.org/css-conditional-5/#dictdef-csscontainercondition
  struct CSSContainerCondition
  {
    CSSOMString Name;
    CSSOMString Query;
  };

  /// @brief An `@container` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-conditional-5/#the-csscontainerrule-interface
  class CSSContainerRule final : public CSSConditionRule
  {
  private:
    CSSContainerRule(css::ContainerRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    KRYS_NODISCARD static Ref<CSSContainerRule> Create(css::ContainerRule &rule,
                                                       CSSStyleSheet *parent) noexcept;

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    KRYS_NODISCARD CSSOMString ConditionText() const noexcept final;
    /// @brief The name of the rule's only condition, or empty if it has several.
    /// @see https://drafts.csswg.org/css-conditional-5/#dom-csscontainerrule-containername
    KRYS_NODISCARD CSSOMString ContainerName() const noexcept;
    /// @brief The query of the rule's only condition, or empty if it has several.
    /// @see https://drafts.csswg.org/css-conditional-5/#dom-csscontainerrule-containerquery
    KRYS_NODISCARD CSSOMString ContainerQuery() const noexcept;
    /// @see https://drafts.csswg.org/css-conditional-5/#dom-csscontainerrule-conditions
    KRYS_NODISCARD List<CSSContainerCondition> Conditions() const noexcept;

  private:
    KRYS_NODISCARD const css::ContainerRule &GetContainerRule() const noexcept;

    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::Container;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSContainerRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::Container;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
