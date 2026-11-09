/*
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

#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSConditionRule.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class MediaList;
  class MediaRule;

  /// @brief An `@media` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/cssom/#the-cssmediarule-interface
  class CSSMediaRule final : public CSSConditionRule
  {
    friend class MediaList;

  private:
    mutable RefPtr<MediaList> _mediaCSSOMWrapper;

    CSSMediaRule(MediaRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSMediaRule() noexcept;

    KRYS_NODISCARD static Ref<CSSMediaRule> Create(MediaRule &rule, CSSStyleSheet *sheet) noexcept;

    KRYS_NODISCARD MediaList *Media() const noexcept;

  private:
    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::Media;
    }

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    KRYS_NODISCARD CSSOMString CssText(const SerialisationContext &context) const noexcept final;
    KRYS_NODISCARD CSSOMString ConditionText() const noexcept final;

    KRYS_NODISCARD const mq::MediaQueryList &MediaQueries() const noexcept;
    void SetMediaQueries(mq::MediaQueryList &&queries) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSMediaRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::Media;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
