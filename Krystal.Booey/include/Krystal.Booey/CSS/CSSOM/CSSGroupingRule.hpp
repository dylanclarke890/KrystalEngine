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

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class CSSRuleList;
}

namespace krys::boo::css
{
  class GroupRule;
}

namespace krys::boo
{
  /// @brief The base of the rules that contain other rules.
  /// @see https://drafts.csswg.org/cssom/#the-cssgroupingrule-interface
  class CSSGroupingRule : public CSSRule
  {
  private:
    Ref<css::GroupRule> _groupRule;
    mutable List<RefPtr<CSSRule>> _childRuleCSSOMWrappers;
    mutable UniquePtr<CSSRuleList> _ruleListCSSOMWrapper;

  protected:
    CSSGroupingRule(css::GroupRule &groupRule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSGroupingRule() noexcept override;

    KRYS_NODISCARD CSSRuleList &CssRules() const noexcept;

    ExceptionOr<uint32> InsertRule(const CSSOMString &rule, uint32 index) noexcept;

    ExceptionOr<void> DeleteRule(uint32 index) noexcept;

    KRYS_NODISCARD uint32 Length() const noexcept;

    KRYS_NODISCARD CSSRule *Item(uint32 index) const noexcept;

    KRYS_NODISCARD virtual bool IsCSSConditionRule() const noexcept
    {
      return false;
    }

  protected:
    KRYS_NODISCARD const css::GroupRule &GetGroupRule() const noexcept
    {
      return *_groupRule;
    }

    KRYS_NODISCARD css::GroupRule &GetGroupRule() noexcept
    {
      return *_groupRule;
    }

    void Reattach(css::Rule &rule) noexcept override;

    void AppendCSSTextForItems(StringBuilder &builder) const noexcept;

    void AppendCSSTextWithReplacementURLsForItems(StringBuilder &builder,
                                                  const css::SerialisationContext &context) const noexcept;

    KRYS_NODISCARD RefPtr<css::StyleRuleWithNesting>
      PrepareChildStyleRuleForNesting(css::StyleRule &styleRule) noexcept override;

  private:
    KRYS_NODISCARD bool IsGroupingRule() const noexcept final
    {
      return true;
    }

    static void AppendCSSTextForItemsInternal(StringBuilder &builder, const StringBuilder &rules) noexcept;

    void CssTextForRules(StringBuilder &rules) const noexcept;

    void CssTextForRulesWithReplacementURLs(StringBuilder &rules,
                                            const css::SerialisationContext &context) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::CSSGroupingRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::CSSRule &rule) noexcept
  {
    return rule.IsGroupingRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
