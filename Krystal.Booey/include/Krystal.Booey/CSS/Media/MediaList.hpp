/*
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004, 2006, 2008, 2009, 2010, 2012 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  template <typename>
  class ExceptionOr;
}

namespace krys::boo::css
{
  class CSSRule;
  class CSSStyleSheet;

  /// @brief The media query list of a style sheet, an `@import` rule or an `@media` rule, as exposed to
  /// scripts.
  /// @see https://drafts.csswg.org/cssom/#the-medialist-interface
  class MediaList final : public RefCounted<MediaList>
  {
  private:
    WeakPtr<CSSStyleSheet> _parentStyleSheet;
    WeakPtr<CSSRule> _parentRule;
    Maybe<mq::MediaQueryList> _detachedMediaQueries;

    explicit MediaList(CSSStyleSheet *parentSheet) noexcept;
    explicit MediaList(CSSRule *parentRule) noexcept;

  public:
    ~MediaList() noexcept;

    KRYS_NODISCARD static Ref<MediaList> Create(CSSStyleSheet *parentSheet) noexcept;
    KRYS_NODISCARD static Ref<MediaList> Create(CSSRule *parentRule) noexcept;

    KRYS_NODISCARD bool IsSupportedPropertyIndex(uint32 index) const noexcept
    {
      return index < Length();
    }

    KRYS_NODISCARD uint32 Length() const noexcept;
    KRYS_NODISCARD CSSOMString Item(uint32 index) const noexcept;
    dom::ExceptionOr<void> DeleteMedium(const CSSOMString &oldMedium) noexcept;
    void AppendMedium(const CSSOMString &newMedium) noexcept;

    KRYS_NODISCARD CSSOMString MediaText() const noexcept;
    void SetMediaText(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSRule *ParentRule() const noexcept;
    KRYS_NODISCARD CSSStyleSheet *ParentStyleSheet() const noexcept;

    /// @brief Keeps a copy of the media queries, so the list still works once its owner has gone.
    void DetachFromParent() noexcept;

    KRYS_NODISCARD const mq::MediaQueryList &MediaQueries() const noexcept;

  private:
    void SetMediaQueries(mq::MediaQueryList &&queries) noexcept;
  };
}
