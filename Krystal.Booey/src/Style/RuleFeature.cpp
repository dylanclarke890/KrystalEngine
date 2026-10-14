#include "Krystal.Booey/Style/RuleFeature.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Style/RuleData.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::style
{
  namespace
  {
    KRYS_NODISCARD bool IsSiblingOrSubject(MatchElement matchElement) noexcept
    {
      switch (matchElement)
      {
        case MatchElement::Subject:
        case MatchElement::IndirectSibling:
        case MatchElement::DirectSibling:
        case MatchElement::AnySibling:
        case MatchElement::HasSibling:
        case MatchElement::HasAnySibling:
        case MatchElement::Host:
        case MatchElement::HostChild:            return true;
        case MatchElement::Parent:
        case MatchElement::Ancestor:
        case MatchElement::ParentSibling:
        case MatchElement::AncestorSibling:
        case MatchElement::ParentAnySibling:
        case MatchElement::AncestorAnySibling:
        case MatchElement::HasChild:
        case MatchElement::HasDescendant:
        case MatchElement::HasSiblingDescendant:
        case MatchElement::HasChildParent:
        case MatchElement::HasChildAncestor:
        case MatchElement::HasDescendantParent:
        case MatchElement::HasNonSubject:
        case MatchElement::HasScopeBreaking:     return false;
      }
      krys_debug_assert(false);
      return false;
    }

    KRYS_NODISCARD MatchElement ComputeNextMatchElement(MatchElement matchElement,
                                                        css::SelectorRelation relation) noexcept
    {
      krys_debug_assert(!IsHasPseudoClassMatchElement(matchElement));

      if (IsSiblingOrSubject(matchElement))
      {
        switch (relation)
        {
          case css::SelectorRelation::Compounding: return matchElement;
          case css::SelectorRelation::Descendant:  return MatchElement::Ancestor;
          case css::SelectorRelation::Child:       return MatchElement::Parent;
          case css::SelectorRelation::SubsequentSibling:
            if (matchElement == MatchElement::AnySibling)
            {
              return MatchElement::AnySibling;
            }
            return MatchElement::IndirectSibling;
          case css::SelectorRelation::NextSibling:
            if (matchElement == MatchElement::AnySibling)
            {
              return MatchElement::AnySibling;
            }
            return matchElement == MatchElement::Subject ? MatchElement::DirectSibling
                                                         : MatchElement::IndirectSibling;
          case css::SelectorRelation::ShadowDescendant:
          case css::SelectorRelation::ShadowPartDescendant: return MatchElement::Host;
          case css::SelectorRelation::ShadowSlotted:        return MatchElement::HostChild;
        }
      }
      switch (relation)
      {
        case css::SelectorRelation::Compounding: return matchElement;
        case css::SelectorRelation::Descendant:
        case css::SelectorRelation::Child:       return MatchElement::Ancestor;
        case css::SelectorRelation::SubsequentSibling:
        case css::SelectorRelation::NextSibling:
          return matchElement == MatchElement::Parent ? MatchElement::ParentSibling
                                                      : MatchElement::AncestorSibling;
        case css::SelectorRelation::ShadowDescendant:
        case css::SelectorRelation::ShadowPartDescendant: return MatchElement::Host;
        case css::SelectorRelation::ShadowSlotted:        return MatchElement::HostChild;
      }
      krys_debug_assert(false);
      return matchElement;
    }

    KRYS_NODISCARD bool IsScopeBreaking(MatchElement matchElement) noexcept
    {
      switch (matchElement)
      {
        case MatchElement::HasAnySibling:
        case MatchElement::HasScopeBreaking:     return true;
        case MatchElement::Subject:
        case MatchElement::IndirectSibling:
        case MatchElement::DirectSibling:
        case MatchElement::AnySibling:
        case MatchElement::HasSibling:
        case MatchElement::Host:
        case MatchElement::HostChild:
        case MatchElement::Parent:
        case MatchElement::Ancestor:
        case MatchElement::ParentSibling:
        case MatchElement::AncestorSibling:
        case MatchElement::ParentAnySibling:
        case MatchElement::AncestorAnySibling:
        case MatchElement::HasChild:
        case MatchElement::HasDescendant:
        case MatchElement::HasSiblingDescendant:
        case MatchElement::HasChildParent:
        case MatchElement::HasChildAncestor:
        case MatchElement::HasDescendantParent:
        case MatchElement::HasNonSubject:        return false;
      }
      krys_debug_assert(false);
      return false;
    }

    KRYS_NODISCARD MatchElement ComputeNextHasPseudoClassMatchElement(MatchElement matchElement,
                                                                      css::SelectorRelation relation,
                                                                      CanBreakScope canBreakScope) noexcept
    {
      krys_debug_assert(IsHasPseudoClassMatchElement(matchElement));

      if (canBreakScope == CanBreakScope::No)
      {
        return matchElement;
      }

      // `:has(:is(foo bar))` can be affected by changes outside the :has scope.
      if (relation == css::SelectorRelation::Descendant || relation == css::SelectorRelation::Child)
      {
        // However, for `:has(> :is(.x > .y))`, the child combinator (>) inside :is() is still scoped to the
        // direct child's tree. The parent in the relationship must be the direct child itself, which is
        // within the :has(>) scope. Only descendant combinators can reach outside this scope (to ancestors of
        // the subject element).
        if (matchElement == MatchElement::HasChild && relation == css::SelectorRelation::Child)
        {
          return matchElement;
        }
        return MatchElement::HasScopeBreaking;
      }

      if (relation == css::SelectorRelation::SubsequentSibling
          || relation == css::SelectorRelation::NextSibling)
      {
        // `:has(~ :is(.x ~ .y))` must look at previous siblings of the :scope scope too.
        if (matchElement == MatchElement::HasSibling)
        {
          return MatchElement::HasAnySibling;
        }
        // `:has(~ :is(.x ~ .y)) .z` must be treated as scope breaking, rather than HasAnySibling like the
        // previous case.
        if (matchElement == MatchElement::HasNonSubject)
        {
          return MatchElement::HasScopeBreaking;
        }
      }

      return matchElement;
    }

    KRYS_NODISCARD MatchElement ComputeSubSelectorMatchElement(MatchElement matchElement,
                                                               const css::Selector &selector,
                                                               const css::Selector &childSelector) noexcept
    {
      if (selector.Match() == css::SelectorMatch::PseudoClass)
      {
        auto type = selector.PseudoClass();
        // For :nth-child(n of .some-subselector) where an element change may affect other elements similar to
        // sibling combinators.
        if (type == css::SelectorPseudoClass::NthChild || type == css::SelectorPseudoClass::NthLastChild)
        {
          if (matchElement == MatchElement::Parent)
          {
            return MatchElement::ParentAnySibling;
          }
          if (matchElement == MatchElement::Ancestor)
          {
            return MatchElement::AncestorAnySibling;
          }
          return MatchElement::AnySibling;
        }

        // Similarly for :host().
        if (type == css::SelectorPseudoClass::Host)
        {
          return MatchElement::Host;
        }

        if (type == css::SelectorPseudoClass::Has)
        {
          auto hasSelectorMatchElement = ComputeHasPseudoClassMatchElement(childSelector);
          if (hasSelectorMatchElement == MatchElement::HasChild)
          {
            // :has(> .changed) > .subject
            if (matchElement == MatchElement::Parent)
            {
              return MatchElement::HasChildParent;
            }
            // :has(> .changed) .subject
            if (matchElement == MatchElement::Ancestor)
            {
              return MatchElement::HasChildAncestor;
            }
          }
          if (hasSelectorMatchElement == MatchElement::HasDescendant)
          {
            if (matchElement == MatchElement::Parent)
            {
              return MatchElement::HasDescendantParent;
            }
          }
          if (matchElement != MatchElement::Subject)
          {
            return MatchElement::HasNonSubject;
          }
          return hasSelectorMatchElement;
        }
      }
      if (selector.Match() == css::SelectorMatch::PseudoElement)
      {
        // Similarly for ::slotted().
        if (selector.PseudoElement() == css::SelectorPseudoElement::Slotted)
        {
          return MatchElement::Host;
        }
      }

      return matchElement;
    }

    KRYS_NODISCARD PseudoClassInvalidationKey MakePseudoClassInvalidationKey(
      css::SelectorPseudoClass pseudoClass, const css::Selector &selector) noexcept
    {
      auto attributeName = css::CSSOMStringAtom::Null();
      auto className = css::CSSOMStringAtom::Null();
      auto tagName = css::CSSOMStringAtom::Null();
      for (auto *simpleSelector = selector.LastInCompoundSelector(); simpleSelector;
           simpleSelector = simpleSelector->PrecedingComplexSelectorComponent())
      {
        if (simpleSelector->Match() == css::SelectorMatch::Id)
        {
          return style::MakePseudoClassInvalidationKey(pseudoClass, InvalidationKeyType::Id,
                                                       simpleSelector->Value());
        }

        if (simpleSelector->Match() == css::SelectorMatch::Class && className == css::CSSOMStringAtom::Null())
        {
          className = simpleSelector->Value();
        }

        if (simpleSelector->Match() == css::SelectorMatch::Type)
        {
          tagName = simpleSelector->TagLocalNameLower();
        }

        if (simpleSelector->IsAttributeSelector()
            && !UnlikelyToHaveSelectorForAttribute(simpleSelector->Attribute().LocalNameLower()))
        {
          attributeName = simpleSelector->Attribute().LocalNameLower();
        }

        if (simpleSelector->Relation() != css::SelectorRelation::Compounding)
        {
          break;
        }
      }
      if (!attributeName.View().empty())
      {
        return style::MakePseudoClassInvalidationKey(pseudoClass, InvalidationKeyType::Attribute,
                                                     attributeName);
      }
      if (!className.View().empty())
      {
        return style::MakePseudoClassInvalidationKey(pseudoClass, InvalidationKeyType::Class, className);
      }
      if (!tagName.View().empty() && tagName != dom::StarAtom())
      {
        return style::MakePseudoClassInvalidationKey(pseudoClass, InvalidationKeyType::Tag, tagName);
      }
      return style::MakePseudoClassInvalidationKey(pseudoClass, InvalidationKeyType::Universal);
    }
  }

  bool IsHasPseudoClassMatchElement(MatchElement matchElement) noexcept
  {
    switch (matchElement)
    {
      case MatchElement::HasChild:
      case MatchElement::HasDescendant:
      case MatchElement::HasSibling:
      case MatchElement::HasSiblingDescendant:
      case MatchElement::HasAnySibling:
      case MatchElement::HasNonSubject:
      case MatchElement::HasScopeBreaking:     return true;
      default:                                 return false;
    }
  }

  MatchElement ComputeHasPseudoClassMatchElement(const css::Selector &hasSelector) noexcept
  {
    auto hasMatchElement = MatchElement::Subject;
    for (auto *simpleSelector = &hasSelector; simpleSelector->PrecedingComplexSelectorComponent();
         simpleSelector = simpleSelector->PrecedingComplexSelectorComponent())
    {
      hasMatchElement = ComputeNextMatchElement(hasMatchElement, simpleSelector->Relation());
    }

    switch (hasMatchElement)
    {
      case MatchElement::Parent:
      case MatchElement::Subject:              return MatchElement::HasChild;
      case MatchElement::Ancestor:             return MatchElement::HasDescendant;
      case MatchElement::IndirectSibling:
      case MatchElement::DirectSibling:
      case MatchElement::AnySibling:           return MatchElement::HasSibling;
      case MatchElement::ParentSibling:
      case MatchElement::AncestorSibling:
      case MatchElement::ParentAnySibling:
      case MatchElement::AncestorAnySibling:   return MatchElement::HasSiblingDescendant;
      case MatchElement::HasChild:
      case MatchElement::HasDescendant:
      case MatchElement::HasSibling:
      case MatchElement::HasSiblingDescendant:
      case MatchElement::HasAnySibling:
      case MatchElement::HasChildParent:
      case MatchElement::HasChildAncestor:
      case MatchElement::HasDescendantParent:
      case MatchElement::HasNonSubject:
      case MatchElement::HasScopeBreaking:
      case MatchElement::Host:
      case MatchElement::HostChild:            krys_debug_assert(false); break;
    }
    return MatchElement::HasChild;
  }

  RuleAndSelector::RuleAndSelector(const RuleData &ruleData) noexcept
      : StyleRule(ShareRefPtr(&ruleData.StyleRule())),
        SelectorIndex(static_cast<uint16>(ruleData.SelectorIndex())),
        SelectorListIndex(static_cast<uint16>(ruleData.SelectorListIndex()))
  {
    krys_debug_assert(SelectorIndex == ruleData.SelectorIndex());
    krys_debug_assert(SelectorListIndex == ruleData.SelectorListIndex());
  }

  const css::Selector &RuleAndSelector::Selector() const noexcept
  {
    return *StyleRule->GetSelectorList().SelectorComponentAt(SelectorIndex);
  }

  RuleFeature::RuleFeature(const RuleData &ruleData, style::MatchElement matchElement,
                           style::IsNegation isNegation) noexcept
      : RuleAndSelector(ruleData), MatchElement(matchElement), IsNegation(isNegation)
  {
  }

  RuleFeatureWithInvalidationSelector::RuleFeatureWithInvalidationSelector(
    const RuleData &ruleData, style::MatchElement matchElement, style::IsNegation isNegation,
    css::SelectorList &&invalidationSelector) noexcept
      : RuleFeature(ruleData, matchElement, isNegation),
        InvalidationSelector(krys::move(invalidationSelector))
  {
  }

  SelectorDeduplicationKey::SelectorDeduplicationKey(const css::Selector &selector) noexcept
      : Selector(&selector),
        CachedHash(css::HashComplexSelector(
          selector, css::ComplexSelectorsEqualMode::IgnoreNonElementBackedPseudoElements))
  {
  }

  bool SelectorDeduplicationKey::operator==(const SelectorDeduplicationKey &other) const noexcept
  {
    // Selectors like '.foo' and '.foo::before' are equal for invalidation as they both invalidate the
    // generating element.
    return css::ComplexSelectorsEqual(*Selector, *other.Selector,
                                      css::ComplexSelectorsEqualMode::IgnoreNonElementBackedPseudoElements);
  }

  DoesBreakScope RuleFeatureSet::RecursivelyCollectFeaturesFromSelector(
    SelectorFeatures &selectorFeatures, const css::Selector &firstSelector, style::MatchElement matchElement,
    style::IsNegation isNegation, CanBreakScope allComponentsCanBreakScope) noexcept
  {
    auto doesBreakScope = DoesBreakScope::No;
    const css::Selector *selector = &firstSelector;
    while (true)
    {
      auto canBreakScope = allComponentsCanBreakScope;
      if (selector->Match() == css::SelectorMatch::Id)
      {
        IdsInRules.insert(selector->Value());
        if (matchElement == style::MatchElement::Parent || matchElement == style::MatchElement::Ancestor)
        {
          IdsMatchingAncestorsInRules.insert(selector->Value());
        }
        else if (IsHasPseudoClassMatchElement(matchElement) || matchElement == style::MatchElement::AnySibling
                 || matchElement == style::MatchElement::Host
                 || matchElement == style::MatchElement::HostChild)
        {
          selectorFeatures.Ids.emplace_back(selector, matchElement, isNegation);
        }
      }
      else if (selector->Match() == css::SelectorMatch::Class)
      {
        selectorFeatures.Classes.emplace_back(selector, matchElement, isNegation);
      }
      else if (selector->IsAttributeSelector())
      {
        AttributeLowercaseLocalNamesInRules.insert(selector->Attribute().LocalNameLower());
        AttributeLocalNamesInRules.insert(selector->Attribute().LocalName());
        selectorFeatures.Attributes.emplace_back(selector, matchElement, isNegation);
      }
      else if (selector->Match() == css::SelectorMatch::PseudoElement)
      {
        // Don't put anything here as selectors that differ by pseudo-element only are collected only once.
        // Pseudo-elements are handled in CollectPseudoElementFeatures.
      }
      else if (selector->Match() == css::SelectorMatch::PseudoClass)
      {
        bool isLogicalCombination = css::IsLogicalCombinationPseudoClass(selector->PseudoClass());
        if (!isLogicalCombination)
        {
          selectorFeatures.PseudoClasses.emplace_back(selector, matchElement, isNegation);
        }

        // Check for the :has(:is(foo bar)) case. In this case `foo` can match elements outside the :has()
        // scope.
        if (isLogicalCombination && IsHasPseudoClassMatchElement(matchElement))
        {
          canBreakScope = CanBreakScope::Yes;
        }
      }

      if (const css::SelectorList *selectorList = selector->SubSelectors())
      {
        auto subSelectorIsNegation = isNegation;
        if (selector->Match() == css::SelectorMatch::PseudoClass
            && selector->PseudoClass() == css::SelectorPseudoClass::Not)
        {
          subSelectorIsNegation =
            isNegation == style::IsNegation::No ? style::IsNegation::Yes : style::IsNegation::No;
        }

        for (auto &subSelector : *selectorList)
        {
          auto subSelectorMatchElement = ComputeSubSelectorMatchElement(matchElement, *selector, subSelector);
          auto pseudoClassDoesBreakScope = RecursivelyCollectFeaturesFromSelector(
            selectorFeatures, subSelector, subSelectorMatchElement, subSelectorIsNegation, canBreakScope);
          if (selector->Match() == css::SelectorMatch::PseudoClass
              && selector->PseudoClass() == css::SelectorPseudoClass::Has)
          {
            selectorFeatures.HasPseudoClasses.emplace_back(&subSelector, subSelectorMatchElement, isNegation,
                                                           pseudoClassDoesBreakScope);
          }
          if (pseudoClassDoesBreakScope == DoesBreakScope::Yes)
          {
            doesBreakScope = DoesBreakScope::Yes;
          }
        }
      }

      if (!selector->PrecedingComplexSelectorComponent())
      {
        break;
      }

      matchElement = [&]
      {
        if (IsHasPseudoClassMatchElement(matchElement))
        {
          return ComputeNextHasPseudoClassMatchElement(matchElement, selector->Relation(),
                                                       allComponentsCanBreakScope);
        }
        return ComputeNextMatchElement(matchElement, selector->Relation());
      }();

      if (IsScopeBreaking(matchElement))
      {
        doesBreakScope = DoesBreakScope::Yes;
      }

      selector = selector->PrecedingComplexSelectorComponent();
    };

    return doesBreakScope;
  }

  PseudoClassInvalidationKey MakePseudoClassInvalidationKey(css::SelectorPseudoClass pseudoClass,
                                                            InvalidationKeyType keyType,
                                                            const css::CSSOMStringAtom &keyString) noexcept
  {
    krys_debug_assert(keyType != InvalidationKeyType::Universal || keyString == dom::StarAtom());
    return {ToUnderlying(pseudoClass), static_cast<uint8>(keyType), keyString};
  }

  PseudoClassInvalidationKey MakePseudoClassInvalidationKey(css::SelectorPseudoClass pseudoClass,
                                                            InvalidationKeyType keyType) noexcept
  {
    return MakePseudoClassInvalidationKey(pseudoClass, keyType, dom::StarAtom());
  }

  bool UnlikelyToHaveSelectorForAttribute(const css::CSSOMStringAtom &name) noexcept
  {
    return name.View() == u8"class" || name.View() == u8"id" || name.View() == u8"style";
  }

  void RuleFeatureSet::CollectFeatures(CollectionContext &collectionContext, const RuleData &ruleData,
                                       const List<Ref<const css::ScopeRule>> &scopeRules) noexcept
  {
    SelectorFeatures selectorFeatures;

    bool firstSeen =
      collectionContext.SelectorDeduplicationSet.insert(SelectorDeduplicationKey(*ruleData.Selector()))
        .second;
    if (firstSeen)
    {
      RecursivelyCollectFeaturesFromSelector(selectorFeatures, *ruleData.Selector());
    }
    if (ruleData.CanMatchPseudoElement())
    {
      CollectPseudoElementFeatures(ruleData);
    }

    for (auto &scopeRule : scopeRules)
    {
      auto collectSelectorList = [&](const css::SelectorList &selectorList)
      {
        if (!selectorList.IsEmpty())
        {
          for (auto &subSelector : selectorList)
          {
            RecursivelyCollectFeaturesFromSelector(selectorFeatures, subSelector,
                                                   style::MatchElement::Ancestor);
            RecursivelyCollectFeaturesFromSelector(selectorFeatures, subSelector,
                                                   style::MatchElement::Subject);
          }
        }
      };
      collectSelectorList(scopeRule->ScopeStart());
      collectSelectorList(scopeRule->ScopeEnd());
    }

    if (ruleData.UsedRuleTypes().Contains(UsedRuleType::StartingStyle))
    {
      HasStartingStyleRules = true;
    }

    auto addToMap = [&](auto &map, auto &entries, HashSet<css::CSSOMStringAtom> *hostAffectingNames)
    {
      for (auto &entry : entries)
      {
        auto &[selector, matchElement, isNegation] = entry;
        auto &name = selector->Value();
        auto &featureVector = map[name];
        if (!featureVector)
        {
          featureVector = CreateUnique<RuleFeatureVector>();
        }
        featureVector->emplace_back(ruleData, matchElement, isNegation);

        SetUsesMatchElement(matchElement);

        if (hostAffectingNames && matchElement == style::MatchElement::Host)
        {
          hostAffectingNames->insert(name);
        }
      }
    };

    addToMap(IdRules, selectorFeatures.Ids, nullptr);
    addToMap(ClassRules, selectorFeatures.Classes, &ClassesAffectingHost);

    for (auto &entry : selectorFeatures.Attributes)
    {
      auto &[selector, matchElement, isNegation] = entry;
      auto &featureVector = AttributeRules[selector->Attribute().LocalNameLower()];
      if (!featureVector)
      {
        featureVector = CreateUnique<List<RuleFeatureWithInvalidationSelector>>();
      }
      featureVector->emplace_back(ruleData, matchElement, isNegation,
                                  css::SelectorList::CopySimple(*selector));
      if (matchElement == style::MatchElement::Host)
      {
        AttributesAffectingHost.insert(selector->Attribute().LocalNameLower());
      }
      SetUsesMatchElement(matchElement);
    }

    for (auto &entry : selectorFeatures.PseudoClasses)
    {
      auto &[selector, matchElement, isNegation] = entry;
      auto &featureVector =
        PseudoClassRules[MakePseudoClassInvalidationKey(selector->PseudoClass(), *selector)];
      if (!featureVector)
      {
        featureVector = CreateUnique<RuleFeatureVector>();
      }
      featureVector->emplace_back(ruleData, matchElement, isNegation);
      if (matchElement == style::MatchElement::Host)
      {
        PseudoClassesAffectingHost.insert(selector->PseudoClass());
      }
      PseudoClasses.insert(selector->PseudoClass());
      SetUsesMatchElement(matchElement);
    }

    for (auto &entry : selectorFeatures.HasPseudoClasses)
    {
      auto &[selector, matchElement, isNegation, doesBreakScope] = entry;
      // The selector argument points to a selector inside :has() selector list instead of :has() itself.
      auto &featureVector =
        HasPseudoClassRules[MakePseudoClassInvalidationKey(css::SelectorPseudoClass::Has, *selector)];
      if (!featureVector)
      {
        featureVector = CreateUnique<List<RuleFeatureWithInvalidationSelector>>();
      }
      featureVector->emplace_back(ruleData, matchElement, isNegation,
                                  css::SelectorList::CopyComplex(*selector));

      if (doesBreakScope == DoesBreakScope::Yes)
      {
        ScopeBreakingHasPseudoClassRules.emplace_back(ruleData);
      }

      SetUsesMatchElement(matchElement);
    }
  }

  void RuleFeatureSet::CollectPseudoElementFeatures(const RuleData &ruleData) noexcept
  {
    krys_debug_assert(ruleData.CanMatchPseudoElement());
    auto &selector = *ruleData.Selector();

    for (auto *simpleSelector = &selector; simpleSelector;
         simpleSelector = simpleSelector->PrecedingCompoundSelectorComponent())
    {
      if (simpleSelector->Match() != css::SelectorMatch::PseudoElement)
      {
        continue;
      }

      switch (simpleSelector->PseudoElement())
      {
        case css::SelectorPseudoElement::FirstLine:   UsesFirstLineRules = true; continue;
        case css::SelectorPseudoElement::FirstLetter: UsesFirstLetterRules = true; continue;
        default:                                      continue;
      }
    }
  }

  void RuleFeatureSet::Add(const RuleFeatureSet &other) noexcept
  {
    IdsInRules.insert(other.IdsInRules.begin(), other.IdsInRules.end());
    IdsMatchingAncestorsInRules.insert(other.IdsMatchingAncestorsInRules.begin(),
                                       other.IdsMatchingAncestorsInRules.end());
    AttributeLowercaseLocalNamesInRules.insert(other.AttributeLowercaseLocalNamesInRules.begin(),
                                               other.AttributeLowercaseLocalNamesInRules.end());
    AttributeLocalNamesInRules.insert(other.AttributeLocalNamesInRules.begin(),
                                      other.AttributeLocalNamesInRules.end());
    ContentAttributeNamesInRules.insert(other.ContentAttributeNamesInRules.begin(),
                                        other.ContentAttributeNamesInRules.end());

    auto addMap = [&](auto &map, auto &otherMap)
    {
      for (auto &[key, value] : otherMap)
      {
        auto &features = map[key];
        if (!features)
        {
          features = CreateUnique<std::decay_t<decltype(*value)>>();
        }
        features->insert(features->end(), value->begin(), value->end());
      }
    };

    addMap(IdRules, other.IdRules);

    addMap(ClassRules, other.ClassRules);
    ClassesAffectingHost.insert(other.ClassesAffectingHost.begin(), other.ClassesAffectingHost.end());

    addMap(AttributeRules, other.AttributeRules);
    AttributesAffectingHost.insert(other.AttributesAffectingHost.begin(),
                                   other.AttributesAffectingHost.end());

    addMap(PseudoClassRules, other.PseudoClassRules);
    PseudoClassesAffectingHost.insert(other.PseudoClassesAffectingHost.begin(),
                                      other.PseudoClassesAffectingHost.end());
    PseudoClasses.insert(other.PseudoClasses.begin(), other.PseudoClasses.end());

    addMap(HasPseudoClassRules, other.HasPseudoClassRules);
    ScopeBreakingHasPseudoClassRules.insert(ScopeBreakingHasPseudoClassRules.end(),
                                            other.ScopeBreakingHasPseudoClassRules.begin(),
                                            other.ScopeBreakingHasPseudoClassRules.end());

    for (size_t i = 0; i < UsedMatchElements.size(); ++i)
    {
      UsedMatchElements[i] = UsedMatchElements[i] || other.UsedMatchElements[i];
    }

    UsesFirstLineRules = UsesFirstLineRules || other.UsesFirstLineRules;
    UsesFirstLetterRules = UsesFirstLetterRules || other.UsesFirstLetterRules;
    HasStartingStyleRules = HasStartingStyleRules || other.HasStartingStyleRules;
  }

  void RuleFeatureSet::RegisterContentAttribute(const css::CSSOMStringAtom &attributeName) noexcept
  {
    ContentAttributeNamesInRules.insert(css::CSSOMStringAtom(text::ToASCIILower(attributeName.View())));
    AttributeLowercaseLocalNamesInRules.insert(attributeName);
    AttributeLocalNamesInRules.insert(attributeName);
  }

  void RuleFeatureSet::Clear() noexcept
  {
    IdsInRules.clear();
    IdsMatchingAncestorsInRules.clear();
    AttributeLowercaseLocalNamesInRules.clear();
    AttributeLocalNamesInRules.clear();
    ContentAttributeNamesInRules.clear();
    IdRules.clear();
    ClassRules.clear();
    HasPseudoClassRules.clear();
    ScopeBreakingHasPseudoClassRules.clear();
    ClassesAffectingHost.clear();
    AttributeRules.clear();
    AttributesAffectingHost.clear();
    PseudoClassRules.clear();
    PseudoClassesAffectingHost.clear();
    PseudoClasses.clear();
    UsesFirstLineRules = false;
    UsesFirstLetterRules = false;
    HasStartingStyleRules = false;
  }

  void RuleFeatureSet::ShrinkToFit() noexcept
  {
    ScopeBreakingHasPseudoClassRules.shrink_to_fit();
    for (auto &[key, rules] : IdRules)
    {
      rules->shrink_to_fit();
    }
    for (auto &[key, rules] : ClassRules)
    {
      rules->shrink_to_fit();
    }
    for (auto &[key, rules] : AttributeRules)
    {
      rules->shrink_to_fit();
    }
    for (auto &[key, rules] : PseudoClassRules)
    {
      rules->shrink_to_fit();
    }
    for (auto &[key, rules] : HasPseudoClassRules)
    {
      rules->shrink_to_fit();
    }
  }
}
