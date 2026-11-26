/*
 * Copyright (C) 2012 Motorola Mobility Inc. All rights reserved.
 * Copyright (C) 2020 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Motorola Mobility Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Web/Css/CSSOM/CSSConditionRule.hpp"

namespace krys::web::css
{
  class SupportsRule;
}

namespace krys::web
{
  /// @brief An `@supports` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-conditional/#the-csssupportsrule-interface
  class CSSSupportsRule final : public CSSConditionRule
  {
  private:
    CSSSupportsRule(css::SupportsRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    KRYS_NODISCARD static Ref<CSSSupportsRule> Create(css::SupportsRule &rule,
                                                      CSSStyleSheet *parent) noexcept;

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;

    KRYS_NODISCARD CSSOMString CssText(const css::SerialisationContext &context) const noexcept final;

    KRYS_NODISCARD CSSOMString ConditionText() const noexcept final;

  private:
    KRYS_NODISCARD css::RuleType StyleRuleType() const noexcept final
    {
      return css::RuleType::Supports;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::CSSSupportsRule)
  KRYS_NODISCARD static bool IsType(const krys::web::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::web::css::RuleType::Supports;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
