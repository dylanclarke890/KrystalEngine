#include "Krystal.Booey/Style/IdChangeInvalidation.hpp"
#include "Krystal.Booey/Style/InvalidationFunctions.hpp"
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
