/*
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004-2020 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Media/MediaList.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSImportRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSMediaRule.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/StyleSheets/CSSStyleSheet.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  MediaList::MediaList(CSSStyleSheet *parentSheet) noexcept : _parentStyleSheet(CreateWeakPtr(parentSheet))
  {
  }

  MediaList::MediaList(CSSRule *parentRule) noexcept : _parentRule(CreateWeakPtr(parentRule))
  {
  }

  MediaList::~MediaList() noexcept = default;

  Ref<MediaList> MediaList::Create(CSSStyleSheet *parentSheet) noexcept
  {
    return AdoptRef(*new MediaList(parentSheet));
  }

  Ref<MediaList> MediaList::Create(CSSRule *parentRule) noexcept
  {
    return AdoptRef(*new MediaList(parentRule));
  }

  void MediaList::DetachFromParent() noexcept
  {
    _detachedMediaQueries = MediaQueries();
    _parentStyleSheet = nullptr;
    _parentRule = nullptr;
  }

  uint32 MediaList::Length() const noexcept
  {
    return static_cast<uint32>(MediaQueries().size());
  }

  const mq::MediaQueryList &MediaList::MediaQueries() const noexcept
  {
    if (_detachedMediaQueries)
    {
      return *_detachedMediaQueries;
    }
    if (auto *rule = DynamicDowncast<CSSImportRule>(_parentRule.get()))
    {
      return rule->MediaQueries();
    }
    if (auto *rule = DynamicDowncast<CSSMediaRule>(_parentRule.get()))
    {
      return rule->MediaQueries();
    }
    return _parentStyleSheet->MediaQueries();
  }

  void MediaList::SetMediaQueries(mq::MediaQueryList &&queries) noexcept
  {
    if (RefPtr parentStyleSheet = _parentStyleSheet.lock())
    {
      parentStyleSheet->SetMediaQueries(krys::move(queries));
      parentStyleSheet->DidMutate();
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(_parentRule.get());
    if (auto *rule = DynamicDowncast<CSSImportRule>(_parentRule.get()))
    {
      rule->SetMediaQueries(krys::move(queries));
    }
    if (auto *rule = DynamicDowncast<CSSMediaRule>(_parentRule.get()))
    {
      rule->SetMediaQueries(krys::move(queries));
    }
  }

  CSSOMString MediaList::MediaText() const noexcept
  {
    StringBuilder builder;
    mq::Serialise(builder, MediaQueries());
    return krys::move(builder).ToString();
  }

  void MediaList::SetMediaText(const CSSOMString &value) noexcept
  {
    SetMediaQueries(mq::MediaQueryParser::Parse(value, StrictParserContext()));
  }

  CSSRule *MediaList::ParentRule() const noexcept
  {
    return _parentRule.get();
  }

  CSSStyleSheet *MediaList::ParentStyleSheet() const noexcept
  {
    return _parentStyleSheet.get();
  }

  CSSOMString MediaList::Item(uint32 index) const noexcept
  {
    const auto &queries = MediaQueries();
    if (index < queries.size())
    {
      StringBuilder builder;
      mq::Serialise(builder, queries[index]);
      return krys::move(builder).ToString();
    }
    return {};
  }

  dom::ExceptionOr<void> MediaList::DeleteMedium(const CSSOMString &value) noexcept
  {
    auto valueToRemove = text::ToASCIILower(CSSOMStringView(value));
    auto queries = MediaQueries();
    for (uint32 i = 0; i < queries.size(); ++i)
    {
      if (Item(i) == valueToRemove)
      {
        queries.erase(queries.begin() + i);
        SetMediaQueries(krys::move(queries));
        return {};
      }
    }
    return dom::ExceptionCode::NotFoundError;
  }

  void MediaList::AppendMedium(const CSSOMString &value) noexcept
  {
    if (value.empty())
    {
      return;
    }

    auto newQuery = mq::MediaQueryParser::Parse(value, StrictParserContext());
    auto queries = MediaQueries();
    queries.insert(queries.end(), newQuery.begin(), newQuery.end());
    SetMediaQueries(krys::move(queries));
  }
}
