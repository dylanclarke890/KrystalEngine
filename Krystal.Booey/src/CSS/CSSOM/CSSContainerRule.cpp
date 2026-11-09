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

#include "Krystal.Booey/CSS/CSSOM/CSSContainerRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/Markup.hpp"
#include "Krystal.Booey/CSS/Queries/GenericMediaQuerySerialisation.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  CSSContainerRule::CSSContainerRule(css::ContainerRule &rule, CSSStyleSheet *parent) noexcept
      : CSSConditionRule(rule, parent)
  {
  }

  Ref<CSSContainerRule> CSSContainerRule::Create(css::ContainerRule &rule, CSSStyleSheet *parent) noexcept
  {
    return AdoptRef(*new CSSContainerRule(rule, parent));
  }

  const css::ContainerRule &CSSContainerRule::GetContainerRule() const noexcept
  {
    return Downcast<css::ContainerRule>(GetGroupRule());
  }

  CSSOMString CSSContainerRule::CssText() const noexcept
  {
    StringBuilder builder;
    builder.Append(u8"@container ");
    cq::Serialise(builder, GetContainerRule().ContainerQueries());
    AppendCSSTextForItems(builder);
    return krys::move(builder).ToString();
  }

  CSSOMString CSSContainerRule::ConditionText() const noexcept
  {
    StringBuilder builder;
    cq::Serialise(builder, GetContainerRule().ContainerQueries());
    return krys::move(builder).ToString();
  }

  CSSOMString CSSContainerRule::ContainerName() const noexcept
  {
    // If conditions has exactly one item, return that item's name; otherwise the empty string.
    auto conditions = Conditions();
    return conditions.size() == 1 ? krys::move(conditions.front().Name) : CSSOMString();
  }

  CSSOMString CSSContainerRule::ContainerQuery() const noexcept
  {
    // If conditions has exactly one item, return that item's query; otherwise the empty string.
    auto conditions = Conditions();
    return conditions.size() == 1 ? krys::move(conditions.front().Query) : CSSOMString();
  }

  List<CSSContainerCondition> CSSContainerRule::Conditions() const noexcept
  {
    List<CSSContainerCondition> conditions;
    for (const auto &query : GetContainerRule().ContainerQueries())
    {
      StringBuilder name;
      if (query.Name != CSSOMStringAtom::Null() && !query.Name.View().empty())
      {
        SerialiseIdentifier(CSSOMString(query.Name.View()), name);
      }
      StringBuilder condition;
      mq::Serialise(condition, query.Condition);
      conditions.push_back({krys::move(name).ToString(), krys::move(condition).ToString()});
    }
    return conditions;
  }
}
