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

#include "Krystal.Booey/Style/Invalidation/ClassChangeInvalidation.hpp"
#include "Krystal.Booey/Style/Invalidation/InvalidationFunctions.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::style
{
  namespace
  {
    enum class ClassChangeType : bool
    {
      Add,
      Remove
    };

    struct ClassChange
    {
      dom::DOMStringAtom ClassName;
      ClassChangeType Type;
    };

    constexpr size_t ClassChangeVectorInlineCapacity = 4;
    using ClassChangeVector = SmallList<ClassChange, ClassChangeVectorInlineCapacity>;

    KRYS_NODISCARD ClassChangeVector CollectClasses(Span<const dom::DOMStringAtom> classes,
                                                    ClassChangeType changeType) noexcept
    {
      ClassChangeVector changes;
      for (auto &className : classes)
      {
        changes.push_back({className, changeType});
      }
      return changes;
    }

    KRYS_NODISCARD ClassChangeVector ComputeClassChanges(Span<const dom::DOMStringAtom> oldClasses,
                                                         Span<const dom::DOMStringAtom> newClasses) noexcept
    {
      auto oldSize = oldClasses.size();

      if (!oldSize)
      {
        return CollectClasses(newClasses, ClassChangeType::Add);
      }
      if (newClasses.empty())
      {
        return CollectClasses(oldClasses, ClassChangeType::Remove);
      }

      ClassChangeVector changedClasses;

      SmallList<bool, ClassChangeVectorInlineCapacity> remainingClassBits(oldSize, false);
      // Class vectors tend to be very short. This is faster than using a hash table.
      for (auto &newClass : newClasses)
      {
        bool foundFromBoth = false;
        for (size_t i = 0; i < oldSize; ++i)
        {
          if (newClass == oldClasses[i])
          {
            remainingClassBits[i] = true;
            foundFromBoth = true;
          }
        }
        if (foundFromBoth)
        {
          continue;
        }
        changedClasses.push_back({newClass, ClassChangeType::Add});
      }
      for (size_t i = 0; i < oldSize; ++i)
      {
        // If the bit is not set the corresponding class has been removed.
        if (remainingClassBits[i])
        {
          continue;
        }
        changedClasses.push_back({oldClasses[i], ClassChangeType::Remove});
      }

      return changedClasses;
    }

    KRYS_NODISCARD bool InvalidateBeforeAndAfterChange(MatchElement matchElement) noexcept
    {
      switch (matchElement)
      {
        case MatchElement::AnySibling:
        case MatchElement::ParentAnySibling:
        case MatchElement::AncestorAnySibling:
        case MatchElement::HasAnySibling:
        case MatchElement::HasNonSubject:
        case MatchElement::HasScopeBreaking:     return true;
        case MatchElement::Subject:
        case MatchElement::Parent:
        case MatchElement::Ancestor:
        case MatchElement::DirectSibling:
        case MatchElement::IndirectSibling:
        case MatchElement::ParentSibling:
        case MatchElement::AncestorSibling:
        case MatchElement::HasChild:
        case MatchElement::HasChildParent:
        case MatchElement::HasChildAncestor:
        case MatchElement::HasDescendantParent:
        case MatchElement::HasDescendant:
        case MatchElement::HasSibling:
        case MatchElement::HasSiblingDescendant:
        case MatchElement::Host:
        case MatchElement::HostChild:            return false;
      }
      krys_unreachable();
    }
  }

  void ClassChangeInvalidation::ComputeInvalidation(Span<const dom::DOMStringAtom> oldClasses,
                                                    Span<const dom::DOMStringAtom> newClasses) noexcept
  {
    auto classChanges = ComputeClassChanges(oldClasses, newClasses);

    bool shouldInvalidateCurrent = false;
    bool mayAffectStyleInShadowTree = false;

    TraverseRuleFeatures(*_element,
                         [&](const RuleFeatureSet &features, bool mayAffectShadowTree)
                         {
                           for (auto &classChange : classChanges)
                           {
                             if (mayAffectShadowTree && features.ClassRules.contains(classChange.ClassName))
                             {
                               mayAffectStyleInShadowTree = true;
                             }
                             if (features.ClassesAffectingHost.contains(classChange.ClassName))
                             {
                               shouldInvalidateCurrent = true;
                             }
                           }
                         });

    if (mayAffectStyleInShadowTree)
    {
      // FIXME: We should do fine-grained invalidation for shadow tree.
      _element->InvalidateStyleForSubtree();
    }

    if (shouldInvalidateCurrent)
    {
      _element->InvalidateStyle();
    }

    auto invalidateBeforeChange = [&](ClassChangeType type, IsNegation isNegation, MatchElement matchElement)
    {
      if (InvalidateBeforeAndAfterChange(matchElement))
      {
        return true;
      }
      return type == ClassChangeType::Remove ? isNegation == IsNegation::No : isNegation == IsNegation::Yes;
    };

    auto invalidateAfterChange = [&](ClassChangeType type, IsNegation isNegation, MatchElement matchElement)
    {
      if (InvalidateBeforeAndAfterChange(matchElement))
      {
        return true;
      }
      return type == ClassChangeType::Add ? isNegation == IsNegation::No : isNegation == IsNegation::Yes;
    };

    auto collect = [&](const ScopeRuleSets &ruleSets, Maybe<MatchElement> onlyMatchElement = null)
    {
      for (auto &classChange : classChanges)
      {
        if (auto *invalidationRuleSets = ruleSets.ClassInvalidationRuleSets(classChange.ClassName))
        {
          for (auto &invalidationRuleSet : *invalidationRuleSets)
          {
            if (onlyMatchElement && invalidationRuleSet.MatchElement != *onlyMatchElement)
            {
              continue;
            }

            if (invalidateBeforeChange(classChange.Type, invalidationRuleSet.IsNegation,
                                       invalidationRuleSet.MatchElement))
            {
              Invalidator::AddToMatchElementRuleSets(_beforeChangeRuleSets, invalidationRuleSet);
            }
            if (invalidateAfterChange(classChange.Type, invalidationRuleSet.IsNegation,
                                      invalidationRuleSet.MatchElement))
            {
              Invalidator::AddToMatchElementRuleSets(_afterChangeRuleSets, invalidationRuleSet);
            }
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

  void ClassChangeInvalidation::InvalidateBeforeChange() noexcept
  {
    Invalidator::InvalidateWithMatchElementRuleSets(*_element, _beforeChangeRuleSets);
  }

  void ClassChangeInvalidation::InvalidateAfterChange() noexcept
  {
    Invalidator::InvalidateWithMatchElementRuleSets(*_element, _afterChangeRuleSets);
  }
}
