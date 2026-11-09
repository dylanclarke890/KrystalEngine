/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSStyleProperties;
  class NestedDeclarationsRule;
  class StyleRuleCSSStyleProperties;

  /// @brief Declarations that follow nested rules, exposed to scripts.
  /// @see https://drafts.csswg.org/css-nesting/#the-cssnestrule
  class CSSNestedDeclarations final : public CSSRule
  {
  private:
    Ref<NestedDeclarationsRule> _styleRule;
    RefPtr<StyleRuleCSSStyleProperties> _propertiesCSSOMWrapper;

    CSSNestedDeclarations(NestedDeclarationsRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSNestedDeclarations() noexcept;

    KRYS_NODISCARD static Ref<CSSNestedDeclarations> Create(NestedDeclarationsRule &rule,
                                                            CSSStyleSheet *sheet) noexcept;

    KRYS_NODISCARD CSSStyleProperties &Style() noexcept;

  private:
    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    void Reattach(Rule &rule) noexcept final;

    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::NestedDeclarations;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSNestedDeclarations)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::NestedDeclarations;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
