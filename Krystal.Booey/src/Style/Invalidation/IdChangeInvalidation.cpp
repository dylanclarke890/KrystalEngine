/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Style/Invalidation/IdChangeInvalidation.hpp"
#include "Krystal.Booey/Style/Invalidation/InvalidationFunctions.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::style
{
  void IdChangeInvalidation::InvalidateStyle(const dom::DOMStringAtom &changedId) noexcept
  {
    if (changedId.View().empty())
    {
      return;
    }

    bool mayAffectStyle = false;
    bool mayAffectStyleInShadowTree = false;

    TraverseRuleFeatures(*_element,
                         [&](const RuleFeatureSet &features, bool mayAffectShadowTree)
                         {
                           if (!features.IdsInRules.contains(changedId))
                           {
                             return;
                           }
                           mayAffectStyle = true;
                           if (mayAffectShadowTree)
                           {
                             mayAffectStyleInShadowTree = true;
                           }
                         });

    if (!mayAffectStyle)
    {
      return;
    }

    if (mayAffectStyleInShadowTree)
    {
      _element->InvalidateStyleForSubtree();
      return;
    }

    _element->InvalidateStyle();

    auto collect = [&](const ScopeRuleSets &ruleSets, Maybe<MatchElement> onlyMatchElement = null)
    {
      // This could be easily optimized for fine-grained descendant invalidation similar to
      // ClassChangeInvalidation. However using ids for dynamic styling is rare and this is probably not worth
      // the memory cost of the required data structures.
      bool mayAffectDescendantStyle = ruleSets.Features().IdsMatchingAncestorsInRules.contains(changedId);
      if (mayAffectDescendantStyle)
      {
        _element->InvalidateStyleForSubtree();
      }
      else
      {
        _element->InvalidateStyle();
      }

      // Invalidation rulesets exist for :has() / :nth-child() / :nth-last-child.
      if (auto *invalidationRuleSets = ruleSets.IdInvalidationRuleSets(changedId))
      {
        for (auto &invalidationRuleSet : *invalidationRuleSets)
        {
          if (onlyMatchElement && invalidationRuleSet.MatchElement != *onlyMatchElement)
          {
            continue;
          }

          Invalidator::AddToMatchElementRuleSets(_matchElementRuleSets, invalidationRuleSet);
        }
      }
    };

    collect(_element->StyleResolver().RuleSets());

    if (auto *shadowRoot = dom::TreeQueries::GetShadowRoot(*_element))
    {
      collect(shadowRoot->StyleScope().GetResolver().RuleSets(), MatchElement::Host);
    }
  }

  void IdChangeInvalidation::InvalidateStyleWithRuleSets() noexcept
  {
    Invalidator::InvalidateWithMatchElementRuleSets(*_element, _matchElementRuleSets);
  }
}
