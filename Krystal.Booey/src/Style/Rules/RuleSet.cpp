/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 2004-2005 Allan Sandfeld Jensen (kde@carewolf.com)
 * Copyright (C) 2006, 2007 Nicholas Shanks (webkit@nickshanks.com)
 * Copyright (C) 2005-2021 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Alexey Proskuryakov <ap@webkit.org>
 * Copyright (C) 2007, 2008 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2008, 2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
 * Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
 * Copyright (C) 2012 Google Inc. All rights reserved.
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

#include "Krystal.Booey/Style/Rules/RuleSet.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorChecker.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Style/Rules/RuleSetBuilder.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"
#include "Krystal.Core/Assert.hpp"
#include <algorithm>

namespace krys::boo::style
{
  namespace
  {
    void ShrinkRulesToFit(RuleSet::RuleDataVector &rules) noexcept
    {
      if (rules.capacity() == rules.size())
      {
        return;
      }
      RuleSet::RuleDataVector shrunk(rules.begin(), rules.end());
      rules.swap(shrunk);
    }

    KRYS_NODISCARD size_t RulesCountForName(const RuleSet::AtomRuleMap &map,
                                            const css::CSSOMStringAtom &name) noexcept
    {
      if (auto it = map.find(name); it != map.end())
      {
        return it->second->size();
      }
      return 0;
    }

    // FIXME: Maybe we can unify both following functions
    KRYS_NODISCARD bool
      HasHostOrScopePseudoClassSubjectInSelectorList(const css::SelectorList *selectorList) noexcept
    {
      if (!selectorList)
      {
        return false;
      }

      for (auto &selector : *selectorList)
      {
        if (selector.IsHostPseudoClass() || selector.IsScopePseudoClass())
        {
          return true;
        }

        if (HasHostOrScopePseudoClassSubjectInSelectorList(selector.SubSelectors()))
        {
          return true;
        }
      }

      return false;
    }

    KRYS_NODISCARD bool IsHostSelectorMatchingInShadowTree(const css::Selector &startSelector) noexcept
    {
      auto isHostSelectorMatchingInShadowTreeInSelectorList = [](const css::SelectorList *selectorList)
      {
        if (!selectorList || selectorList->IsEmpty())
        {
          return false;
        }
        for (auto &selector : *selectorList)
        {
          if (IsHostSelectorMatchingInShadowTree(selector))
          {
            return true;
          }
        }
        return false;
      };

      bool hasOnlyOneCompound = true;
      bool hasHostInLastCompound = false;
      for (auto *selector = &startSelector; selector;
           selector = selector->PrecedingComplexSelectorComponent())
      {
        if (selector->Match() == css::SelectorMatch::PseudoClass
            && selector->PseudoClass() == css::SelectorPseudoClass::Host)
        {
          hasHostInLastCompound = true;
        }
        if (isHostSelectorMatchingInShadowTreeInSelectorList(selector->SubSelectors()))
        {
          return true;
        }
        if (selector->PrecedingComplexSelectorComponent()
            && selector->Relation() != css::SelectorRelation::Compounding)
        {
          hasOnlyOneCompound = false;
          hasHostInLastCompound = false;
        }
      }
      return !hasOnlyOneCompound && hasHostInLastCompound;
    }

    KRYS_NODISCARD bool ShouldHaveBucketForAttributeName(const css::Selector &attributeSelector) noexcept
    {
      // Don't make buckets for lazy attributes since we don't want to synchronize.
      if (attributeSelector.Attribute().LocalNameLower().View() == u8"style")
      {
        return false;
      }
      if (svg::SVGElement::AnimatableAttributeForName(attributeSelector.Attribute().LocalName().View())
          != dom::NullQualifiedName())
      {
        return false;
      }
      return true;
    }
  }

  RuleSet::RuleSet() noexcept = default;

  RuleSet::~RuleSet() noexcept = default;

  Ref<RuleSet> RuleSet::Create() noexcept
  {
    return AdoptRef(*new RuleSet);
  }

  void RuleSet::AddToRuleSet(const css::CSSOMStringAtom &key, AtomRuleMap &map,
                             const RuleData &ruleData) noexcept
  {
    if (key == css::CSSOMStringAtom::Null())
    {
      return;
    }
    auto &rules = map[key];
    if (!rules)
    {
      rules = CreateUnique<RuleDataVector>();
    }
    rules->push_back(ruleData);
  }

  void RuleSet::AddRule(const css::StyleRule &rule, uint32 selectorIndex, uint32 selectorListIndex) noexcept
  {
    RuleData ruleData(rule, selectorIndex, selectorListIndex, _ruleCount, {});
    // This path is used when building invalidation RuleSets, no need to collect features (nullptr
    // CollectionContext).
    AddRule(krys::move(ruleData), 0, 0, 0, nullptr);
  }

  void RuleSet::AddRule(RuleData &&ruleData, CascadeLayerIdentifier cascadeLayerIdentifier,
                        ContainerQueryIdentifier containerQueryIdentifier,
                        ScopeRuleIdentifier scopeRuleIdentifier,
                        RuleFeatureSet::CollectionContext *featureCollectionContext) noexcept
  {
    krys_debug_assert(ruleData.Position() == _ruleCount);

    ++_ruleCount;

    auto storeIdentifier = [&](auto identifier, auto &container)
    {
      if (identifier)
      {
        container.resize(_ruleCount, 0);
        container.back() = identifier;
      }
    };
    storeIdentifier(cascadeLayerIdentifier, _cascadeLayerIdentifierForRulePosition);
    storeIdentifier(containerQueryIdentifier, _containerQueryIdentifierForRulePosition);
    storeIdentifier(scopeRuleIdentifier, _scopeRuleIdentifierForRulePosition);

    const auto scopeRules = ScopeRulesFor(ruleData);
    if (featureCollectionContext)
    {
      _features.CollectFeatures(*featureCollectionContext, ruleData, scopeRules);
    }

    size_t classBucketSize = 0;
    const css::Selector *idSelector = nullptr;
    const css::Selector *tagSelector = nullptr;
    const css::Selector *classSelector = nullptr;
    const css::Selector *attributeSelector = nullptr;
    const css::Selector *linkSelector = nullptr;
    const css::Selector *focusSelector = nullptr;
    const css::Selector *focusVisibleSelector = nullptr;
    const css::Selector *rootElementSelector = nullptr;
    const css::Selector *hostPseudoClassSelector = nullptr;
    const css::Selector *customPseudoElementSelector = nullptr;
    const css::Selector *slottedPseudoElementSelector = nullptr;
    const css::Selector *partPseudoElementSelector = nullptr;
    const css::Selector *cuePseudoElementSelector = nullptr;
    const css::Selector *selector = ruleData.Selector();
    do
    {
      switch (selector->Match())
      {
        case css::SelectorMatch::Id: idSelector = selector; break;
        case css::SelectorMatch::Class:
        {
          auto &className = selector->Value();
          if (!classSelector)
          {
            classSelector = selector;
            classBucketSize = RulesCountForName(_classRules, className);
          }
          else if (classBucketSize)
          {
            auto newClassBucketSize = RulesCountForName(_classRules, className);
            if (newClassBucketSize < classBucketSize)
            {
              classSelector = selector;
              classBucketSize = newClassBucketSize;
            }
          }
          break;
        }
        case css::SelectorMatch::AttributeEquals:
        case css::SelectorMatch::AttributeExists:
        case css::SelectorMatch::AttributeIncludes:
        case css::SelectorMatch::AttributeDash:
        case css::SelectorMatch::AttributeSubstring:
        case css::SelectorMatch::AttributePrefix:
        case css::SelectorMatch::AttributeSuffix:
          if (ShouldHaveBucketForAttributeName(*selector))
          {
            attributeSelector = selector;
          }
          break;
        case css::SelectorMatch::Type:
          if (selector->TagQualifiedName().LocalName() != dom::StarAtom())
          {
            tagSelector = selector;
          }
          break;
        case css::SelectorMatch::PseudoElement:
          switch (selector->PseudoElement())
          {
            case css::SelectorPseudoElement::UserAgentPart: customPseudoElementSelector = selector; break;
            case css::SelectorPseudoElement::Slotted:       slottedPseudoElementSelector = selector; break;
            case css::SelectorPseudoElement::Part:          partPseudoElementSelector = selector; break;
            case css::SelectorPseudoElement::Cue:           cuePseudoElementSelector = selector; break;
            default:                                        break;
          }
          break;
        case css::SelectorMatch::PseudoClass:
          switch (selector->PseudoClass())
          {
            case css::SelectorPseudoClass::Link:
            case css::SelectorPseudoClass::Visited:
            case css::SelectorPseudoClass::AnyLink:      linkSelector = selector; break;
            case css::SelectorPseudoClass::Focus:        focusSelector = selector; break;
            case css::SelectorPseudoClass::FocusVisible: focusVisibleSelector = selector; break;
            case css::SelectorPseudoClass::Host:         hostPseudoClassSelector = selector; break;
            case css::SelectorPseudoClass::Root:         rootElementSelector = selector; break;
            case css::SelectorPseudoClass::Scope:
              _hasHostOrScopePseudoClassRulesInUniversalBucket = true;
              break;
            default:
              if (HasHostOrScopePseudoClassSubjectInSelectorList(selector->SubSelectors()))
              {
                _hasHostOrScopePseudoClassRulesInUniversalBucket = true;
              }
              break;
          }
          break;
        case css::SelectorMatch::None:
        case css::SelectorMatch::ForgivingUnknown:
        case css::SelectorMatch::ForgivingUnknownNestContaining:
        case css::SelectorMatch::HasScope:
        case css::SelectorMatch::NestingParent:                  break;
      }
      // We only process the subject (rightmost compound selector).
      if (selector->Relation() != css::SelectorRelation::Compounding)
      {
        break;
      }
      selector = selector->PrecedingComplexSelectorComponent();
    } while (selector);

    if (!_hasHostPseudoClassRulesMatchingInShadowTree)
    {
      _hasHostPseudoClassRulesMatchingInShadowTree = IsHostSelectorMatchingInShadowTree(*ruleData.Selector());
    }

    if (cuePseudoElementSelector)
    {
      _cuePseudoRules.push_back(ruleData);
      return;
    }

    if (slottedPseudoElementSelector)
    {
      // ::slotted pseudo elements work accross shadow boundary making filtering difficult.
      ruleData.DisableSelectorFiltering();
      _slottedPseudoElementRules.push_back(ruleData);
      return;
    }

    if (partPseudoElementSelector)
    {
      // Filtering doesn't work accross shadow boundaries.
      ruleData.DisableSelectorFiltering();
      _partPseudoElementRules.push_back(ruleData);
      return;
    }

    if (customPseudoElementSelector)
    {
      // FIXME: Custom pseudo elements are handled by the shadow tree's selector filter. It doesn't know about
      // the main DOM.
      ruleData.DisableSelectorFiltering();

      auto *nextSelector = customPseudoElementSelector->PrecedingComplexSelectorComponent();
      if (nextSelector && nextSelector->Match() == css::SelectorMatch::PseudoElement
          && nextSelector->PseudoElement() == css::SelectorPseudoElement::Part)
      {
        // Handle selectors like ::part(foo)::placeholder with the part codepath.
        _partPseudoElementRules.push_back(ruleData);
        return;
      }

      AddToRuleSet(customPseudoElementSelector->Value(), _userAgentPartRules, ruleData);
      // FIXME: Clone `::cue` rules into rules for the cue background box (`::-internal-cue-background`), once
      // there are WebVTT cues.
      return;
    }

    if (hostPseudoClassSelector)
    {
      _hostPseudoClassRules.push_back(ruleData);
      return;
    }

    if (idSelector)
    {
      AddToRuleSet(idSelector->Value(), _idRules, ruleData);
      return;
    }

    if (classSelector)
    {
      AddToRuleSet(classSelector->Value(), _classRules, ruleData);
      return;
    }

    if (attributeSelector)
    {
      AddToRuleSet(attributeSelector->Attribute().LocalName(), _attributeLocalNameRules, ruleData);
      AddToRuleSet(attributeSelector->Attribute().LocalNameLower(), _attributeLowercaseLocalNameRules,
                   ruleData);
      return;
    }

    if (linkSelector)
    {
      _linkPseudoClassRules.push_back(ruleData);
      return;
    }

    if (focusSelector)
    {
      _focusPseudoClassRules.push_back(ruleData);
      return;
    }

    if (focusVisibleSelector)
    {
      _focusVisiblePseudoClassRules.push_back(ruleData);
      return;
    }

    if (rootElementSelector)
    {
      _rootElementRules.push_back(ruleData);
      return;
    }

    if (tagSelector)
    {
      AddToRuleSet(tagSelector->TagQualifiedName().LocalName(), _tagLocalNameRules, ruleData);
      AddToRuleSet(tagSelector->TagLocalNameLower(), _tagLowercaseLocalNameRules, ruleData);
      return;
    }

    // If we didn't find a specialized map to stick it in, file under universal rules.
    _universalRules.push_back(ruleData);
  }

  template <typename TFunction>
  void RuleSet::TraverseRuleDatas(TFunction &&function) noexcept
  {
    auto traverseVector = [&](auto &vector)
    {
      for (auto &ruleData : vector)
      {
        function(ruleData);
      }
    };

    auto traverseMap = [&](auto &map)
    {
      for (auto &[key, ruleDatas] : map)
      {
        traverseVector(*ruleDatas);
      }
    };

    traverseMap(_idRules);
    traverseMap(_classRules);
    traverseMap(_attributeLocalNameRules);
    traverseMap(_attributeLowercaseLocalNameRules);
    traverseMap(_tagLocalNameRules);
    traverseMap(_tagLowercaseLocalNameRules);
    traverseMap(_userAgentPartRules);
    traverseVector(_linkPseudoClassRules);
    traverseVector(_cuePseudoRules);
    traverseVector(_hostPseudoClassRules);
    traverseVector(_slottedPseudoElementRules);
    traverseVector(_partPseudoElementRules);
    traverseVector(_focusPseudoClassRules);
    traverseVector(_focusVisiblePseudoClassRules);
    traverseVector(_rootElementRules);
    traverseVector(_universalRules);
  }

  template <typename TFunction>
  void RuleSet::TraverseRuleDatas(TFunction &&function) const noexcept
  {
    const_cast<RuleSet &>(*this).TraverseRuleDatas([&](const RuleData &ruleData) { function(ruleData); });
  }

  Maybe<DynamicMediaQueryEvaluationChanges>
    RuleSet::EvaluateDynamicMediaQueryRules(const css::mq::MediaQueryEvaluator &evaluator) noexcept
  {
    auto collectedChanges = EvaluateDynamicMediaQueryRules(evaluator, 0);

    if (collectedChanges.RequiredFullReset)
    {
      return DynamicMediaQueryEvaluationChanges {DynamicMediaQueryEvaluationChanges::Type::ResetStyle};
    }

    if (collectedChanges.ChangedQueryIndexes.empty())
    {
      return {};
    }

    auto it = _mediaQueryInvalidationRuleSetCache.find(collectedChanges.ChangedQueryIndexes);
    if (it == _mediaQueryInvalidationRuleSetCache.end())
    {
      auto ruleSet = RuleSet::Create();
      {
        const css::mq::MediaQueryEvaluator screenEvaluator {css::CSSOMStringAtom(u8"screen"),
                                                            css::mq::EvaluationResult::True};
        RuleSetBuilder builder(*ruleSet, screenEvaluator);
        for (auto *rules : collectedChanges.AffectedRules)
        {
          for (auto &rule : *rules)
          {
            builder.AddStyleRule(*rule);
          }
        }
      }
      it =
        _mediaQueryInvalidationRuleSetCache.emplace(collectedChanges.ChangedQueryIndexes, krys::move(ruleSet))
          .first;
    }

    DynamicMediaQueryEvaluationChanges changes {DynamicMediaQueryEvaluationChanges::Type::InvalidateStyle};
    changes.InvalidationRuleSets.push_back(RuleSetAndNegation {.RuleSet = it->second});
    return changes;
  }

  RuleSet::CollectedMediaQueryChanges
    RuleSet::EvaluateDynamicMediaQueryRules(const css::mq::MediaQueryEvaluator &evaluator,
                                            size_t startIndex) noexcept
  {
    CollectedMediaQueryChanges collectedChanges;

    HashMap<size_t, bool> affectedRulePositionsAndResults;

    for (size_t i = startIndex; i < _dynamicMediaQueryRules.size(); ++i)
    {
      auto &dynamicRules = _dynamicMediaQueryRules[i];
      bool result = true;
      for (auto &queryList : dynamicRules.MediaQueries)
      {
        if (!evaluator.Evaluate(queryList))
        {
          result = false;
          break;
        }
      }

      if (result != dynamicRules.Result)
      {
        dynamicRules.Result = result;

        if (dynamicRules.RequiresFullReset)
        {
          collectedChanges.RequiredFullReset = true;
          continue;
        }

        for (auto position : dynamicRules.AffectedRulePositions)
        {
          affectedRulePositionsAndResults.try_emplace(position, result);
        }

        collectedChanges.ChangedQueryIndexes.push_back(i);
        collectedChanges.AffectedRules.push_back(&dynamicRules.AffectedRules);
      }
    }

    if (affectedRulePositionsAndResults.empty())
    {
      return collectedChanges;
    }

    TraverseRuleDatas(
      [&](RuleData &ruleData)
      {
        if (auto it = affectedRulePositionsAndResults.find(ruleData.Position());
            it != affectedRulePositionsAndResults.end())
        {
          ruleData.SetEnabled(it->second);
        }
      });

    return collectedChanges;
  }

  void RuleSet::ShrinkToFit() noexcept
  {
    auto shrinkMapVectorsToFit = [](AtomRuleMap &map)
    {
      for (auto &[key, vector] : map)
      {
        ShrinkRulesToFit(*vector);
      }
    };

    shrinkMapVectorsToFit(_idRules);
    shrinkMapVectorsToFit(_classRules);
    shrinkMapVectorsToFit(_attributeLocalNameRules);
    shrinkMapVectorsToFit(_attributeLowercaseLocalNameRules);
    shrinkMapVectorsToFit(_tagLocalNameRules);
    shrinkMapVectorsToFit(_tagLowercaseLocalNameRules);
    shrinkMapVectorsToFit(_userAgentPartRules);
    ShrinkRulesToFit(_linkPseudoClassRules);
    ShrinkRulesToFit(_cuePseudoRules);
    ShrinkRulesToFit(_hostPseudoClassRules);
    ShrinkRulesToFit(_slottedPseudoElementRules);
    ShrinkRulesToFit(_partPseudoElementRules);
    ShrinkRulesToFit(_focusPseudoClassRules);
    ShrinkRulesToFit(_focusVisiblePseudoClassRules);
    ShrinkRulesToFit(_rootElementRules);
    ShrinkRulesToFit(_universalRules);

    _features.ShrinkToFit();

    for (auto &rule : _dynamicMediaQueryRules)
    {
      rule.ShrinkToFit();
    }
    _dynamicMediaQueryRules.shrink_to_fit();

    _cascadeLayers.shrink_to_fit();
    _cascadeLayerIdentifierForRulePosition.shrink_to_fit();
    _containerQueries.shrink_to_fit();
    _containerQueryIdentifierForRulePosition.shrink_to_fit();
    _resolverMutatingRulesInLayers.shrink_to_fit();
  }

  List<Ref<const css::ContainerRule>> RuleSet::ContainerQueryRules() const noexcept
  {
    List<Ref<const css::ContainerRule>> rules;
    rules.reserve(_containerQueries.size());
    for (auto &entry : _containerQueries)
    {
      rules.push_back(entry.ContainerRule);
    }
    return rules;
  }

  List<Ref<const css::ScopeRule>> RuleSet::ScopeRulesFor(const RuleData &ruleData) const noexcept
  {
    if (_scopeRuleIdentifierForRulePosition.size() <= ruleData.Position())
    {
      return {};
    }

    List<Ref<const css::ScopeRule>> queries;
    auto identifier = _scopeRuleIdentifierForRulePosition[ruleData.Position()];
    while (identifier)
    {
      auto &query = _scopeRules[identifier - 1];
      queries.push_back(query.ScopeRule);
      identifier = query.Parent;
    };

    // Order scopes from outermost to innermost.
    std::ranges::reverse(queries);
    return queries;
  }

  RefPtr<const css::PositionTryRule>
    RuleSet::PositionTryRuleForName(const css::CSSOMStringAtom &name) const noexcept
  {
    auto it = _positionTryRules.find(name);
    return it == _positionTryRules.end() ? nullptr : it->second;
  }
}
