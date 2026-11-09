/*
 * Copyright (C) 2024-2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
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

#include "Krystal.Booey/CSS/CSSOM/CSSPositionTryRule.hpp"
#include "Krystal.Booey/CSS/Descriptors/CSSPositionTryDescriptors.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/PositionTryRule.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  CSSPositionTryRule::CSSPositionTryRule(css::PositionTryRule &rule, CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _positionTryRule(ShareRef(rule))
  {
  }

  CSSPositionTryRule::~CSSPositionTryRule() noexcept = default;

  Ref<CSSPositionTryRule> CSSPositionTryRule::Create(css::PositionTryRule &rule,
                                                     CSSStyleSheet *parent) noexcept
  {
    return AdoptRef(*new CSSPositionTryRule(rule, parent));
  }

  Ref<css::PositionTryRule> CSSPositionTryRule::ProtectedPositionTryRule() const noexcept
  {
    return _positionTryRule;
  }

  CSSOMString CSSPositionTryRule::CssText() const noexcept
  {
    StringBuilder builder;
    builder.Append(u8"@position-try ", Name().View(), u8" {");

    auto properties = _positionTryRule->ProtectedProperties();
    if (auto declarations = properties->AsText(DefaultSerialisationContext()); !declarations.empty())
    {
      builder.Append(u8" ", declarations, u8' ');
    }
    else
    {
      builder.Append(u8' ');
    }

    builder.Append(u8'}');
    return krys::move(builder).ToString();
  }

  void CSSPositionTryRule::Reattach(Rule &rule) noexcept
  {
    _positionTryRule = ShareRef(Downcast<css::PositionTryRule>(rule));
    if (auto propertiesCSSOMWrapper = _propertiesCSSOMWrapper)
    {
      propertiesCSSOMWrapper->Reattach(*ProtectedPositionTryRule()->ProtectedMutableProperties());
    }
  }

  CSSOMStringAtom CSSPositionTryRule::Name() const noexcept
  {
    return _positionTryRule->Name();
  }

  CSSPositionTryDescriptors &CSSPositionTryRule::Style() noexcept
  {
    auto mutableProperties = ShareRef(ProtectedPositionTryRule()->MutableProperties());
    if (!_propertiesCSSOMWrapper)
    {
      _propertiesCSSOMWrapper = CSSPositionTryDescriptors::Create(*mutableProperties, *this);
    }
    return *_propertiesCSSOMWrapper;
  }
}
