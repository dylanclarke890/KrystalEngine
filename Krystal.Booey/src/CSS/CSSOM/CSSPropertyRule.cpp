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

#include "Krystal.Booey/CSS/CSSOM/CSSPropertyRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/Markup.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Values/VariableData.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  CSSPropertyRule::CSSPropertyRule(css::PropertyRule &rule, CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _propertyRule(ShareRef(rule))
  {
  }

  CSSPropertyRule::~CSSPropertyRule() noexcept = default;

  Ref<CSSPropertyRule> CSSPropertyRule::Create(css::PropertyRule &rule, CSSStyleSheet *parent) noexcept
  {
    return AdoptRef(*new CSSPropertyRule(rule, parent));
  }

  CSSOMString CSSPropertyRule::Name() const noexcept
  {
    return CSSOMString(_propertyRule->GetDescriptor().Name.View());
  }

  CSSOMString CSSPropertyRule::Syntax() const noexcept
  {
    return _propertyRule->GetDescriptor().Syntax;
  }

  bool CSSPropertyRule::Inherits() const noexcept
  {
    return _propertyRule->GetDescriptor().Inherits.value_or(false);
  }

  Maybe<CSSOMString> CSSPropertyRule::InitialValue() const noexcept
  {
    if (!_propertyRule->GetDescriptor().InitialValue)
    {
      return null;
    }
    return _propertyRule->GetDescriptor().InitialValue->Serialise();
  }

  CSSOMString CSSPropertyRule::CssText() const noexcept
  {
    StringBuilder builder;
    const auto &descriptor = _propertyRule->GetDescriptor();

    builder.Append(u8"@property ");
    SerialiseIdentifier(CSSOMString(descriptor.Name.View()), builder);
    builder.Append(u8" { ");

    if (!descriptor.Syntax.empty())
    {
      builder.Append(u8"syntax: ");
      SerialiseString(Syntax(), builder);
      builder.Append(u8"; ");
    }

    if (descriptor.Inherits)
    {
      builder.Append(u8"inherits: ", *descriptor.Inherits ? u8"true" : u8"false", u8"; ");
    }

    if (descriptor.InitialValue)
    {
      builder.Append(u8"initial-value: ", *InitialValue(), u8"; ");
    }

    builder.Append(u8'}');
    return krys::move(builder).ToString();
  }

  void CSSPropertyRule::Reattach(Rule &rule) noexcept
  {
    _propertyRule = ShareRef(Downcast<css::PropertyRule>(rule));
  }
}
