/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  class CSSFunctionDescriptors;
}

namespace krys::boo::css
{
  class FunctionDeclarationsRule;
}

namespace krys::boo
{
  /// @brief The declarations of an `@function` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-mixins/#cssfunctiondeclarations
  class CSSFunctionDeclarations final : public CSSRule
  {
  private:
    Ref<css::FunctionDeclarationsRule> _styleRule;
    RefPtr<CSSFunctionDescriptors> _descriptorsCSSOMWrapper;

    CSSFunctionDeclarations(css::FunctionDeclarationsRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSFunctionDeclarations() noexcept override;

    KRYS_NODISCARD static Ref<CSSFunctionDeclarations> Create(css::FunctionDeclarationsRule &rule,
                                                              CSSStyleSheet *sheet) noexcept;

    KRYS_NODISCARD CSSFunctionDescriptors &Style() noexcept;

  private:
    KRYS_NODISCARD CSSOMString CssText() const noexcept final;

    void Reattach(css::Rule &rule) noexcept final;

    KRYS_NODISCARD css::RuleType StyleRuleType() const noexcept final
    {
      return css::RuleType::FunctionDeclarations;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::CSSFunctionDeclarations)
  KRYS_NODISCARD static bool IsType(const krys::boo::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::FunctionDeclarations;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
