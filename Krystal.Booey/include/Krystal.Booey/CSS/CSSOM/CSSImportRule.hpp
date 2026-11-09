/*
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * (C) 2002-2003 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2002-2025 Apple Inc. All rights reserved.
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
#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class ImportRule;
  class MediaList;

  /// @brief An `@import` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/cssom/#the-cssimportrule-interface
  class CSSImportRule final : public CSSRule
  {
    friend class MediaList;

  private:
    const Ref<ImportRule> _importRule;
    mutable RefPtr<MediaList> _mediaCSSOMWrapper;
    mutable RefPtr<CSSStyleSheet> _styleSheetCSSOMWrapper;

    CSSImportRule(ImportRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSImportRule() noexcept;

    KRYS_NODISCARD static Ref<CSSImportRule> Create(ImportRule &rule, CSSStyleSheet *sheet) noexcept;

    KRYS_NODISCARD CSSOMString Href() const noexcept;
    KRYS_NODISCARD MediaList &Media() const noexcept;

    /// @brief The imported style sheet, or null if it hasn't loaded.
    KRYS_NODISCARD CSSStyleSheet *GetStyleSheet() const noexcept;
    KRYS_NODISCARD RefPtr<CSSStyleSheet> ProtectedStyleSheet() const noexcept;

    /// @brief The layer the sheet is imported into: empty for an anonymous layer, null for none.
    KRYS_NODISCARD Maybe<CSSOMString> LayerName() const noexcept;
    KRYS_NODISCARD CSSOMString SupportsText() const noexcept;

  private:
    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::Import;
    }

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    void Reattach(Rule &rule) noexcept final;
    void GetChildStyleSheets(HashSet<RefPtr<CSSStyleSheet>> &childStyleSheets) noexcept final;

    KRYS_NODISCARD CSSOMString CssTextInternal(const CSSOMString &urlString) const noexcept;

    KRYS_NODISCARD const mq::MediaQueryList &MediaQueries() const noexcept;
    void SetMediaQueries(mq::MediaQueryList &&queries) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSImportRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::Import;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
