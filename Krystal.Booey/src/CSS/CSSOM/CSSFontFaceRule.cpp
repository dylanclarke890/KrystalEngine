/*
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * (C) 2002-2003 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2002, 2005, 2006, 2008, 2012 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/CSSOM/CSSFontFaceRule.hpp"
#include "Krystal.Booey/CSS/Descriptors/CSSFontFaceDescriptors.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"

namespace krys::boo::css
{
  CSSFontFaceRule::CSSFontFaceRule(FontFaceRule &fontFaceRule, CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _fontFaceRule(ShareRef(fontFaceRule))
  {
  }

  CSSFontFaceRule::~CSSFontFaceRule() noexcept
  {
    if (_propertiesCSSOMWrapper)
    {
      _propertiesCSSOMWrapper->ClearParentRule();
    }
  }

  Ref<CSSFontFaceRule> CSSFontFaceRule::Create(FontFaceRule &rule, CSSStyleSheet *sheet) noexcept
  {
    return AdoptRef(*new CSSFontFaceRule(rule, sheet));
  }

  CSSFontFaceDescriptors &CSSFontFaceRule::Style() noexcept
  {
    if (!_propertiesCSSOMWrapper)
    {
      _propertiesCSSOMWrapper = CSSFontFaceDescriptors::Create(_fontFaceRule->MutableProperties(), *this);
    }
    return *_propertiesCSSOMWrapper;
  }

  CSSOMString CSSFontFaceRule::CssText() const noexcept
  {
    return CssTextInternal(_fontFaceRule->Properties().AsText(DefaultSerialisationContext()));
  }

  CSSOMString CSSFontFaceRule::CssText(const SerialisationContext &context) const noexcept
  {
    return CssTextInternal(_fontFaceRule->Properties().AsText(context));
  }

  CSSOMString CSSFontFaceRule::CssTextInternal(const CSSOMString &declarations) noexcept
  {
    if (declarations.empty())
    {
      return u8"@font-face { }";
    }

    CSSOMString result;
    result.append(u8"@font-face { ").append(declarations).append(u8" }");
    return result;
  }

  void CSSFontFaceRule::Reattach(Rule &rule) noexcept
  {
    _fontFaceRule = ShareRef(Downcast<FontFaceRule>(rule));
    if (_propertiesCSSOMWrapper)
    {
      _propertiesCSSOMWrapper->Reattach(_fontFaceRule->MutableProperties());
    }
  }
}
