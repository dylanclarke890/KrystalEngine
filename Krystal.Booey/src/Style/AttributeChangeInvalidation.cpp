#include "Krystal.Booey/Style/AttributeChangeInvalidation.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorChecker.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Style/InvalidationFunctions.hpp"
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
