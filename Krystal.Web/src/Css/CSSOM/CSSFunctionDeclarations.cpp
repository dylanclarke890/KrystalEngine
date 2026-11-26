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

#include "Krystal.Web/Css/CSSOM/CSSFunctionDeclarations.hpp"
#include "Krystal.Web/Css/CSSOM/CSSFunctionDescriptors.hpp"
#include "Krystal.Web/Css/Properties/MutableStyleProperties.hpp"
#include "Krystal.Web/Css/Properties/StyleProperties.hpp"
#include "Krystal.Web/Css/Rules/FunctionRule.hpp"
#include "Krystal.Web/Css/Values/SerialisationContext.hpp"

namespace krys::web
{
  CSSFunctionDeclarations::CSSFunctionDeclarations(css::FunctionDeclarationsRule &rule,
                                                   CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _styleRule(ShareRef(rule))
  {
  }

  CSSFunctionDeclarations::~CSSFunctionDeclarations() noexcept = default;

  Ref<CSSFunctionDeclarations> CSSFunctionDeclarations::Create(css::FunctionDeclarationsRule &rule,
                                                               CSSStyleSheet *sheet) noexcept
  {
    return AdoptRef(*new CSSFunctionDeclarations(rule, sheet));
  }

  CSSFunctionDescriptors &CSSFunctionDeclarations::Style() noexcept
  {
    if (!_descriptorsCSSOMWrapper)
    {
      auto properties = ShareRef(_styleRule->MutableProperties());
      _descriptorsCSSOMWrapper = CSSFunctionDescriptors::Create(*properties, *this);
    }
    return *_descriptorsCSSOMWrapper;
  }

  CSSOMString CSSFunctionDeclarations::CssText() const noexcept
  {
    return _styleRule->Properties().AsText(css::DefaultSerialisationContext());
  }

  void CSSFunctionDeclarations::Reattach(css::Rule &rule) noexcept
  {
    _styleRule = ShareRef(Downcast<css::FunctionDeclarationsRule>(rule));
    if (_descriptorsCSSOMWrapper)
    {
      auto properties = ShareRef(_styleRule->MutableProperties());
      _descriptorsCSSOMWrapper->Reattach(*properties);
    }
  }
}
