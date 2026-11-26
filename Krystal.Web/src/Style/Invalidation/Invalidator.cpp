/*
 * Copyright (C) 2012-2022 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Web/Style/Invalidation/Invalidator.hpp"
#include "Krystal.Core/Containers/SmallList.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"
#include "Krystal.Web/Css/Rules/ImportRule.hpp"
#include "Krystal.Web/Css/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Web/Dom/Internals/TreeTraversal.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Dom/Trees/TreeQueries.hpp"
#include "Krystal.Web/Html/Elements/HTMLSlotElement.hpp"
#include "Krystal.Web/Style/Resolver.hpp"
#include "Krystal.Web/Style/Rules/ElementRuleCollector.hpp"
#include "Krystal.Web/Style/Rules/RuleSetBuilder.hpp"
#include "Krystal.Web/Style/Rules/ScopeRuleSets.hpp"
#include "Krystal.Web/Style/Rules/SelectorMatchingState.hpp"
#include "Krystal.Web/Style/Scope/Scope.hpp"
#include <ranges>

namespace krys::web::style
{
  namespace
  {
    KRYS_NODISCARD bool ShouldDirtyAllStyle(const List<Ref<css::Rule>> &rules) noexcept
    {
      for (auto &rule : rules)
      {
        if (auto *styleRuleMedia = DynamicDowncast<css::MediaRule>(rule.get()))
        {
          if (ShouldDirtyAllStyle(styleRuleMedia->ChildRules()))
          {
            return true;
          }
          continue;
        }
        if (auto *styleRuleWithNesting = DynamicDowncast<css::StyleRuleWithNesting>(rule.get()))
        {
          if (ShouldDirtyAllStyle(styleRuleWithNesting->NestedRules()))
          {
            return true;
          }
          continue;
        }
        // FIXME: At least font faces don't need full recalc in all cases.
        if (!Is<css::StyleRule>(rule.get()))
        {
          return true;
        }
      }
      return false;
    }

    KRYS_NODISCARD bool ShouldDirtyAllStyle(const css::StyleSheetContents &sheet) noexcept
    {
      for (auto &import : sheet.ImportRules())
      {
        if (!import->StyleSheet())
        {
          continue;
        }
        if (ShouldDirtyAllStyle(*import->StyleSheet()))
        {
          return true;
        }
      }
      if (ShouldDirtyAllStyle(sheet.ChildRules()))
      {
        return true;
      }
      return false;
    }

    KRYS_NODISCARD bool ShouldDirtyAllStyle(const List<Ref<css::StyleSheetContents>> &sheets) noexcept
    {
      for (auto &sheet : sheets)
      {
        if (ShouldDirtyAllStyle(*sheet))
        {
          return true;
        }
      }
      return false;
    }

    /// @brief The first element at or after @p node in tree order, within @p stayWithin.
    KRYS_NODISCARD Element *FirstElementFrom(Node *node, const Node &stayWithin) noexcept
    {
      while (node && !Is<Element>(*node))
      {
        node = dom::TreeQueries::Next(*node, &stayWithin);
      }
      return static_cast<Element *>(node);
    }

    void InvalidateAssignedElements(HTMLSlotElement &slot) noexcept
    {
      auto *assignedNodes = slot.AssignedNodes();
      if (!assignedNodes)
      {
        return;
      }
      for (auto &node : *assignedNodes)
      {
        auto *element = DynamicDowncast<Element>(node.get());
        if (!element)
        {
          continue;
        }
        if (auto *slotElement = DynamicDowncast<HTMLSlotElement>(*element);
            slotElement && dom::TreeTraversal::ContainingShadowRoot(*node))
        {
          InvalidateAssignedElements(*slotElement);
          continue;
        }
        element->InvalidateStyleInternal();
      }
    }
  }

  Invalidator::Invalidator(const List<Ref<css::StyleSheetContents>> &sheets,
                           const css::mq::MediaQueryEvaluator &mediaQueryEvaluator) noexcept
      : _ownedRuleSet(RuleSet::Create()), _ruleSets({{_ownedRuleSet}}),
        _dirtiesAllStyle(ShouldDirtyAllStyle(sheets))
  {
    if (_dirtiesAllStyle)
    {
      return;
    }

    RuleSetBuilder ruleSetBuilder(*_ownedRuleSet, mediaQueryEvaluator, nullptr,
                                  RuleSetBuilder::ShrinkToFit::Disable);

    for (auto &sheet : sheets)
    {
      ruleSetBuilder.AddRulesFromSheet(*sheet);
    }

    _ruleInformation = CollectRuleInformation();
  }

  Invalidator::Invalidator(InvalidationRuleSetVector ruleSets) noexcept
      : _ruleSets(krys::move(ruleSets)), _ruleInformation(CollectRuleInformation())
  {
    krys_debug_assert(!_ruleSets.empty());
  }

  Invalidator::~Invalidator() noexcept = default;

  Invalidator::RuleInformation Invalidator::CollectRuleInformation() const noexcept
  {
    RuleInformation information;
    for (auto &ruleSet : _ruleSets)
    {
      if (!ruleSet.RuleSet->SlottedPseudoElementRules().empty())
      {
        information.HasSlottedPseudoElementRules = true;
      }
      if (!ruleSet.RuleSet->HostPseudoClassRules().empty())
      {
        information.HasHostPseudoClassRules = true;
      }
      if (ruleSet.RuleSet->HasHostPseudoClassRulesMatchingInShadowTree())
      {
        information.HasHostPseudoClassRulesMatchingInShadowTree = true;
      }
      if (ruleSet.RuleSet->HasUserAgentPartRules())
      {
        information.HasUserAgentPartRules = true;
      }
      if (!ruleSet.RuleSet->CuePseudoRules().empty())
      {
        information.HasCuePseudoElementRules = true;
      }
      if (!ruleSet.RuleSet->PartPseudoElementRules().empty())
      {
        information.HasPartPseudoElementRules = true;
      }
    }
    return information;
  }

  Invalidator::CheckDescendants
    Invalidator::InvalidateIfNeeded(Element &element, SelectorMatchingState *selectorMatchingState) noexcept
  {
    InvalidateInShadowTreeIfNeeded(element);

    if (_ruleInformation.HasSlottedPseudoElementRules)
    {
      if (auto *slotElement = DynamicDowncast<HTMLSlotElement>(element))
      {
        InvalidateAssignedElements(*slotElement);
      }
    }

    switch (element.StyleValidity())
    {
      case Validity::Valid:
      case Validity::AnimationInvalid:
      case Validity::InlineStyleInvalid:
      {
        for (auto &ruleSet : _ruleSets)
        {
          ElementRuleCollector ruleCollector(element, *ruleSet.RuleSet, selectorMatchingState,
                                             css::SelectorChecker::Mode::StyleInvalidation);

          auto matches = ruleCollector.MatchesAnyAuthorRules();
          if (ruleSet.IsNegation == IsNegation::No ? matches : !matches)
          {
            element.InvalidateStyleInternal();
            break;
          }
        }

        return CheckDescendants::Yes;
      }
      case Validity::ElementInvalid: return CheckDescendants::Yes;
      case Validity::SubtreeInvalid: return CheckDescendants::No;
    }
    krys_unreachable();
  }

  void Invalidator::InvalidateStyleForTree(Element &root,
                                           SelectorMatchingState *selectorMatchingState) noexcept
  {
    if (InvalidateIfNeeded(root, selectorMatchingState) == CheckDescendants::No)
    {
      return;
    }
    InvalidateStyleForDescendants(root, selectorMatchingState);
  }

  void Invalidator::InvalidateStyleForDescendants(Element &root,
                                                  SelectorMatchingState *selectorMatchingState) noexcept
  {
    SmallList<Element *, 20> parentStack;
    Element *previousElement = &root;
    for (auto *it = FirstElementFrom(root.FirstChild(), root); it;)
    {
      auto &descendant = *it;
      auto *parent = descendant.ParentElement();
      if (parentStack.empty() || parentStack.back() != parent)
      {
        if (parent == previousElement)
        {
          parentStack.push_back(parent);
          if (selectorMatchingState)
          {
            selectorMatchingState->SelectorFilter.PushParentInitializingIfNeeded(*parent);
          }
        }
        else
        {
          while (parentStack.back() != parent)
          {
            parentStack.pop_back();
            if (selectorMatchingState)
            {
              selectorMatchingState->SelectorFilter.PopParent();
            }
          }
        }
      }
      previousElement = &descendant;

      if (InvalidateIfNeeded(descendant, selectorMatchingState) == CheckDescendants::Yes)
      {
        it = FirstElementFrom(dom::TreeQueries::Next(descendant, &root), root);
      }
      else
      {
        it = FirstElementFrom(dom::TreeTraversal::NextSkippingChildren(descendant, &root), root);
      }
    }
  }

  void Invalidator::InvalidateStyle(Document &document) noexcept
  {
    krys_debug_assert(!_dirtiesAllStyle);

    auto *documentElement = dom::TreeQueries::DocumentElement(document);
    if (!documentElement)
    {
      return;
    }

    SelectorMatchingState selectorMatchingState;
    InvalidateStyleForTree(*documentElement, &selectorMatchingState);
  }

  void Invalidator::InvalidateStyle(Scope &scope) noexcept
  {
    if (_dirtiesAllStyle)
    {
      InvalidateAllStyle(scope);
      return;
    }

    if (auto *shadowRoot = scope.ShadowRoot())
    {
      InvalidateStyle(*shadowRoot);
      return;
    }

    InvalidateStyle(scope.Document());
  }

  void Invalidator::InvalidateStyle(ShadowRoot &shadowRoot) noexcept
  {
    krys_debug_assert(!_dirtiesAllStyle);

    if (_ruleInformation.HasHostPseudoClassRules && shadowRoot.Host())
    {
      shadowRoot.Host()->InvalidateStyleInternal();
    }

    for (auto *child = dom::TreeTraversal::FirstElementChild(shadowRoot); child;
         child = dom::TreeTraversal::NextElementSibling(*child))
    {
      SelectorMatchingState selectorMatchingState;
      InvalidateStyleForTree(*child, &selectorMatchingState);
    }
  }

  void Invalidator::InvalidateStyle(Element &element) noexcept
  {
    krys_debug_assert(!_dirtiesAllStyle);

    // Don't use SelectorFilter as the rule sets here tend to be small and the filter would have setup cost
    // deep in the tree.
    InvalidateStyleForTree(element, nullptr);
  }

  void Invalidator::InvalidateStyleWithMatchElement(Element &element, MatchElement matchElement) noexcept
  {
    auto forEachChild = [](ContainerNode &parent, auto &&function)
    {
      for (auto *child = dom::TreeTraversal::FirstElementChild(parent); child;
           child = dom::TreeTraversal::NextElementSibling(*child))
      {
        function(*child);
      }
    };

    switch (matchElement)
    {
      case MatchElement::Subject:
      {
        // .changed
        InvalidateIfNeeded(element, nullptr);
        break;
      }
      case MatchElement::Parent:
      {
        // .changed > .subject
        forEachChild(element, [&](Element &child) { InvalidateIfNeeded(child, nullptr); });
        break;
      }
      case MatchElement::Ancestor:
      {
        // .changed .subject
        SelectorMatchingState selectorMatchingState;
        InvalidateStyleForDescendants(element, &selectorMatchingState);
        break;
      }
      case MatchElement::DirectSibling:
        // .changed + .subject
        if (auto *sibling = dom::TreeTraversal::NextElementSibling(element))
        {
          InvalidateIfNeeded(*sibling, nullptr);
        }
        break;
      case MatchElement::IndirectSibling:
        // .changed ~ .subject
        for (auto *sibling = dom::TreeTraversal::NextElementSibling(element); sibling;
             sibling = dom::TreeTraversal::NextElementSibling(*sibling))
        {
          InvalidateIfNeeded(*sibling, nullptr);
        }
        break;
      case MatchElement::AnySibling:
        // :nth-last-child(even of .changed)
        forEachChild(*element.ParentNode(),
                     [&](Element &parentChild) { InvalidateIfNeeded(parentChild, nullptr); });
        break;
      case MatchElement::ParentSibling:
        // .changed ~ .a > .subject
        for (auto *sibling = dom::TreeTraversal::NextElementSibling(element); sibling;
             sibling = dom::TreeTraversal::NextElementSibling(*sibling))
        {
          forEachChild(*sibling, [&](Element &siblingChild) { InvalidateIfNeeded(siblingChild, nullptr); });
        }
        break;
      case MatchElement::AncestorSibling:
      {
        // .changed ~ .a .subject
        SelectorMatchingState selectorMatchingState;
        for (auto *sibling = dom::TreeTraversal::NextElementSibling(element); sibling;
             sibling = dom::TreeTraversal::NextElementSibling(*sibling))
        {
          selectorMatchingState.SelectorFilter.PopParentsUntil(element.ParentElement());
          InvalidateStyleForDescendants(*sibling, &selectorMatchingState);
        }
        break;
      }
      case MatchElement::ParentAnySibling:
        // :nth-last-child(even of .changed) > .subject
        forEachChild(*element.ParentNode(),
                     [&](Element &sibling)
                     {
                       forEachChild(sibling, [&](Element &siblingChild)
                                    { InvalidateIfNeeded(siblingChild, nullptr); });
                     });
        break;
      case MatchElement::AncestorAnySibling:
      {
        // :nth-last-child(even of .changed) .subject
        SelectorMatchingState selectorMatchingState;
        forEachChild(*element.ParentNode(),
                     [&](Element &sibling)
                     {
                       selectorMatchingState.SelectorFilter.PopParentsUntil(element.ParentElement());
                       InvalidateStyleForDescendants(sibling, &selectorMatchingState);
                     });
        break;
      }
      case MatchElement::HasChild:
      {
        // :has(> .changed)
        if (auto *parent = element.ParentElement())
        {
          InvalidateIfNeeded(*parent, nullptr);
        }
        break;
      }
      case MatchElement::HasDescendant:
      {
        // :has(.changed)
        SmallList<Element *, 16> ancestors;
        for (auto *parent = element.ParentElement(); parent; parent = parent->ParentElement())
        {
          ancestors.push_back(parent);
        }

        SelectorMatchingState selectorMatchingState;
        selectorMatchingState.SelectorFilter.ParentStackReserveInitialCapacity(ancestors.size());
        for (auto *ancestor : ancestors | std::views::reverse)
        {
          InvalidateIfNeeded(*ancestor, &selectorMatchingState);
          selectorMatchingState.SelectorFilter.PushParent(*ancestor);
        }
        break;
      }
      case MatchElement::HasSibling:
        // :has(~ .changed)
        if (auto *sibling = dom::TreeTraversal::PreviousElementSibling(element))
        {
          SelectorMatchingState selectorMatchingState;
          if (auto *parent = element.ParentElement())
          {
            selectorMatchingState.SelectorFilter.PushParentInitializingIfNeeded(*parent);
          }

          for (; sibling; sibling = dom::TreeTraversal::PreviousElementSibling(*sibling))
          {
            InvalidateIfNeeded(*sibling, &selectorMatchingState);
          }
        }
        break;

      case MatchElement::HasChildParent:
        // :has(> .changed) > .subject
      case MatchElement::HasAnySibling:
      {
        // :has(~ :is(.changed ~ .x))
        SelectorMatchingState selectorMatchingState;
        if (auto *parent = element.ParentElement())
        {
          selectorMatchingState.SelectorFilter.PushParentInitializingIfNeeded(*parent);
        }
        forEachChild(*element.ParentNode(),
                     [&](Element &sibling) { InvalidateIfNeeded(sibling, &selectorMatchingState); });
        break;
      }
      case MatchElement::HasSiblingDescendant:
      {
        // :has(~ .a .changed)
        SmallList<Element *, 16> elementAndAncestors;
        elementAndAncestors.push_back(&element);
        for (auto *parent = element.ParentElement(); parent; parent = parent->ParentElement())
        {
          elementAndAncestors.push_back(parent);
        }

        SelectorMatchingState selectorMatchingState;
        selectorMatchingState.SelectorFilter.ParentStackReserveInitialCapacity(elementAndAncestors.size());
        for (auto *elementOrAncestor : elementAndAncestors | std::views::reverse)
        {
          for (auto *sibling = dom::TreeTraversal::PreviousElementSibling(*elementOrAncestor); sibling;
               sibling = dom::TreeTraversal::PreviousElementSibling(*sibling))
          {
            InvalidateIfNeeded(*sibling, &selectorMatchingState);
          }

          selectorMatchingState.SelectorFilter.PushParent(*elementOrAncestor);
        }
        break;
      }
      case MatchElement::HasDescendantParent:
      {
        // :has(.changed) > .subject
        SmallList<Element *, 16> ancestors;
        for (auto *parent = element.ParentElement(); parent; parent = parent->ParentElement())
        {
          ancestors.push_back(parent);
        }

        SelectorMatchingState selectorMatchingState;
        selectorMatchingState.SelectorFilter.ParentStackReserveInitialCapacity(ancestors.size());
        for (auto *ancestor : ancestors | std::views::reverse)
        {
          selectorMatchingState.SelectorFilter.PushParent(*ancestor);
          forEachChild(*ancestor, [&](Element &ancestorChild)
                       { InvalidateIfNeeded(ancestorChild, &selectorMatchingState); });
        }
        break;
      }
      case MatchElement::HasChildAncestor:
      {
        // :has(> .changed) .subject
        if (auto *parent = element.ParentElement())
        {
          SelectorMatchingState selectorMatchingState;
          InvalidateStyleForDescendants(*parent, &selectorMatchingState);
        }
        break;
      }
      case MatchElement::HasNonSubject:
        // :has(.changed) .subject
      case MatchElement::HasScopeBreaking:
      {
        // :has(:is(.changed .a))
        SelectorMatchingState selectorMatchingState;
        InvalidateStyleForDescendants(*dom::TreeQueries::DocumentElement(element.NodeDocument()),
                                      &selectorMatchingState);
        break;
      }
      case MatchElement::Host:
        // :host(.changed) .subject
        InvalidateInShadowTreeIfNeeded(element);
        break;
      case MatchElement::HostChild:
        // ::slotted(.changed)
        if (auto *host = dom::TreeTraversal::ShadowHost(element))
        {
          forEachChild(*host, [&](Element &hostChild) { InvalidateIfNeeded(hostChild, nullptr); });
        }
        break;
    }
  }

  void Invalidator::InvalidateShadowParts(ShadowRoot &shadowRoot) noexcept
  {
    if (shadowRoot.Mode() == dom::ShadowRootMode::UserAgent)
    {
      return;
    }

    for (auto *descendant = FirstElementFrom(shadowRoot.FirstChild(), shadowRoot); descendant;
         descendant = FirstElementFrom(dom::TreeQueries::Next(*descendant, &shadowRoot), shadowRoot))
    {
      // FIXME: We could only invalidate part names that actually show up in rules.
      if (auto part = descendant->GetAttribute(u8"part"); part && !part->empty())
      {
        descendant->InvalidateStyleInternal();
      }

      auto *nestedShadowRoot = dom::TreeTraversal::GetShadowRoot(*descendant);
      if (auto exportParts = descendant->GetAttribute(u8"exportparts");
          nestedShadowRoot && exportParts && !exportParts->empty())
      {
        InvalidateShadowParts(*nestedShadowRoot);
      }
    }
  }

  void Invalidator::InvalidateUserAgentParts(ShadowRoot &shadowRoot) noexcept
  {
    if (shadowRoot.Mode() != dom::ShadowRootMode::UserAgent)
    {
      return;
    }

    for (auto *descendant = FirstElementFrom(shadowRoot.FirstChild(), shadowRoot); descendant;
         descendant = FirstElementFrom(dom::TreeQueries::Next(*descendant, &shadowRoot), shadowRoot))
    {
      auto part = descendant->UserAgentPart();
      if (part.View().empty())
      {
        continue;
      }
      for (auto &ruleSet : _ruleSets)
      {
        if (ruleSet.RuleSet->UserAgentPartRules(part))
        {
          descendant->InvalidateStyleInternal();
        }
      }
    }
  }

  void Invalidator::InvalidateInShadowTreeIfNeeded(Element &element) noexcept
  {
    auto *shadowRoot = dom::TreeTraversal::GetShadowRoot(element);
    if (!shadowRoot)
    {
      return;
    }

    if (_ruleInformation.HasUserAgentPartRules)
    {
      InvalidateUserAgentParts(*shadowRoot);
    }

    if (_ruleInformation.HasHostPseudoClassRulesMatchingInShadowTree)
    {
      for (auto *child = dom::TreeTraversal::FirstElementChild(*shadowRoot); child;
           child = dom::TreeTraversal::NextElementSibling(*child))
      {
        SelectorMatchingState selectorMatchingState;
        InvalidateStyleForTree(*child, &selectorMatchingState);
      }
    }

    // FIXME: Invalidate the subtree of media elements when there are ::cue rules, once there are WebVTT cues.

    // FIXME: More fine-grained invalidation for ::part()
    if (_ruleInformation.HasPartPseudoElementRules)
    {
      InvalidateShadowParts(*shadowRoot);
    }
  }

  void Invalidator::AddToMatchElementRuleSets(MatchElementRuleSets &matchElementRuleSets,
                                              const InvalidationRuleSet &invalidationRuleSet) noexcept
  {
    matchElementRuleSets[invalidationRuleSet.MatchElement].push_back(
      {invalidationRuleSet.RuleSet, IsNegation::No});
  }

  void Invalidator::AddToMatchElementRuleSetsRespectingNegation(
    MatchElementRuleSets &matchElementRuleSets, const InvalidationRuleSet &invalidationRuleSet) noexcept
  {
    matchElementRuleSets[invalidationRuleSet.MatchElement].push_back(
      {invalidationRuleSet.RuleSet, invalidationRuleSet.IsNegation});
  }

  void
    Invalidator::InvalidateWithMatchElementRuleSets(Element &element,
                                                    const MatchElementRuleSets &matchElementRuleSets) noexcept
  {
    SetForScope isInvalidating(element.StyleResolver().RuleSets().IsInvalidatingStyleWithRuleSets(), true);

    for (auto &[matchElement, ruleSets] : matchElementRuleSets)
    {
      Invalidator invalidator(ruleSets);
      invalidator.InvalidateStyleWithMatchElement(element, matchElement);
    }
  }

  void Invalidator::InvalidateWithScopeBreakingHasPseudoClassRuleSet(Element &element,
                                                                     const RuleSet *ruleSet) noexcept
  {
    SetForScope isInvalidating(element.StyleResolver().RuleSets().IsInvalidatingStyleWithRuleSets(), true);
    Invalidator invalidator(InvalidationRuleSetVector {{ShareRefPtr(ruleSet)}});
    invalidator.InvalidateStyleWithMatchElement(element, MatchElement::HasScopeBreaking);
  }

  void Invalidator::InvalidateAllStyle(Scope &scope) noexcept
  {
    if (auto *shadowRoot = scope.ShadowRoot())
    {
      for (auto *shadowChild = dom::TreeTraversal::FirstElementChild(*shadowRoot); shadowChild;
           shadowChild = dom::TreeTraversal::NextElementSibling(*shadowChild))
      {
        shadowChild->InvalidateStyleForSubtreeInternal();
      }
      InvalidateHostAndSlottedStyleIfNeeded(*shadowRoot);
      return;
    }

    scope.Document().ScheduleFullStyleRebuild();
  }

  void Invalidator::InvalidateHostAndSlottedStyleIfNeeded(ShadowRoot &shadowRoot) noexcept
  {
    auto &host = *shadowRoot.Host();
    auto *resolver = shadowRoot.StyleScope().ResolverIfExists();

    if (!resolver
        || resolver->RuleSets().HasMatchingUserOrAuthorStyle(
          [](RuleSet &style) { return !style.HostPseudoClassRules().empty(); }))
    {
      host.InvalidateStyleInternal();
    }

    if (!resolver
        || resolver->RuleSets().HasMatchingUserOrAuthorStyle(
          [](RuleSet &style) { return !style.SlottedPseudoElementRules().empty(); }))
    {
      for (auto *shadowChild = dom::TreeTraversal::FirstElementChild(host); shadowChild;
           shadowChild = dom::TreeTraversal::NextElementSibling(*shadowChild))
      {
        shadowChild->InvalidateStyleInternal();
      }
    }
  }
}
