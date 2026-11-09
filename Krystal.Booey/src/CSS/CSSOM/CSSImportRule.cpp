/*
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * (C) 2002-2003 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2002, 2005, 2006, 2008, 2009, 2010, 2012, 2013 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/CSSOM/CSSImportRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSLayerBlockRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/Markup.hpp"
#include "Krystal.Booey/CSS/Media/MediaList.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/Rules/ImportRule.hpp"
#include "Krystal.Booey/CSS/StyleSheets/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  CSSImportRule::CSSImportRule(ImportRule &importRule, CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _importRule(ShareRef(importRule))
  {
  }

  CSSImportRule::~CSSImportRule() noexcept
  {
    if (_styleSheetCSSOMWrapper)
    {
      _styleSheetCSSOMWrapper->ClearOwnerRule();
    }
    if (_mediaCSSOMWrapper)
    {
      _mediaCSSOMWrapper->DetachFromParent();
    }
  }

  Ref<CSSImportRule> CSSImportRule::Create(ImportRule &rule, CSSStyleSheet *sheet) noexcept
  {
    return AdoptRef(*new CSSImportRule(rule, sheet));
  }

  CSSOMString CSSImportRule::Href() const noexcept
  {
    return _importRule->Href();
  }

  MediaList &CSSImportRule::Media() const noexcept
  {
    if (!_mediaCSSOMWrapper)
    {
      _mediaCSSOMWrapper = MediaList::Create(const_cast<CSSImportRule *>(this));
    }
    return *_mediaCSSOMWrapper;
  }

  Maybe<CSSOMString> CSSImportRule::LayerName() const noexcept
  {
    const auto &name = _importRule->GetCascadeLayerName();
    if (!name)
    {
      return null;
    }
    return StringFromCascadeLayerName(*name);
  }

  CSSOMString CSSImportRule::SupportsText() const noexcept
  {
    return _importRule->SupportsText();
  }

  CSSOMString CSSImportRule::CssTextInternal(const CSSOMString &urlString) const noexcept
  {
    StringBuilder builder;
    builder.Append(u8"@import ", SerialiseURL(urlString));

    if (auto layerName = LayerName())
    {
      if (layerName->empty())
      {
        builder.Append(u8" layer");
      }
      else
      {
        builder.Append(u8" layer(", *layerName, u8')');
      }
    }

    auto supports = SupportsText();
    if (!supports.empty())
    {
      builder.Append(u8" supports(", supports, u8')');
    }

    if (!MediaQueries().empty())
    {
      builder.Append(u8' ');
      mq::Serialise(builder, MediaQueries());
    }

    builder.Append(u8';');
    return krys::move(builder).ToString();
  }

  CSSOMString CSSImportRule::CssText() const noexcept
  {
    return CssTextInternal(_importRule->Href());
  }

  CSSStyleSheet *CSSImportRule::GetStyleSheet() const noexcept
  {
    if (!_importRule->StyleSheet())
    {
      return nullptr;
    }

    Maybe<bool> isOriginClean;

    if (!_styleSheetCSSOMWrapper)
    {
      _styleSheetCSSOMWrapper = CSSStyleSheet::Create(ShareRef(*_importRule->StyleSheet()),
                                                      const_cast<CSSImportRule *>(this), isOriginClean);
    }
    return _styleSheetCSSOMWrapper.get();
  }

  RefPtr<CSSStyleSheet> CSSImportRule::ProtectedStyleSheet() const noexcept
  {
    return ShareRefPtr(GetStyleSheet());
  }

  void CSSImportRule::Reattach(Rule &) noexcept
  {
    // FIXME: Implement when enabling caching for stylesheets with import rules.
    krys_debug_assert(false);
  }

  const mq::MediaQueryList &CSSImportRule::MediaQueries() const noexcept
  {
    return _importRule->MediaQueries();
  }

  void CSSImportRule::SetMediaQueries(mq::MediaQueryList &&queries) noexcept
  {
    _importRule->SetMediaQueries(krys::move(queries));
  }

  void CSSImportRule::GetChildStyleSheets(HashSet<RefPtr<CSSStyleSheet>> &childStyleSheets) noexcept
  {
    auto sheet = ProtectedStyleSheet();
    if (!sheet)
    {
      return;
    }

    if (childStyleSheets.insert(sheet).second)
    {
      sheet->GetChildStyleSheets(childStyleSheets);
    }
  }
}
