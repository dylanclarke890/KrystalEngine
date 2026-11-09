/**
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * (C) 2002-2003 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2002-2020 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Samuel Weinig (sam@webkit.org)
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

#include "Krystal.Booey/CSS/CSSOM/CSSMediaRule.hpp"
#include "Krystal.Booey/CSS/Media/MediaList.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  CSSMediaRule::CSSMediaRule(MediaRule &mediaRule, CSSStyleSheet *parent) noexcept
      : CSSConditionRule(mediaRule, parent)
  {
  }

  CSSMediaRule::~CSSMediaRule() noexcept
  {
    if (_mediaCSSOMWrapper)
    {
      _mediaCSSOMWrapper->DetachFromParent();
    }
  }

  Ref<CSSMediaRule> CSSMediaRule::Create(MediaRule &rule, CSSStyleSheet *sheet) noexcept
  {
    return AdoptRef(*new CSSMediaRule(rule, sheet));
  }

  const mq::MediaQueryList &CSSMediaRule::MediaQueries() const noexcept
  {
    return Downcast<MediaRule>(GetGroupRule()).MediaQueries();
  }

  void CSSMediaRule::SetMediaQueries(mq::MediaQueryList &&queries) noexcept
  {
    Downcast<MediaRule>(GetGroupRule()).SetMediaQueries(krys::move(queries));
  }

  CSSOMString CSSMediaRule::CssText() const noexcept
  {
    StringBuilder builder;
    builder.Append(u8"@media ", ConditionText());
    AppendCSSTextForItems(builder);
    return krys::move(builder).ToString();
  }

  CSSOMString CSSMediaRule::CssText(const SerialisationContext &context) const noexcept
  {
    StringBuilder builder;
    builder.Append(u8"@media ", ConditionText());
    AppendCSSTextWithReplacementURLsForItems(builder, context);
    return krys::move(builder).ToString();
  }

  CSSOMString CSSMediaRule::ConditionText() const noexcept
  {
    StringBuilder builder;
    mq::Serialise(builder, MediaQueries());
    return krys::move(builder).ToString();
  }

  MediaList *CSSMediaRule::Media() const noexcept
  {
    if (!_mediaCSSOMWrapper)
    {
      _mediaCSSOMWrapper = MediaList::Create(const_cast<CSSMediaRule *>(this));
    }
    return _mediaCSSOMWrapper.get();
  }
}
