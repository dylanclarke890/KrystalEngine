#include "Krystal.Booey/Style/ChildChangeInvalidation.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorChecker.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/PseudoClassChangeInvalidation.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/ScopeRuleSets.hpp"

namespace krys::boo::style
{
  using ChildChange = dom::ContainerNode::ChildChange;

  namespace
  {
    KRYS_NODISCARD bool NeedsDescendantTraversal(const RuleFeatureSet &features) noexcept
    {
      return features.UsesMatchElement(MatchElement::HasNonSubject)
             || features.UsesMatchElement(MatchElement::HasScopeBreaking)
             || features.UsesMatchElement(MatchElement::HasDescendant)
             || features.UsesMatchElement(MatchElement::HasSiblingDescendant);
    }

    /// @brief Calls @p function for each element descendant of @p root, in tree order.
    template <typename TFunction>
    void ForEachDescendantElement(dom::Element &root, TFunction &&function) noexcept
    {
      for (auto *node = dom::TreeQueries::Next(root, &root); node;
           node = dom::TreeQueries::Next(*node, &root))
      {
        if (auto *element = DynamicDowncast<dom::Element>(*node))
        {
          Ref<dom::Element> protectedElement = ShareRef(*element);
          function(*element);
        }
      }
    }

    void CheckForEmptyStyleChange(dom::Element &element) noexcept
    {
      if (!element.StyleAffectedByEmpty())
      {
        return;
      }

      auto *style = element.GetRenderStyle();
      if (!style || (!style->EmptyState() || element.HasChildNodes()))
      {
        element.InvalidateStyleForSubtree();
      }
    }

    void InvalidateForForwardPositionalRules(dom::Element &parent, dom::Element *elementAfterChange) noexcept
    {
      bool childrenAffected = parent.ChildrenAffectedByForwardPositionalRules();
      bool descendantsAffected = parent.DescendantsAffectedByForwardPositionalRules();

      if (!childrenAffected && !descendantsAffected)
      {
        return;
      }

      for (auto *sibling = elementAfterChange; sibling;
           sibling = dom::TreeQueries::NextElementSibling(*sibling))
      {
        if (childrenAffected)
        {
          sibling->InvalidateStyleInternal();
        }
        if (descendantsAffected)
        {
          for (auto *siblingChild = dom::TreeQueries::FirstElementChild(*sibling); siblingChild;
               siblingChild = dom::TreeQueries::NextElementSibling(*siblingChild))
          {
            siblingChild->InvalidateStyleForSubtreeInternal();
          }
        }
      }
    }

    void InvalidateForBackwardPositionalRules(dom::Element &parent,
                                              dom::Element *elementBeforeChange) noexcept
    {
      bool childrenAffected = parent.ChildrenAffectedByBackwardPositionalRules();
      bool descendantsAffected = parent.DescendantsAffectedByBackwardPositionalRules();

      if (!childrenAffected && !descendantsAffected)
      {
        return;
      }

      for (auto *sibling = elementBeforeChange; sibling;
           sibling = dom::TreeQueries::PreviousElementSibling(*sibling))
      {
        if (childrenAffected)
        {
          sibling->InvalidateStyleInternal();
        }
        if (descendantsAffected)
        {
          for (auto *siblingChild = dom::TreeQueries::FirstElementChild(*sibling); siblingChild;
               siblingChild = dom::TreeQueries::NextElementSibling(*siblingChild))
          {
            siblingChild->InvalidateStyleForSubtreeInternal();
          }
        }
      }
    }

    void InvalidateForFirstChildState(dom::Element &child, bool state) noexcept
    {
      auto *style = child.GetRenderStyle();
      if (!style || style->FirstChildState() == state)
      {
        child.InvalidateStyleForSubtreeInternal();
      }
    }

    void InvalidateForLastChildState(dom::Element &child, bool state) noexcept
    {
      auto *style = child.GetRenderStyle();
      if (!style || style->LastChildState() == state)
      {
        child.InvalidateStyleForSubtreeInternal();
      }
    }
  }

  void ChildChangeInvalidation::InvalidateForChangedElement(
    dom::Element &changedElement, MatchingHasSelectors &matchingHasSelectors,
    ChangedElementRelation changedElementRelation) noexcept
  {
    auto &ruleSets = ParentElement().StyleResolver().RuleSets();

    Invalidator::MatchElementRuleSets matchElementRuleSets;

    bool isChild = changedElement.ParentElement() == &ParentElement();

    auto canAffectElementsWithStyle = [&](MatchElement matchElement)
    {
      switch (matchElement)
      {
        case MatchElement::HasSibling:
        case MatchElement::HasAnySibling:
        case MatchElement::HasChild:
        case MatchElement::HasChildAncestor:
        case MatchElement::HasChildParent:       return isChild;
        case MatchElement::HasDescendant:
        case MatchElement::HasSiblingDescendant:
        case MatchElement::HasDescendantParent:
        case MatchElement::HasNonSubject:
        case MatchElement::HasScopeBreaking:     return true;
        default:                                 krys_debug_assert(false); return false;
      }
    };

    bool isFirst =
      isChild
      && _childChange.PreviousSiblingElement == dom::TreeQueries::PreviousElementSibling(changedElement)
      && changedElementRelation == ChangedElementRelation::SelfOrDescendant;

    auto hasMatchingInvalidationSelector = [&](const InvalidationRuleSet &invalidationRuleSet)
    {
      css::SelectorChecker selectorChecker(changedElement.NodeDocument());
      css::SelectorChecker::CheckingContext checkingContext(css::SelectorChecker::Mode::StyleInvalidation);
      checkingContext.MatchesAllHasScopes = true;

      for (auto &selector : invalidationRuleSet.InvalidationSelectors)
      {
        if (isFirst && invalidationRuleSet.IsNegation == IsNegation::No)
        {
          // If this :has() matches ignoring this mutation, nothing actually changes and we don't need to
          // invalidate.
          // FIXME: We could cache this state across invalidations instead of just testing a single sibling.
          auto *sibling = _childChange.PreviousSiblingElement ? _childChange.PreviousSiblingElement
                                                              : _childChange.NextSiblingElement;
          if (sibling && selectorChecker.Match(selector, *sibling, checkingContext))
          {
            matchingHasSelectors.insert(&selector);
            continue;
          }
        }

        if (matchingHasSelectors.contains(&selector))
        {
          continue;
        }

        if (selectorChecker.Match(selector, changedElement, checkingContext))
        {
          matchingHasSelectors.insert(&selector);
          return true;
        }
      }
      return false;
    };

    auto addHasInvalidation = [&](const List<InvalidationRuleSet> *invalidationRuleSets)
    {
      if (!invalidationRuleSets)
      {
        return;
      }
      for (auto &invalidationRuleSet : *invalidationRuleSets)
      {
        if (!canAffectElementsWithStyle(invalidationRuleSet.MatchElement))
        {
          continue;
        }
        if (!hasMatchingInvalidationSelector(invalidationRuleSet))
        {
          continue;
        }
        Invalidator::AddToMatchElementRuleSetsRespectingNegation(matchElementRuleSets, invalidationRuleSet);
      }
    };

    for (auto &key : MakePseudoClassInvalidationKeys(css::SelectorPseudoClass::Has, changedElement))
    {
      addHasInvalidation(ruleSets.HasPseudoClassInvalidationRuleSets(key));
    }

    Invalidator::InvalidateWithMatchElementRuleSets(changedElement, matchElementRuleSets);
  }

  void ChildChangeInvalidation::InvalidateForChangeOutsideHasScope() noexcept
  {
    // FIXME: This is a performance footgun. Any mutation will trigger a full document traversal.
    if (auto *invalidationRuleSet =
          ParentElement().StyleResolver().RuleSets().ScopeBreakingHasPseudoClassInvalidationRuleSet())
    {
      Invalidator::InvalidateWithScopeBreakingHasPseudoClassRuleSet(ParentElement(), invalidationRuleSet);
    }
  }

  void ChildChangeInvalidation::InvalidateForHasBeforeMutation() noexcept
  {
    krys_debug_assert(_needsHasInvalidation);

    InvalidateForChangeOutsideHasScope();

    MatchingHasSelectors matchingHasSelectors;

    TraverseRemovedElements(
      [&](dom::Element &changedElement)
      {
        InvalidateForChangedElement(changedElement, matchingHasSelectors,
                                    ChangedElementRelation::SelfOrDescendant);
      });

    // :empty is affected by text changes.
    if (_childChange.ChangeType == ChildChange::Type::TextRemoved
        || _childChange.ChangeType == ChildChange::Type::AllChildrenRemoved)
    {
      InvalidateForChangedElement(ParentElement(), matchingHasSelectors,
                                  ChangedElementRelation::SelfOrDescendant);
    }

    auto firstChildStateWillStopMatching = [&]
    {
      if (!_childChange.NextSiblingElement)
      {
        return false;
      }

      if (!ParentElement().ChildrenAffectedByFirstChildRules())
      {
        return false;
      }

      if (_childChange.IsInsertion()
          && !dom::TreeQueries::PreviousElementSibling(*_childChange.NextSiblingElement))
      {
        return true;
      }

      return false;
    };

    auto lastChildStateWillStopMatching = [&]
    {
      if (!_childChange.PreviousSiblingElement)
      {
        return false;
      }

      if (!ParentElement().ChildrenAffectedByLastChildRules())
      {
        return false;
      }

      if (_childChange.IsInsertion()
          && !dom::TreeQueries::NextElementSibling(*_childChange.PreviousSiblingElement))
      {
        return true;
      }

      return false;
    };

    if (ParentElement().AffectedByHasWithPositionalPseudoClass())
    {
      TraverseRemainingExistingSiblings(
        [&](dom::Element &changedElement)
        {
          InvalidateForChangedElement(changedElement, matchingHasSelectors, ChangedElementRelation::Sibling);
        });
    }
    else
    {
      if (firstChildStateWillStopMatching())
      {
        InvalidateForChangedElement(*_childChange.NextSiblingElement, matchingHasSelectors,
                                    ChangedElementRelation::Sibling);
      }

      if (lastChildStateWillStopMatching())
      {
        InvalidateForChangedElement(*_childChange.PreviousSiblingElement, matchingHasSelectors,
                                    ChangedElementRelation::Sibling);
      }
    }
  }

  void ChildChangeInvalidation::InvalidateForHasAfterMutation() noexcept
  {
    krys_debug_assert(_needsHasInvalidation);

    InvalidateForChangeOutsideHasScope();

    MatchingHasSelectors matchingHasSelectors;

    TraverseAddedElements(
      [&](dom::Element &changedElement)
      {
        InvalidateForChangedElement(changedElement, matchingHasSelectors,
                                    ChangedElementRelation::SelfOrDescendant);
      });

    // :empty is affected by text changes.
    if (_childChange.ChangeType == ChildChange::Type::TextInserted && _wasEmpty)
    {
      InvalidateForChangedElement(ParentElement(), matchingHasSelectors,
                                  ChangedElementRelation::SelfOrDescendant);
    }

    auto firstChildStateWillStartMatching = [&](dom::Element *elementAfterChange)
    {
      if (!elementAfterChange)
      {
        return false;
      }

      if (!ParentElement().ChildrenAffectedByFirstChildRules())
      {
        return false;
      }

      if (!_childChange.IsInsertion() && !dom::TreeQueries::PreviousElementSibling(*elementAfterChange))
      {
        return true;
      }

      return false;
    };

    auto lastChildStateWillStartMatching = [&](dom::Element *elementBeforeChange)
    {
      if (!elementBeforeChange)
      {
        return false;
      }

      if (!ParentElement().ChildrenAffectedByLastChildRules())
      {
        return false;
      }

      if (!_childChange.IsInsertion() && !dom::TreeQueries::NextElementSibling(*elementBeforeChange))
      {
        return true;
      }

      return false;
    };

    if (ParentElement().AffectedByHasWithPositionalPseudoClass())
    {
      TraverseRemainingExistingSiblings(
        [&](dom::Element &changedElement)
        {
          InvalidateForChangedElement(changedElement, matchingHasSelectors, ChangedElementRelation::Sibling);
        });
    }
    else
    {
      if (firstChildStateWillStartMatching(_childChange.NextSiblingElement))
      {
        InvalidateForChangedElement(*_childChange.NextSiblingElement, matchingHasSelectors,
                                    ChangedElementRelation::Sibling);
      }

      if (lastChildStateWillStartMatching(_childChange.PreviousSiblingElement))
      {
        InvalidateForChangedElement(*_childChange.PreviousSiblingElement, matchingHasSelectors,
                                    ChangedElementRelation::Sibling);
      }
    }
  }

  template <typename TFunction>
  void ChildChangeInvalidation::TraverseRemovedElements(TFunction &&function) noexcept
  {
    if (_childChange.IsInsertion() && _childChange.ChangeType != ChildChange::Type::AllChildrenReplaced)
    {
      return;
    }

    auto &features = ParentElement().StyleResolver().RuleSets().Features();
    bool needsDescendantTraversal = NeedsDescendantTraversal(features);

    RefPtr<dom::Element> firstToRemove =
      ShareRefPtr(_childChange.PreviousSiblingElement
                    ? dom::TreeQueries::NextElementSibling(*_childChange.PreviousSiblingElement)
                    : dom::TreeQueries::FirstElementChild(ParentElement()));

    for (RefPtr<dom::Element> toRemove = firstToRemove; toRemove.get() != _childChange.NextSiblingElement;
         toRemove = ShareRefPtr(dom::TreeQueries::NextElementSibling(*toRemove)))
    {
      function(*toRemove);

      if (!needsDescendantTraversal)
      {
        continue;
      }

      ForEachDescendantElement(*toRemove, function);
    }
  }

  template <typename TFunction>
  void ChildChangeInvalidation::TraverseAddedElements(TFunction &&function) noexcept
  {
    if (!_childChange.IsInsertion())
    {
      return;
    }

    RefPtr<dom::Element> newElement = [&]
    {
      auto *previous = _childChange.PreviousSiblingElement;
      auto *candidate = previous ? dom::TreeQueries::NextElementSibling(*previous)
                                 : dom::TreeQueries::FirstElementChild(ParentElement());
      if (candidate == _childChange.NextSiblingElement)
      {
        candidate = nullptr;
      }
      return ShareRefPtr(candidate);
    }();

    if (!newElement)
    {
      return;
    }

    function(*newElement);

    auto &features = ParentElement().StyleResolver().RuleSets().Features();
    if (!NeedsDescendantTraversal(features))
    {
      return;
    }

    ForEachDescendantElement(*newElement, function);
  }

  template <typename TFunction>
  void ChildChangeInvalidation::TraverseRemainingExistingSiblings(TFunction &&function) noexcept
  {
    if (_childChange.IsInsertion() && _childChange.ChangeType == ChildChange::Type::AllChildrenReplaced)
    {
      return;
    }

    for (RefPtr<dom::Element> child = ShareRefPtr(_childChange.PreviousSiblingElement); child;
         child = ShareRefPtr(dom::TreeQueries::PreviousElementSibling(*child)))
    {
      function(*child);
    }

    for (RefPtr<dom::Element> child = ShareRefPtr(_childChange.NextSiblingElement); child;
         child = ShareRefPtr(dom::TreeQueries::NextElementSibling(*child)))
    {
      function(*child);
    }
  }

  void ChildChangeInvalidation::InvalidateAfterChange() noexcept
  {
    CheckForEmptyStyleChange(ParentElement());

    if (_childChange.ChangeSource == ChildChange::Source::Parser)
    {
      return;
    }

    CheckForSiblingStyleChanges();
  }

  void ChildChangeInvalidation::InvalidateAfterFinishedParsingChildren(dom::Element &parent) noexcept
  {
    if (!parent.NeedsStyleInvalidation())
    {
      return;
    }

    CheckForEmptyStyleChange(parent);

    auto *lastChildElement = dom::TreeQueries::LastElementChild(parent);
    if (!lastChildElement)
    {
      return;
    }

    if (parent.ChildrenAffectedByLastChildRules())
    {
      InvalidateForLastChildState(*lastChildElement, false);
    }

    InvalidateForBackwardPositionalRules(parent, lastChildElement);
  }

  void ChildChangeInvalidation::CheckForSiblingStyleChanges() noexcept
  {
    auto &parent = ParentElement();
    auto *elementBeforeChange = _childChange.PreviousSiblingElement;
    auto *elementAfterChange = _childChange.NextSiblingElement;

    // :first-child. In the parser callback case, we don't have to check anything, since we were right the
    // first time. In the DOM case, we only need to do something if |afterChange| is not 0. |afterChange| is 0
    // in the parser case, so it works out that we'll skip this block.
    if (parent.ChildrenAffectedByFirstChildRules() && elementAfterChange)
    {
      // Find our new first child.
      auto *newFirstElement = dom::TreeQueries::FirstElementChild(parent);

      // This is the insert/append case.
      if (newFirstElement != elementAfterChange)
      {
        InvalidateForFirstChildState(*elementAfterChange, true);
      }

      // We also have to handle node removal.
      if (_childChange.ChangeType == ChildChange::Type::ElementRemoved
          && newFirstElement == elementAfterChange)
      {
        InvalidateForFirstChildState(*newFirstElement, false);
      }
    }

    // :last-child. In the parser callback case, we don't have to check anything, since we were right the
    // first time. In the DOM case, we only need to do something if |afterChange| is not 0.
    if (parent.ChildrenAffectedByLastChildRules() && elementBeforeChange)
    {
      // Find our new last child.
      auto *newLastElement = dom::TreeQueries::LastElementChild(parent);

      if (newLastElement != elementBeforeChange)
      {
        InvalidateForLastChildState(*elementBeforeChange, true);
      }

      // We also have to handle node removal.
      if (_childChange.ChangeType == ChildChange::Type::ElementRemoved
          && newLastElement == elementBeforeChange)
      {
        InvalidateForLastChildState(*newLastElement, false);
      }
    }

    dom::InvalidateForSiblingCombinators(elementAfterChange);

    InvalidateForForwardPositionalRules(parent, elementAfterChange);
    InvalidateForBackwardPositionalRules(parent, elementBeforeChange);
  }
}
