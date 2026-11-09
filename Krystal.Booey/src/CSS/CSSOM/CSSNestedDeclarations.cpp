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

#include "Krystal.Booey/CSS/CSSOM/CSSNestedDeclarations.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"

namespace krys::boo::css
{
  CSSNestedDeclarations::CSSNestedDeclarations(NestedDeclarationsRule &rule, CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _styleRule(ShareRef(rule))
  {
  }

  CSSNestedDeclarations::~CSSNestedDeclarations() noexcept = default;

  Ref<CSSNestedDeclarations> CSSNestedDeclarations::Create(NestedDeclarationsRule &rule,
                                                           CSSStyleSheet *sheet) noexcept
  {
    return AdoptRef(*new CSSNestedDeclarations(rule, sheet));
  }

  CSSStyleProperties &CSSNestedDeclarations::Style() noexcept
  {
    if (!_propertiesCSSOMWrapper)
    {
      _propertiesCSSOMWrapper = StyleRuleCSSStyleProperties::Create(_styleRule->MutableProperties(), *this);
    }
    return *_propertiesCSSOMWrapper;
  }

  CSSOMString CSSNestedDeclarations::CssText() const noexcept
  {
    return _styleRule->Properties().AsText(DefaultSerialisationContext());
  }

  void CSSNestedDeclarations::Reattach(Rule &rule) noexcept
  {
    _styleRule = ShareRef(Downcast<NestedDeclarationsRule>(rule));
    if (_propertiesCSSOMWrapper)
    {
      _propertiesCSSOMWrapper->Reattach(_styleRule->MutableProperties());
    }
  }
}
