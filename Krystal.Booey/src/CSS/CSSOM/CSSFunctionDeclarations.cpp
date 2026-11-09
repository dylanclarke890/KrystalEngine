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

#include "Krystal.Booey/CSS/CSSOM/CSSFunctionDeclarations.hpp"
#include "Krystal.Booey/CSS/Descriptors/CSSFunctionDescriptors.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/FunctionRule.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"

namespace krys::boo::css
{
  CSSFunctionDeclarations::CSSFunctionDeclarations(FunctionDeclarationsRule &rule,
                                                   CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _styleRule(ShareRef(rule))
  {
  }

  CSSFunctionDeclarations::~CSSFunctionDeclarations() noexcept = default;

  Ref<CSSFunctionDeclarations> CSSFunctionDeclarations::Create(FunctionDeclarationsRule &rule,
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
    return _styleRule->Properties().AsText(DefaultSerialisationContext());
  }

  void CSSFunctionDeclarations::Reattach(Rule &rule) noexcept
  {
    _styleRule = ShareRef(Downcast<FunctionDeclarationsRule>(rule));
    if (_descriptorsCSSOMWrapper)
    {
      auto properties = ShareRef(_styleRule->MutableProperties());
      _descriptorsCSSOMWrapper->Reattach(*properties);
    }
  }
}
