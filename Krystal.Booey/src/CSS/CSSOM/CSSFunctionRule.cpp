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

#include "Krystal.Booey/CSS/CSSOM/CSSFunctionRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/Markup.hpp"
#include "Krystal.Booey/CSS/Rules/FunctionRule.hpp"
#include "Krystal.Booey/CSS/Values/VariableData.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  CSSFunctionRule::CSSFunctionRule(css::FunctionRule &rule, CSSStyleSheet *parent) noexcept
      : CSSGroupingRule(rule, parent)
  {
  }

  Ref<CSSFunctionRule> CSSFunctionRule::Create(css::FunctionRule &rule, CSSStyleSheet *parent) noexcept
  {
    return AdoptRef(*new CSSFunctionRule(rule, parent));
  }

  CSSOMString CSSFunctionRule::Name() const noexcept
  {
    return CSSOMString(GetFunctionRule().Name().View());
  }

  List<CSSFunctionRule::FunctionParameter> CSSFunctionRule::GetParameters() const noexcept
  {
    List<FunctionParameter> result;

    for (const auto &parameter : GetFunctionRule().Parameters())
    {
      const auto &defaultValue = parameter.DefaultValue;
      result.push_back({
        .Name = CSSOMString(parameter.Name.View()),
        .Type = parameter.Type.SyntaxString(),
        .DefaultValue = defaultValue ? Maybe<CSSOMString>(defaultValue->Serialise()) : null,
      });
    }

    return result;
  }

  CSSOMString CSSFunctionRule::ReturnType() const noexcept
  {
    return GetFunctionRule().ReturnType().SyntaxString();
  }

  CSSOMString CSSFunctionRule::CssText() const noexcept
  {
    StringBuilder builder;
    builder.Append(u8"@function ");
    SerialiseIdentifier(Name(), builder);
    builder.Append(u8'(');

    CSSOMStringView separator = u8"";
    for (const auto &parameter : GetFunctionRule().Parameters())
    {
      builder.Append(separator);
      SerialiseIdentifier(CSSOMString(parameter.Name.View()), builder);
      if (!parameter.Type.IsUniversal())
      {
        builder.Append(u8' ', parameter.Type.CSSTypeString());
      }
      if (const auto &defaultValue = parameter.DefaultValue)
      {
        builder.Append(u8": ", defaultValue->Serialise());
      }
      separator = u8", ";
    }

    builder.Append(u8')');
    if (const auto &returnType = GetFunctionRule().ReturnType(); !returnType.IsUniversal())
    {
      builder.Append(u8" returns ", returnType.CSSTypeString());
    }
    builder.Append(u8" { ");
    for (uint32 index = 0; index < Length(); ++index)
    {
      auto ruleText = Item(index)->CssText();
      builder.Append(ruleText, u8' ');
    }

    builder.Append(u8'}');
    return krys::move(builder).ToString();
  }

  const css::FunctionRule &CSSFunctionRule::GetFunctionRule() const noexcept
  {
    return Downcast<css::FunctionRule>(GetGroupRule());
  }
}
