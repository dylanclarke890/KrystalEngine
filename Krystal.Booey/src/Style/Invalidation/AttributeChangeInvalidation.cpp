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

#include "Krystal.Booey/Style/Invalidation/AttributeChangeInvalidation.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorChecker.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/Style/Invalidation/InvalidationFunctions.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::style
{
  namespace
  {
    KRYS_NODISCARD bool MayBeAffectedByAttributeChange(const RuleFeatureSet &features, bool isHTML,
                                                       const dom::QualifiedName &attributeName) noexcept
    {
      auto &nameSet =
        isHTML ? features.AttributeLowercaseLocalNamesInRules : features.AttributeLocalNamesInRules;
      return nameSet.contains(attributeName.LocalName());
    }
  }

  void AttributeChangeInvalidation::InvalidateStyle(const dom::QualifiedName &attributeName,
                                                    Maybe<dom::DOMStringView> oldValue,
                                                    Maybe<dom::DOMStringView> newValue) noexcept
  {
    if (newValue == oldValue)
    {
      return;
    }

    bool isHTML = Is<html::HTMLElement>(*_element) && Is<dom::HTMLDocument>(_element->NodeDocument());

    bool shouldInvalidateCurrent = false;
    bool mayAffectStyleInShadowTree = false;

    css::CSSOMStringAtom attributeNameForLookups(text::ToASCIILower(attributeName.LocalName().View()));

    TraverseRuleFeatures(*_element,
                         [&](const RuleFeatureSet &features, bool mayAffectShadowTree)
                         {
                           if (mayAffectShadowTree
                               && MayBeAffectedByAttributeChange(features, isHTML, attributeName))
                           {
                             mayAffectStyleInShadowTree = true;
                           }
                           if (features.AttributesAffectingHost.contains(attributeNameForLookups))
                           {
                             shouldInvalidateCurrent = true;
                           }
                           else if (features.ContentAttributeNamesInRules.contains(attributeNameForLookups))
                           {
                             shouldInvalidateCurrent = true;
                           }
                         });

    if (mayAffectStyleInShadowTree)
    {
      // FIXME: More fine-grained invalidation.
      _element->InvalidateStyleForSubtree();
    }

    if (shouldInvalidateCurrent)
    {
      _element->InvalidateStyle();
    }

    auto collect = [&](const ScopeRuleSets &ruleSets, Maybe<MatchElement> onlyMatchElement = null)
    {
      auto *invalidationRuleSets = ruleSets.AttributeInvalidationRuleSets(attributeNameForLookups);
      if (!invalidationRuleSets)
      {
        return;
      }

      Maybe<css::CSSOMStringAtom> oldValueAtom;
      Maybe<css::CSSOMStringAtom> newValueAtom;
      if (oldValue)
      {
        oldValueAtom.emplace(*oldValue);
      }
      if (newValue)
      {
        newValueAtom.emplace(*newValue);
      }

      for (auto &invalidationRuleSet : *invalidationRuleSets)
      {
        if (onlyMatchElement && invalidationRuleSet.MatchElement != *onlyMatchElement)
        {
          continue;
        }

        for (auto &selector : invalidationRuleSet.InvalidationSelectors)
        {
          krys_debug_assert(selector.IsAttributeSelector());
          bool oldMatches = oldValueAtom
                            && css::SelectorChecker::AttributeSelectorMatches(*_element, attributeName,
                                                                              *oldValueAtom, selector);
          bool newMatches = newValueAtom
                            && css::SelectorChecker::AttributeSelectorMatches(*_element, attributeName,
                                                                              *newValueAtom, selector);
          if (oldMatches != newMatches)
          {
            Invalidator::AddToMatchElementRuleSets(_matchElementRuleSets, invalidationRuleSet);
            break;
          }
        }
      }
    };

    collect(_element->StyleResolver().RuleSets());

    if (auto *shadowRoot = dom::TreeQueries::GetShadowRoot(*_element))
    {
      collect(shadowRoot->StyleScope().GetResolver().RuleSets(), MatchElement::Host);
    }
  }

  void AttributeChangeInvalidation::InvalidateStyleWithRuleSets() noexcept
  {
    Invalidator::InvalidateWithMatchElementRuleSets(*_element, _matchElementRuleSets);
  }
}
