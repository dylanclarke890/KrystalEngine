/*
 * Copyright (C) 2020 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Style/Invalidation/PseudoClassChangeInvalidation.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/Style/Invalidation/InvalidationFunctions.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::style
{
  SmallList<PseudoClassInvalidationKey, 4>
    MakePseudoClassInvalidationKeys(css::SelectorPseudoClass pseudoClass,
                                    const dom::Element &element) noexcept
  {
    SmallList<PseudoClassInvalidationKey, 4> keys;

    if (auto id = element.IdForStyleResolution(); !id.empty())
    {
      keys.push_back(
        MakePseudoClassInvalidationKey(pseudoClass, InvalidationKeyType::Id, css::CSSOMStringAtom(id)));
    }

    for (auto &className : element.ClassNames())
    {
      keys.push_back(MakePseudoClassInvalidationKey(pseudoClass, InvalidationKeyType::Class, className));
    }

    for (auto &attribute : element.AttributeList())
    {
      css::CSSOMStringAtom localNameLowercase(text::ToASCIILower(attribute->LocalName().View()));
      if (UnlikelyToHaveSelectorForAttribute(localNameLowercase))
      {
        continue;
      }
      keys.push_back(
        MakePseudoClassInvalidationKey(pseudoClass, InvalidationKeyType::Attribute, localNameLowercase));
    }

    keys.push_back(
      MakePseudoClassInvalidationKey(pseudoClass, InvalidationKeyType::Tag,
                                     css::CSSOMStringAtom(text::ToASCIILower(element.LocalName().View()))));
    keys.push_back(MakePseudoClassInvalidationKey(pseudoClass, InvalidationKeyType::Universal));

    return keys;
  }

  void PseudoClassChangeInvalidation::ComputeInvalidation(css::SelectorPseudoClass pseudoClass, Value value,
                                                          InvalidationScope invalidationScope) noexcept
  {
    bool shouldInvalidateCurrent = false;
    bool mayAffectStyleInShadowTree = false;

    TraverseRuleFeatures(_element,
                         [&](const RuleFeatureSet &features, bool mayAffectShadowTree)
                         {
                           if (mayAffectShadowTree && features.PseudoClasses.contains(pseudoClass))
                           {
                             mayAffectStyleInShadowTree = true;
                           }
                           if (dom::TreeQueries::GetShadowRoot(_element)
                               && features.PseudoClassesAffectingHost.contains(pseudoClass))
                           {
                             shouldInvalidateCurrent = true;
                           }
                         });

    if (mayAffectStyleInShadowTree)
    {
      // FIXME: We should do fine-grained invalidation for shadow tree.
      _element.InvalidateStyleForSubtree();
    }

    if (shouldInvalidateCurrent)
    {
      _element.InvalidateStyle();
    }

    for (auto &key : MakePseudoClassInvalidationKeys(pseudoClass, _element))
    {
      CollectRuleSets(key, value, invalidationScope);
    }
  }

  void PseudoClassChangeInvalidation::CollectRuleSets(const PseudoClassInvalidationKey &key, Value value,
                                                      InvalidationScope invalidationScope) noexcept
  {
    auto collect = [&](const ScopeRuleSets &ruleSets, Maybe<MatchElement> onlyMatchElement = null)
    {
      auto *invalidationRuleSets = ruleSets.PseudoClassInvalidationRuleSets(key);
      if (!invalidationRuleSets)
      {
        return;
      }

      for (auto &invalidationRuleSet : *invalidationRuleSets)
      {
        if (onlyMatchElement && invalidationRuleSet.MatchElement != *onlyMatchElement)
        {
          continue;
        }

        // For focus/hover we flip the whole ancestor chain. We only need to do deep invalidation traversal in
        // the change root.
        auto shouldInvalidate = [&]
        {
          bool invalidatesAllDescendants =
            invalidationRuleSet.MatchElement == MatchElement::Ancestor && IsUniversalInvalidation(key);
          switch (invalidationScope)
          {
            case InvalidationScope::All:                     return true;
            case InvalidationScope::SelfChildrenAndSiblings: return !invalidatesAllDescendants;
            case InvalidationScope::Descendants:             return invalidatesAllDescendants;
          }
          krys_unreachable();
        }();
        if (!shouldInvalidate)
        {
          continue;
        }

        if (value == Value::Any)
        {
          Invalidator::AddToMatchElementRuleSets(_beforeChangeRuleSets, invalidationRuleSet);
          Invalidator::AddToMatchElementRuleSets(_afterChangeRuleSets, invalidationRuleSet);
          continue;
        }

        bool invalidateBeforeChange =
          invalidationRuleSet.IsNegation == IsNegation::Yes ? value == Value::True : value == Value::False;
        if (invalidateBeforeChange)
        {
          Invalidator::AddToMatchElementRuleSets(_beforeChangeRuleSets, invalidationRuleSet);
        }
        else
        {
          Invalidator::AddToMatchElementRuleSets(_afterChangeRuleSets, invalidationRuleSet);
        }
      }
    };

    collect(_element.StyleResolver().RuleSets());

    if (auto *shadowRoot = dom::TreeQueries::GetShadowRoot(_element))
    {
      collect(shadowRoot->StyleScope().GetResolver().RuleSets(), MatchElement::Host);
    }
  }

  void PseudoClassChangeInvalidation::InvalidateBeforeChange() noexcept
  {
    Invalidator::InvalidateWithMatchElementRuleSets(_element, _beforeChangeRuleSets);
  }

  void PseudoClassChangeInvalidation::InvalidateAfterChange() noexcept
  {
    Invalidator::InvalidateWithMatchElementRuleSets(_element, _afterChangeRuleSets);
  }
}
