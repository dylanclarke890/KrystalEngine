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
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  class CSSStyleProperties;
  class StyleRuleCSSStyleProperties;
}

namespace krys::boo::css
{
  class NestedDeclarationsRule;
}

namespace krys::boo
{
  /// @brief Declarations that follow nested rules, exposed to scripts.
  /// @see https://drafts.csswg.org/css-nesting/#the-cssnestrule
  class CSSNestedDeclarations final : public CSSRule
  {
  private:
    Ref<css::NestedDeclarationsRule> _styleRule;
    RefPtr<StyleRuleCSSStyleProperties> _propertiesCSSOMWrapper;

    CSSNestedDeclarations(css::NestedDeclarationsRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSNestedDeclarations() noexcept override;

    KRYS_NODISCARD static Ref<CSSNestedDeclarations> Create(css::NestedDeclarationsRule &rule,
                                                            CSSStyleSheet *sheet) noexcept;

    KRYS_NODISCARD CSSStyleProperties &Style() noexcept;

  private:
    KRYS_NODISCARD CSSOMString CssText() const noexcept final;

    void Reattach(css::Rule &rule) noexcept final;

    KRYS_NODISCARD css::RuleType StyleRuleType() const noexcept final
    {
      return css::RuleType::NestedDeclarations;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::CSSNestedDeclarations)
  KRYS_NODISCARD static bool IsType(const krys::boo::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::NestedDeclarations;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
