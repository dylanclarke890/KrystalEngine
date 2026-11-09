/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/CSSOM/CSSScopeRule.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  CSSScopeRule::CSSScopeRule(css::ScopeRule &rule, CSSStyleSheet *parent) noexcept
      : CSSGroupingRule(rule, parent)
  {
  }

  Ref<CSSScopeRule> CSSScopeRule::Create(css::ScopeRule &rule, CSSStyleSheet *parent) noexcept
  {
    return AdoptRef(*new CSSScopeRule(rule, parent));
  }

  const css::ScopeRule &CSSScopeRule::GetScopeRule() const noexcept
  {
    return Downcast<css::ScopeRule>(GetGroupRule());
  }

  CSSOMString CSSScopeRule::CssText() const noexcept
  {
    StringBuilder builder;
    builder.Append(u8"@scope");

    auto start = Start();
    if (!start.empty())
    {
      builder.Append(u8" (", start, u8')');
    }

    auto end = End();
    if (!end.empty())
    {
      builder.Append(u8" to ", u8"(", end, u8')');
    }

    AppendCSSTextForItems(builder);
    return krys::move(builder).ToString();
  }

  CSSOMString CSSScopeRule::Start() const noexcept
  {
    const auto &scope = GetScopeRule().OriginalScopeStart();
    if (scope.IsEmpty())
    {
      return {};
    }
    return scope.SelectorsText();
  }

  CSSOMString CSSScopeRule::End() const noexcept
  {
    const auto &scope = GetScopeRule().OriginalScopeEnd();
    if (scope.IsEmpty())
    {
      return {};
    }
    return scope.SelectorsText();
  }
}
