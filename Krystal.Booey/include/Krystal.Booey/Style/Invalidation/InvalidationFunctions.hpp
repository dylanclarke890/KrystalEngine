/*
 * Copyright (C) 2016-2017 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Rules/ScopeRuleSets.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"

namespace krys::boo::style
{
  /// @brief Calls @p function with the features of @p element's shadow tree, if that tree has `:host` rules.
  template <typename TFunction>
  inline void TraverseRuleFeaturesInShadowTree(dom::Element &element, TFunction &&function) noexcept
  {
    auto *shadowRoot = dom::TreeQueries::GetShadowRoot(element);
    if (!shadowRoot)
    {
      return;
    }

    auto &shadowRuleSets = shadowRoot->StyleScope().GetResolver().RuleSets();
    bool hasHostPseudoClassRule = shadowRuleSets.HasMatchingUserOrAuthorStyle(
      [&](RuleSet &style)
      {
        return !style.HostPseudoClassRules().empty() || style.HasHostPseudoClassRulesMatchingInShadowTree();
      });
    if (!hasHostPseudoClassRule)
    {
      return;
    }

    function(shadowRuleSets.Features(), false);
  }

  /// @brief Calls @p function with the features of the shadow trees @p element is slotted into, for those
  /// with `::slotted()` rules.
  template <typename TFunction>
  inline void TraverseRuleFeaturesForSlotted(dom::Element &element, TFunction &&function) noexcept
  {
    auto assignedShadowRoots = dom::AssignedShadowRootsIfSlotted(element);
    for (auto &assignedShadowRoot : assignedShadowRoots)
    {
      auto &ruleSets = assignedShadowRoot->StyleScope().GetResolver().RuleSets();
      if (!ruleSets.HasMatchingUserOrAuthorStyle([](RuleSet &style)
                                                 { return !style.SlottedPseudoElementRules().empty(); }))
      {
        continue;
      }

      function(ruleSets.Features(), false);
    }
  }

  /// @brief Calls @p function with the features of every rule set that may match @p element: its own tree's,
  /// its shadow tree's and those of the shadow trees it's slotted into. The second argument says whether the
  /// rules may also affect the element's (user agent) shadow tree.
  template <typename TFunction>
  inline void TraverseRuleFeatures(dom::Element &element, TFunction &&function) noexcept
  {
    auto &ruleSets = element.StyleResolver().RuleSets();

    auto mayAffectShadowTree = [&]
    {
      auto *shadowRoot = dom::TreeQueries::GetShadowRoot(element);
      if (shadowRoot && shadowRoot->Mode() == dom::ShadowRootMode::UserAgent)
      {
        if (ruleSets.HasMatchingUserOrAuthorStyle([](RuleSet &style)
                                                  { return style.HasUserAgentPartRules(); }))
        {
          return true;
        }
        // FIXME: ::cue rules for media elements, with media.
      }
      return false;
    };

    function(ruleSets.Features(), mayAffectShadowTree());

    TraverseRuleFeaturesInShadowTree(element, function);
    TraverseRuleFeaturesForSlotted(element, function);

    // Ensure that the containing tree resolver also exists so it doesn't get created in the middle of
    // invalidation.
    if (element.IsInShadowTree())
    {
      if (auto *containingShadowRoot = dom::TreeQueries::ContainingShadowRoot(element))
      {
        auto &host = *containingShadowRoot->Host();
        if (host.IsConnected())
        {
          (void)Scope::ForNode(host).GetResolver();
        }
      }
    }
  }
}
