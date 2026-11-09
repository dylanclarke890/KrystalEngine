/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/CSSOM/CSSLayerStatementRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSLayerBlockRule.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::css
{
  CSSLayerStatementRule::CSSLayerStatementRule(LayerRule &rule, CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _layerRule(ShareRef(rule))
  {
    krys_debug_assert(rule.IsStatement());
  }

  CSSLayerStatementRule::~CSSLayerStatementRule() noexcept = default;

  Ref<CSSLayerStatementRule> CSSLayerStatementRule::Create(LayerRule &rule, CSSStyleSheet *parent) noexcept
  {
    return AdoptRef(*new CSSLayerStatementRule(rule, parent));
  }

  CSSOMString CSSLayerStatementRule::CssText() const noexcept
  {
    CSSOMString result;
    result.append(u8"@layer ");

    auto nameList = NameList();
    for (const auto &name : nameList)
    {
      result.append(name);
      if (&name != &nameList.back())
      {
        result.append(u8", ");
      }
    }
    result.push_back(u8';');

    return result;
  }

  List<CSSOMString> CSSLayerStatementRule::NameList() const noexcept
  {
    List<CSSOMString> result;

    for (const auto &name : _layerRule->NameList())
    {
      result.push_back(StringFromCascadeLayerName(name));
    }

    return result;
  }

  void CSSLayerStatementRule::Reattach(Rule &rule) noexcept
  {
    _layerRule = ShareRef(Downcast<LayerRule>(rule));
  }
}
