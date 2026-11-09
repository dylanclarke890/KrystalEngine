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

#include "Krystal.Booey/CSS/CSSOM/CSSLayerBlockRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/Markup.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  CSSLayerBlockRule::CSSLayerBlockRule(LayerRule &rule, CSSStyleSheet *parent) noexcept
      : CSSGroupingRule(rule, parent)
  {
    krys_debug_assert(!rule.IsStatement());
  }

  Ref<CSSLayerBlockRule> CSSLayerBlockRule::Create(LayerRule &rule, CSSStyleSheet *parent) noexcept
  {
    return AdoptRef(*new CSSLayerBlockRule(rule, parent));
  }

  CSSOMString CSSLayerBlockRule::CssText() const noexcept
  {
    StringBuilder builder;
    builder.Append(u8"@layer");
    if (auto name = Name(); !name.empty())
    {
      builder.Append(u8" ", name);
    }
    AppendCSSTextForItems(builder);
    return krys::move(builder).ToString();
  }

  CSSOMString CSSLayerBlockRule::Name() const noexcept
  {
    const auto &layer = Downcast<LayerRule>(GetGroupRule());

    if (layer.Name().empty())
    {
      return {};
    }

    return StringFromCascadeLayerName(layer.Name());
  }

  CSSOMString StringFromCascadeLayerName(const CascadeLayerName &name) noexcept
  {
    StringBuilder result;
    for (const auto &segment : name)
    {
      SerialiseIdentifier(CSSOMString(segment.View()), result);
      if (&segment != &name.back())
      {
        result.Append(u8'.');
      }
    }
    return krys::move(result).ToString();
  }
}
