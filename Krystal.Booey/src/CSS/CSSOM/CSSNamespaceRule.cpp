/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/CSSOM/CSSNamespaceRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/Markup.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  CSSNamespaceRule::CSSNamespaceRule(NamespaceRule &namespaceRule, CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _namespaceRule(ShareRef(namespaceRule))
  {
  }

  CSSNamespaceRule::~CSSNamespaceRule() noexcept = default;

  Ref<CSSNamespaceRule> CSSNamespaceRule::Create(NamespaceRule &rule, CSSStyleSheet *sheet) noexcept
  {
    return AdoptRef(*new CSSNamespaceRule(rule, sheet));
  }

  CSSOMStringAtom CSSNamespaceRule::NamespaceURI() const noexcept
  {
    return _namespaceRule->Uri();
  }

  CSSOMStringAtom CSSNamespaceRule::Prefix() const noexcept
  {
    return _namespaceRule->Prefix();
  }

  CSSOMString CSSNamespaceRule::CssText() const noexcept
  {
    auto prefix = Prefix();
    auto prefixIsEmpty = prefix == CSSOMStringAtom::Null() || prefix.View().empty();

    StringBuilder result;
    result.Append(u8"@namespace ");
    if (!prefixIsEmpty)
    {
      SerialiseIdentifier(CSSOMString(prefix.View()), result);
    }
    result.Append(prefixIsEmpty ? u8"" : u8" ", u8"url(");
    result.Append(SerialiseString(CSSOMString(NamespaceURI().View())), u8");");
    return krys::move(result).ToString();
  }

  void CSSNamespaceRule::Reattach(Rule &) noexcept
  {
  }
}
