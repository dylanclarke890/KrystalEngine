#pragma once

#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/ScopeRuleSets.hpp"

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
